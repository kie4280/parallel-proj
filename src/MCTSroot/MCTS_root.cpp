#include "MCTS_root.h"

#include <cmath>
#include <iostream>
#include <ratio>

#include "../utils.h"

#define COMD_RUN (char)0
#define COMD_STOP (char)1
#define COMD_WAIT (char)2

#define STAT_IDLE (char)1
#define STAT_FIN (char)2

namespace {
inline int argmaxUCB(Node *n) {
  float max_s = -10000.0f;
  int candidate = 0;
  float par_log = log(n->visits);
  for (int a = 0; a < n->child_count; ++a) {
    const Node &child = *n->children[a];
    float score = (float)child.wins / child.visits +
                  UCB_explore * std::sqrt(par_log / child.visits);
    if (score > max_s) {
      max_s = score;
      candidate = a;
    }
  }
  return candidate;
}

Logger logger("MCTS_root.log");
}  // namespace

void runMCTS(MCTS_args args) {
  while (true) {
    args.cv->wait(*args.lock, [args] {
      return args.flag->load() != COMD_WAIT && args.status->load() == STAT_IDLE;
    });
    if (args.flag->load() == COMD_STOP) break;
    args.single->run(args.go_opt, args.cr, args.start);
    args.status->store(STAT_FIN);
    args.cv->notify_all();
  }
}

bool timesUp(const std::chrono::time_point<std::chrono::steady_clock> *start,
             unsigned int TL) {
  auto timer = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - *start);
  return timer.count() >= TL;
}

MCTS_ROOT::MCTS_ROOT(unsigned int threads)
    : thread_count(threads), cv_uniq(cv_mux, std::defer_lock) {
  for (int a = 0; a < threads; ++a) {
    command[a].store(COMD_WAIT);
    MCTS_args args = {&trees[a],  &cv,  &cv_uniq,    &command[a],
                      &status[a], &opt, &chessrules, &start_run};
    std::thread t(runMCTS, args);
    t.detach();
    pool[a] = std::move(t);
  }
}

MCTS_ROOT::~MCTS_ROOT() {}

thc::Move MCTS_ROOT::run(const UCI_go_opt &go_opt,
                         const std::shared_ptr<thc::ChessRules> cr) {
  start_run = std::chrono::steady_clock::now();
  opt = go_opt;
  chessrules = *cr;
  for (int a = 0; a < thread_count; ++a) {
    command[a].store(COMD_RUN);
    status[a].store(STAT_IDLE);
  }
  cv.notify_all();
  // unblocks all threads

  thc::MOVELIST list;
  cr->GenLegalMoveList(&list);

  cv.wait(cv_uniq, [&, this] {
    bool fin = true;
    for (int a = 0; a < this->thread_count; ++a) {
      fin = fin && this->status[a].load() == STAT_FIN;
    }

    return fin;
  });

  // all threads returned results
  int total[256] = {0};
  int wins[256] = {0};
  for (int a = 0; a < list.count; ++a) {
    for (int t = 0; t < thread_count; ++t) {
      if (trees[t].root->child_count == list.count) {
        total[a] += trees[t].root->children[a]->visits;
        wins[a] += trees[t].root->children[a]->wins;
      }
    }
  }
  float max_score = -10000.0f;
  int i = 0;
  for (int a = 0; a < list.count; ++a) {
    float cand = (float)wins[a] / total[a];
    if (cand > max_score) {
      max_score = cand;
      i = a;
    }
  }

  return list.moves[i];
}

void MCTS_ROOT::reset() {
  for (int a = 0; a < thread_count; ++a) {
    trees[a].reset();
  }
}

void MCTS_ROOT::quit() {
  for (int a = 0; a < thread_count; ++a) {
    command[a].store(COMD_STOP);
    status[a].store(STAT_IDLE);
  }
  cv.notify_all();
  // for (int a = 0; a < thread_count; ++a) {
  //   pool[a].join();
  // }
}

