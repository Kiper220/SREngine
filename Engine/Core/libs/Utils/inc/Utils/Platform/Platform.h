//
// Created by Monika on 17.03.2022.
//

#ifndef SRENGINE_PLATFORM_H
#define SRENGINE_PLATFORM_H

#include <Utils/Math/Vector2.h>
#include <Utils/Common/ThreadUtils.h>
#include <Utils/FileSystem/Path.h>

namespace SR_UTILS_NS::Platform {
    SR_DLL_EXPORT extern void SetInstance(void* pInstance);
    SR_DLL_EXPORT extern void* GetInstance();

    SR_DLL_EXPORT extern std::optional<std::string> ReadFile(const Path& path);
    SR_DLL_EXPORT extern void TextToClipboard(const std::string& text);
    SR_DLL_EXPORT extern void CopyFilesToClipboard(std::list<SR_UTILS_NS::Path> paths);
    SR_DLL_EXPORT extern void PasteFilesFromClipboard(const SR_UTILS_NS::Path& topath);
    SR_DLL_EXPORT extern Path GetApplicationPath();
    SR_DLL_EXPORT extern Path GetApplicationName();
    SR_DLL_EXPORT extern std::string GetClipboardText();
    SR_DLL_EXPORT extern void ClearClipboard();
    SR_DLL_EXPORT extern SR_MATH_NS::FVector2 GetMousePos();
    SR_DLL_EXPORT extern void Sleep(uint64_t milliseconds);
    SR_DLL_EXPORT extern uint64_t GetProcessUsedMemory();
    SR_DLL_EXPORT extern void SetThreadPriority(void* nativeHandle, ThreadPriority priority);
    SR_DLL_EXPORT extern void WriteConsoleLog(const std::string& msg);
    SR_DLL_EXPORT extern void WriteConsoleError(const std::string& msg);
    SR_DLL_EXPORT extern void WriteConsoleWarn(const std::string& msg);
    SR_DLL_EXPORT extern void Terminate();
    SR_DLL_EXPORT extern void SelfOpen();
    SR_DLL_EXPORT extern std::vector<SR_MATH_NS::UVector2> GetScreenResolutions();
    SR_DLL_EXPORT extern void OpenWithAssociatedApp(const Path& filepath);
    SR_DLL_EXPORT extern bool Copy(const Path& from, const Path& to);
    SR_DLL_EXPORT extern bool CreateFolder(const std::string& path);
    SR_DLL_EXPORT extern bool Delete(const Path& path);
    SR_DLL_EXPORT extern bool IsAbsolutePath(const Path& path);
    SR_DLL_EXPORT extern bool IsExists(const Path& path);
    SR_DLL_EXPORT extern bool FileIsHidden(const Path& path);
    SR_DLL_EXPORT extern std::list<Path> GetInDirectory(const Path& dir, Path::Type type);
    SR_DLL_EXPORT extern void OpenInNativeFileExplorer(const Path& dir);
}


#endif //SRENGINE_PLATFORM_H
