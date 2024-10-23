#ifndef GLOBAL_H
#define GLOBAL_H
#include <mutex>
#include <string>
class Global {
    private:
        static std::string content_directory_root;
        static std::mutex  content_directory_root_mutex;
    public:
    static void        SetContentDirectoryRoot(std::string value);

    static std::string GetContentDirectory();
};
#endif
