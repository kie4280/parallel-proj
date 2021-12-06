#include "utils.h"

Logger::Logger(const std::string filename)
    : debugFile(filename, std::ios::trunc | std::ios::out) {}

Logger::~Logger() { closeLog(); }

void Logger::closeLog() { Logger::debugFile.close(); }