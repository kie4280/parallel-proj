#ifndef UTILS_H
#define UTILS_H

#include <fstream>
#include <string>

// Warning: This library is not thread-safe!!
namespace Logger {
extern std::fstream debugFile;
void debug(std::string str);
void closeLog();
}  // namespace Logger

#endif