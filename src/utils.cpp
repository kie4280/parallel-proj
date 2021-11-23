#include "utils.h"

std::fstream Logger::debugFile{"debug.log", std::ios::trunc | std::ios::out};
void Logger::debug(std::string str) { debugFile << str << std::endl; }
void Logger::closeLog() { Logger::debugFile.close(); }