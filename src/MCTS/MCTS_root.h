#ifndef MCTS_ROOT_HEADER
#define MCTS_ROOT_HEADER

#include "thread_pool.hpp"

struct Node {
  int score = 0;
  int visits = 0;
  char move[4];
};

class MCTS {
 private:
  thread_pool pool;

 public:
  MCTS(unsigned int threads = std::thread::hardware_concurrency());
  ~MCTS();
};

#endif