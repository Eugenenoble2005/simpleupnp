#include "logger.h"
void LogInfo(std::string message)
{
  std::cout << "\033[32m" << "[INFO]" << message << "\033[0m" << std::endl;
}

void LogError(std::string message){
  std::cerr <<  "\033[1;31m" << "[FATAL]" << message  << "\033[0m" << std::endl;
}
