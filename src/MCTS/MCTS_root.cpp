#include "MCTS_root.h"

#include <cmath>
#include <thread>

namespace {
inline int argmaxUCB(Node *n) {
  float max_s = 0.0f;
  int candidate = -1;
  float par_log = log(n->visits);
  for (int a = 0; a < n->child_count; ++a) {
    Node child = *n->children[a];
    float score = (float)child.wins / child.visits +
                  UCB_explore * std::sqrt(par_log / child.visits);
    if (score > max_s) {
      max_s = score;
      candidate = a;
    }
  }
  return candidate;
}
}  // namespace

MCTS_ROOT::MCTS_ROOT(unsigned int threads) { pool.reset(threads); }

MCTS_ROOT::~MCTS_ROOT() {}

thc::Move MCTS_ROOT::run(const UCI_go_opt &go_opt,
                         const std::shared_ptr<thc::ChessRules> cr) {
  std::cout << "hello" << std::endl;
}

void MCTS_ROOT::reset() {}

// MCTS is submodule of MCTS_ROOT

MCTS::MCTS(unsigned int threads) : gen(rd()) { pool.reset(threads); }

MCTS::~MCTS() {}

thc::Move MCTS::run(const UCI_go_opt &go_opt,
                    const std::shared_ptr<thc::ChessRules> cr) {
  std::cout << "hello" << std::endl;
  while (1) {
    thc::Move mv;
    thc::ChessRules cr_copy(*cr);
    Node *par = selection(&cr_copy);
    Node *new_node = expansion(par, &cr_copy);
    auto result = simulate(new_node, &cr_copy);
    backprop(new_node, result);
  }
  return root->children[argmaxUCB(root.get())]->move;
}

void MCTS::reset() { root.reset(); }

Node *MCTS::selection(thc::ChessRules *cr) {
  Node *wp = root.get();
  Node *prev_wp = wp;
  while (wp->total_child == wp->child_count) {
    int candidate = argmaxUCB(wp);

    cr->PlayMove(wp->move);
    if (candidate == -1) return wp;
    prev_wp = wp;
    wp = wp->children[candidate].get();
  }
  return prev_wp;
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
    }
  }

  std::shared_ptr<Node> &new_n = leaf->children[leaf->child_count++];
  new_n->color = leaf->color == WHITE ? BLACK : WHITE;  // invert color
  new_n->parent = leaf;
  cr->PlayMove(new_n->move);
  return new_n.get();
}

std::array<uint8_t, 2> MCTS::simulate(Node *node, thc::ChessRules *cr) {
  std::array<uint8_t, 2> wins = {0, 0};
  for (int t = 0; t < SIMS; ++t) {
    thc::ChessRules crc(*cr);
    thc::MOVELIST list;
    thc::TERMINAL term;
    crc.Evaluate(term);
    while (term == thc::NOT_TERMINAL) {
      crc.GenLegalMoveList(&list);
      std::uniform_int_distribution unif(0, list.count);
      int index = unif(gen);
      crc.PlayMove(list.moves[index]);
      crc.Evaluate(term);
    }
    if (node->color == BLACK && term == thc::TERMINAL_WCHECKMATE) {
      ++wins[BLACK];
    } else if (node->color == WHITE && term == thc::TERMINAL_BCHECKMATE) {
      ++wins[WHITE];
    }
  }
  return wins;
}

void MCTS::backprop(Node *node, std::array<uint8_t, 2> &result) {
  while (node->parent != nullptr) {
    node->wins += result[node->color];
    ++node->visits;
    node = node->parent;
  }
}
