#include "MCTS_tree.h"

#include <chrono>
#include <cmath>
#include <ratio>
#include <thread>
#include <shared_mutex>
#include <mutex>

#include "../utils.h"
std::shared_mutex g_mutex;
#define THREAD_NUM 4
namespace {
inline int argmaxUCB(Node *n) {
  float max_s = 0.0f;
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

Logger logger("debug_MCTS.log");
}  // namespace


MCTS::MCTS() : gen(rd()) {}

MCTS::~MCTS() {}

thc::Move MCTS::run(const UCI_go_opt &go_opt,
                    const std::shared_ptr<thc::ChessRules> cr) {
  std::chrono::time_point<std::chrono::steady_clock> start =
      std::chrono::steady_clock::now();
  logger.debug("[MCTS] computing next move...");
  this->isWhite = cr->WhiteToPlay();
  // if (this->root == nullptr) {
  this->root = std::make_shared<Node>();
  this->root->color = this->isWhite ? BLACK : WHITE;
  // }
  int expand_count = 0;
	
  std::thread threads[THREAD_NUM];
  for (int i=0; i< THREAD_NUM;i++)
  {	
	threads[i]=std::thread(tree_p,cr,go_opt,start, this);
  }
  for (int i=0; i< THREAD_NUM;i++)
  {	
	threads[i].join();
  }
  for (int a = 0; a < this->root->child_count; ++a) {
    logger.debug(this->root->children[a]->wins);
  }
  logger.debug("[MCTS] calculate complete");
  return this->root->children[argmaxUCB(this->root.get())]->move;
}

void MCTS::reset() { root.reset(); }

void tree_p(const std::shared_ptr<thc::ChessRules> cr,const UCI_go_opt go_opt,std::chrono::time_point<std::chrono::steady_clock> start,
		MCTS *tree){
  while (true) {
logger.debug("2222");
    thc::Move mv;
    thc::ChessRules cr_copy(*cr);

	g_mutex.lock();
    Node *par = tree->selection(&cr_copy);
    Node *new_node = tree->expansion(par, &cr_copy);
    g_mutex.unlock();

  //  g_mutex.lock_shared();
    auto result = tree->simulate(new_node, &cr_copy);
   // g_mutex.unlock_shared();

    g_mutex.lock();
    tree->backprop(new_node, result);
    g_mutex.unlock();

 //   g_mutex.lock();
    auto timer = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
  //  g_mutex.unlock();

    if (timer.count() >= go_opt.movetime - TL_PADDING) {
      break;
    }
  }
}
Node *MCTS::selection(thc::ChessRules *cr) {
  Node *wp = root.get();
  while (wp->total_child == wp->child_count) {
    int candidate = argmaxUCB(wp);

    if (candidate == -1) {
      // throw "no candidate";
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
    }
  }
  if (leaf->total_child == 0) {
    return leaf;
	
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
    thc::DRAWTYPE drawtype;
    crc.Evaluate(term);
    while (term == thc::NOT_TERMINAL && !crc.IsDraw(true, drawtype)) {
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
