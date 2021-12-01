#ifndef UTILS_H
#define UTILS_H

#include <fstream>
#include <string>

// Warning: This library is not thread-safe!!
namespace Logger {
extern std::fstream debugFile;
template<typename T>
void debug(T obj) {
  debugFile << obj << std::endl;
}
void closeLog();
}  // namespace Logger

#endif