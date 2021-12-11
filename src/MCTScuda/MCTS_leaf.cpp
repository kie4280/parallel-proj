#include "MCTS_leaf.h"

#include <chrono>
#include <cmath>
#include <ratio>
#include <thread>

#include "../utils.h"

extern void leaf_parallel(char *chess_raw, float *scores, int n);

namespace {
inline int argmaxUCB(Node *n) {
  float max_s = -10000000.0f;
  int candidate = 0;
  float par_log = log(n->visits);
  for (int a = 0; a < n->child_count; ++a) {
    const Node &child = *n->children[a];
    float score = child.wins / child.visits +
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


void generate_next_board_parallel(int id, thc::ChessRules *cr, thc::MOVELIST *list, char *chess_raw){
  int count = list->count;
  int step = count / THREAD_NUM;
  int start = step * id;
  if(id == THREAD_NUM - 1){
    step = count - start;
  }

  for(int i = 0; i < step; i++){
    thc::ChessRules crc(*cr);
    crc.PlayMove(list->moves[i + start]);
    memcpy(chess_raw + (i + start) * BOARD_LENGTH, crc.squares, BOARD_LENGTH);
    //logger.debug(crc.squares);
  }

};


MCTS::MCTS() : gen(rd()) {}

MCTS::~MCTS() {}

thc::Move MCTS::run(const UCI_go_opt &go_opt,
                    std::shared_ptr<thc::ChessRules> cr) {
  std::chrono::time_point<std::chrono::steady_clock> start =
      std::chrono::steady_clock::now();
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
    //logger.debug("[MCTS] selection start");
    Node *par = selection(&cr_copy);
    //logger.debug(par->child_count);
    //logger.debug(par->total_child);
    //logger.debug("[MCTS] expansion start");
    Node *new_node = expansion(par, &cr_copy);
    //logger.debug("[MCTS] simulate start");
    auto result = simulate(new_node, &cr_copy);
    //logger.debug("[MCTS] backprop start");
    backprop(new_node, result);

    auto timer = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    if (timer.count() >= go_opt.movetime - TL_PADDING) {
      break;
    }
  }
  for (int a = 0; a < this->root->child_count; ++a) {
    logger.debug(this->root->children[a]->wins);
  }
  logger.debug("expand non-leaf node:");
  logger.debug(expand_count);
  logger.debug("simulate leaf node:");
  logger.debug(this->root->visits);
  logger.debug("[MCTS] calculate complete");
  return this->root->children[argmaxUCB(this->root.get())]->move;
}

void MCTS::reset() { root.reset(); }

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
  new_n->color = 1 - leaf->color;  // invert color
  new_n->parent = leaf;
  cr->PlayMove(new_n->move);
  return new_n.get();
}


std::array<float, 2> MCTS::simulate(Node *node, thc::ChessRules *cr) {
  std::array<float, 2> result = {0, 0};//{total score , num of leafs}
  thc::MOVELIST list;
  cr->GenLegalMoveList(&list);

  int count = list.count;
  result[1] = count;

  char *chess_raw = new char [count * BOARD_LENGTH];
  float *scores = new float [count];

  std::thread threads[THREAD_NUM];
  for(int i = 0; i < THREAD_NUM; i++){
    threads[i] = std::thread(generate_next_board_parallel, i, cr, &list, chess_raw);
  }
  
  for(int i = 0; i < THREAD_NUM; i++){
    threads[i].join();
  }

  leaf_parallel(chess_raw, scores, count);

  for(int i = 0; i < count; i++){
    result[0] += scores[i];
  }
  result[0] /= count;

  if(node->color != this->isWhite)
    result[0] = -result[0];
  
  delete [] chess_raw;
  delete [] scores;

  return result;
}

void MCTS::backprop(Node *node, std::array<float, 2> &result) {
  while (node != nullptr) {
    node->wins += result[0];
    node->visits += result[1];
    node = node->parent;
  }
}
