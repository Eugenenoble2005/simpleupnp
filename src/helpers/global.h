#ifndef GLOBAL_H
#define GLOBAL_H
#include <mutex>
#include <string>
namespace Global{
  extern std::string content_directory_root;
  extern std::mutex content_directory_root_mutex;

  void SetContentDirectoryRoot(std::string value);

  std::string GetContentDirectory();
}
#endif
