#ifndef MCTS_ROOT_HEADER
#define MCTS_ROOT_HEADER

#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <random>
#include <thread>

#include "../THC-chess/src/thc.h"
#include "../UCI/UCI.h"

#define WHITE 0
#define BLACK 1

#define UCB_explore 1.414f
#define SIMS 2
#define TL_PADDING 10

struct Node;
struct MCTS_args;
class MCTS;
class MCTS_ROOT;

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

struct MCTS_args {
  MCTS *single;
  std::condition_variable *cv;
  std::unique_lock<std::mutex> *lock;
  std::atomic_char *flag;
  std::atomic_char *status;
  UCI_go_opt *go_opt;
  const thc::ChessRules *cr;
  std::chrono::time_point<std::chrono::steady_clock> *start;
};

void runMCTS(MCTS_args args);
inline bool timesUp(
    const std::chrono::time_point<std::chrono::steady_clock> *start,
    unsigned int TL);

class MCTS {
 private:
  std::random_device rd;
  std::default_random_engine gen;
  inline Node *selection(thc::ChessRules *cr);
  inline Node *expansion(Node *leaf, thc::ChessRules *cr);
  inline std::array<uint8_t, 2> simulate(
      Node *node, thc::ChessRules *cr,
      const std::chrono::time_point<std::chrono::steady_clock> *start,
      unsigned int TL);
  inline void backprop(Node *node, std::array<uint8_t, 2> &result);

 public:
  bool isWhite;
  std::shared_ptr<Node> root;

 public:
  MCTS();
  ~MCTS();
  thc::Move run(
      const UCI_go_opt *go_opt, const thc::ChessRules *cr,
      const std::chrono::time_point<std::chrono::steady_clock> *start);

  void reset();
};

class MCTS_ROOT {
 private:
  std::thread pool[32];
  MCTS trees[32];
  int thread_count = 0;
  UCI_go_opt opt;
  thc::ChessRules chessrules;
  std::chrono::time_point<std::chrono::steady_clock> start_run;
  std::condition_variable cv;
  std::mutex cv_mux;
  std::unique_lock<std::mutex> cv_uniq;
  std::atomic_char command[32];
  std::atomic_char status[32];

 public:
  MCTS_ROOT(unsigned int threads = std::thread::hardware_concurrency());
  ~MCTS_ROOT();
  thc::Move run(const UCI_go_opt &go_opt,
                const std::shared_ptr<thc::ChessRules> cr);

  void reset();
  void quit();
};

#endif