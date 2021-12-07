#ifndef MCTS_ROOT_HEADER
#define MCTS_ROOT_HEADER

#include <cstdint>
#include <memory>
#include <random>
#include <array>

#include "../THC-chess/src/thc.h"
#include "../UCI/UCI.h"
#include "leaf.h"

#define WHITE 0
#define BLACK 1

#define BOARD_LENGTH 64
#define THREAD_NUM 8

#define UCB_explore 1.414f
#define SIMS 3
#define TL_PADDING 5

struct Node;

struct Node {
  int wins = 0;
  int visits = 0;
  thc::Move move;
  uint8_t color = WHITE;
  std::shared_ptr<Node> children[256];
  int child_count = 0;
  int total_child = -1;
  Node *parent;
};

class MCTS {
 private:
  std::random_device rd;
  std::default_random_engine gen;
  std::shared_ptr<Node> root;
  bool isWhite;
  inline Node *selection(thc::ChessRules *cr);
  inline Node *expansion(Node *leaf, thc::ChessRules *cr);
  inline std::array<float, 2> simulate(Node *node, thc::ChessRules *cr);
  inline void backprop(Node *node, std::array<float, 2> &result);

 public:
  MCTS();
  ~MCTS();
  thc::Move run(const UCI_go_opt &go_opt,
                const std::shared_ptr<thc::ChessRules> cr);

  void reset();
};
#endif