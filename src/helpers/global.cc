#include "global.h"
#include <mutex>

std::string Global::content_directory_root;
std::mutex Global::content_directory_root_mutex;
std::mutex content_directory_root_mutex;
void        Global::SetContentDirectoryRoot(std::string value) {
    std::lock_guard<std::mutex> lock(content_directory_root_mutex);
    content_directory_root = value;
}

std::string Global::GetContentDirectory() {
    std::lock_guard<std::mutex> lock(content_directory_root_mutex);
    return content_directory_root;
}
