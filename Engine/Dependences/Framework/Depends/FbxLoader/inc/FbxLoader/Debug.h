//
// Created by Monika on 27.07.2021.
//

#ifndef FBXLOADER_DEBUG_H
#define FBXLOADER_DEBUG_H

#include <functional>

namespace FbxLoader {
    class Debug {
    public:
        Debug() = delete;
        Debug(const Debug&) = delete;
        ~Debug() = delete;
    private:
        static inline bool g_init = false;
    public:
        static void Init(const std::function<void(const std::string& msg)>& err_fun) {
            if (!g_init) {
                Error = err_fun;
            }
            g_init = true;
        }
        static bool IsInit() {
            return g_init;
        }
    public:
        static inline std::function<void(const std::string& msg)> Error = std::function<void(const std::string& msg)>();
    };
}

#define FBX_ERROR(msg) Debug::Error(msg);

#endif //FBXLOADER_DEBUG_H
