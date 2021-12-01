#include "utils.h"

std::fstream Logger::debugFile{"debug.log", std::ios::trunc | std::ios::out};

void Logger::closeLog() { Logger::debugFile.close(); }