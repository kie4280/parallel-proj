#include "MCTS_root.h"

#include <thread>

MCTS::MCTS(unsigned int threads) { pool.reset(threads); }

MCTS::~MCTS() {}