// MCTS is submodule of MCTS_ROOT

MCTS::MCTS() : gen(rd()) {}

MCTS::~MCTS() {}

thc::Move MCTS::run(
    const UCI_go_opt *go_opt, const thc::ChessRules *cr,
    const std::chrono::time_point<std::chrono::steady_clock> *start) {
  logger.debug("[MCTS] computing next move...");
  this->isWhite = cr->WhiteToPlay();
  // if (this->root == nullptr) {
  this->root = std::make_shared<Node>();
  this->root->color = this->isWhite ? BLACK : WHITE;
  // }
  int expand_count = 0;
  while (true) {
    ++expand_count;
    thc::Move mv;
    thc::ChessRules cr_copy(*cr);

    Node *par = selection(&cr_copy);
    Node *new_node = expansion(par, &cr_copy);
    auto result =
        simulate(new_node, &cr_copy, start, go_opt->movetime - TL_PADDING);
    backprop(new_node, result);

    if (timesUp(start, go_opt->movetime - TL_PADDING)) break;
  }
  // for (int a = 0; a < this->root->child_count; ++a) {
  //   logger.debug(this->root->children[a]->wins);
  // }
  logger.debug(expand_count);
  logger.debug("[MCTS] calculate complete");
  return this->root->children[argmaxUCB(this->root.get())]->move;
}

void MCTS::reset() { root = nullptr; }

Node *MCTS::selection(thc::ChessRules *cr) {
  Node *wp = root.get();
  while (wp->total_child == wp->child_count) {
    int candidate = argmaxUCB(wp);

    if (wp->child_count == 0) {
      break;
    } else {
      wp = wp->children[candidate].get();
      cr->PlayMove(wp->move);
    }
  }
  return wp;
}

Node *MCTS::expansion(Node *leaf, thc::ChessRules *cr) {
  if (leaf->total_child == -1) {
    thc::MOVELIST list;
    cr->GenLegalMoveList(&list);
    leaf->total_child = list.count;
    for (int a = 0; a < list.count; ++a) {
      std::shared_ptr<Node> &c = leaf->children[a];
      c = std::make_shared<Node>();
      c->move = list.moves[a];
      c->color = leaf->color == WHITE ? BLACK : WHITE;  // invert color
      c->parent = leaf;
    }
  }
  if (leaf->total_child == 0) {
    return leaf;
  }

  std::shared_ptr<Node> &new_n = leaf->children[leaf->child_count++];
  cr->PlayMove(new_n->move);
  return new_n.get();
}

std::array<uint8_t, 2> MCTS::simulate(
    Node *node, thc::ChessRules *cr,
    const std::chrono::time_point<std::chrono::steady_clock> *start,
    unsigned int TL) {
  std::array<uint8_t, 2> wins = {0, 0};
  for (int t = 0; t < SIMS; ++t) {
    thc::ChessRules crc(*cr);
    thc::MOVELIST list;
    thc::TERMINAL term;
    thc::DRAWTYPE drawtype;
    crc.Evaluate(term);
    while (!timesUp(start, TL) && term == thc::NOT_TERMINAL &&
           !crc.IsDraw(true, drawtype)) {
      crc.GenLegalMoveList(&list);
      std::uniform_int_distribution unif(0, list.count);
      int index = unif(gen);
      crc.PlayMove(list.moves[index]);
      crc.Evaluate(term);
    }
    if (term == thc::TERMINAL_WCHECKMATE || term == thc::TERMINAL_WSTALEMATE) {
      ++wins[BLACK];
    } else if (term == thc::TERMINAL_BCHECKMATE ||
               term == thc::TERMINAL_BSTALEMATE) {
      ++wins[WHITE];
    }
  }
  return wins;
}

void MCTS::backprop(Node *node, std::array<uint8_t, 2> &result) {
  while (node != nullptr) {
    node->wins += result[node->color];
    node->visits += SIMS;
    node = node->parent;
  }
}
