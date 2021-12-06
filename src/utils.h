#ifndef UTILS_H
#define UTILS_H

#include <fstream>
#include <string>

class Logger {
 private:
  std::fstream debugFile;
  void closeLog();

 public:
  Logger(const std::string filename = "debug.log");
  ~Logger();
  template <typename T>
  void debug(T obj) {
    debugFile << obj << std::endl;
  }
};

#endif