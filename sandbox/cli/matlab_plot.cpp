#include "matlab_plot.h"

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#ifdef _WIN32
#include <process.h>
#else
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;
#endif

namespace {
constexpr const char* kMatlabExecutableEnvironment =
    "TINYSPICE_MATLAB_EXECUTABLE";
constexpr const char* kMatlabScriptDirectoryEnvironment =
    "TINYSPICE_MATLAB_SCRIPT_DIR";

std::filesystem::path absolute_path(const std::filesystem::path& path) {
    std::error_code error;
    const std::filesystem::path absolute =
        std::filesystem::absolute(path, error);
    if (error) {
        throw std::runtime_error(
            "cannot resolve absolute path '" + path.string()
            + "': " + error.message());
    }
    return absolute.lexically_normal();
}

std::filesystem::path configured_script_directory() {
    if (const char* configured =
            std::getenv(kMatlabScriptDirectoryEnvironment);
        configured != nullptr && configured[0] != '\0') {
        return absolute_path(configured);
    }
#ifdef TINYSPICE_MATLAB_SCRIPT_DIR
    return absolute_path(TINYSPICE_MATLAB_SCRIPT_DIR);
#else
    return absolute_path("scripts");
#endif
}

#ifdef __APPLE__
void collect_macos_matlab_installations(
    const std::filesystem::path& root,
    std::vector<std::filesystem::path>& candidates) {
    std::error_code error;
    if (!std::filesystem::is_directory(root, error) || error) {
        return;
    }

    std::filesystem::recursive_directory_iterator iterator(
        root,
        std::filesystem::directory_options::skip_permission_denied,
        error);
    const std::filesystem::recursive_directory_iterator end;
    while (!error && iterator != end) {
        const std::filesystem::directory_entry& entry = *iterator;
        const std::string filename = entry.path().filename().string();
        const bool is_matlab_application =
            entry.is_directory(error)
            && filename.starts_with("MATLAB_")
            && filename.ends_with(".app");
        if (!error && is_matlab_application) {
            const std::filesystem::path executable =
                entry.path() / "bin" / "matlab";
            if (std::filesystem::is_regular_file(executable, error)
                && !error) {
                candidates.push_back(executable);
            }
            iterator.disable_recursion_pending();
        } else if (iterator.depth() >= 1) {
            iterator.disable_recursion_pending();
        }
        iterator.increment(error);
    }
}
#endif

std::filesystem::path find_matlab_executable() {
    if (const char* configured =
            std::getenv(kMatlabExecutableEnvironment);
        configured != nullptr && configured[0] != '\0') {
        return configured;
    }

#ifdef __APPLE__
    std::vector<std::filesystem::path> candidates;
    collect_macos_matlab_installations("/Applications", candidates);
    if (const char* user_directory = std::getenv("HOME");
        user_directory != nullptr && user_directory[0] != '\0') {
        collect_macos_matlab_installations(
            std::filesystem::path(user_directory) / "Applications",
            candidates);
    }
    if (!candidates.empty()) {
        return *std::max_element(
            candidates.begin(),
            candidates.end(),
            [](const auto& left, const auto& right) {
                return left.parent_path().parent_path().filename().string()
                    < right.parent_path().parent_path().filename().string();
            });
    }
#endif

    return "matlab";
}

std::string matlab_string_literal(const std::filesystem::path& path) {
    std::string literal{"'"};
    for (char character : absolute_path(path).string()) {
        literal += character;
        if (character == '\'') {
            literal += '\'';
        }
    }
    literal += '\'';
    return literal;
}

int run_process(
    const std::filesystem::path& executable,
    const std::vector<std::string>& arguments) {
    std::vector<std::string> storage;
    storage.reserve(arguments.size() + 1);
    storage.push_back(executable.string());
    storage.insert(storage.end(), arguments.begin(), arguments.end());

#ifdef _WIN32
    std::vector<const char*> argv;
    argv.reserve(storage.size() + 1);
    for (const std::string& argument : storage) {
        argv.push_back(argument.c_str());
    }
    argv.push_back(nullptr);
    const intptr_t result = _spawnvp(
        _P_WAIT,
        executable.string().c_str(),
        argv.data());
    if (result == -1) {
        throw std::runtime_error(
            "cannot start MATLAB executable '" + executable.string()
            + "': " + std::strerror(errno));
    }
    return static_cast<int>(result);
#else
    std::vector<char*> argv;
    argv.reserve(storage.size() + 1);
    for (std::string& argument : storage) {
        argv.push_back(argument.data());
    }
    argv.push_back(nullptr);

    pid_t child = 0;
    const int spawn_error = posix_spawnp(
        &child,
        executable.string().c_str(),
        nullptr,
        nullptr,
        argv.data(),
        environ);
    if (spawn_error != 0) {
        throw std::runtime_error(
            "cannot start MATLAB executable '" + executable.string()
            + "': " + std::strerror(spawn_error)
            + "; add MATLAB to PATH or set "
            + kMatlabExecutableEnvironment);
    }

    int status = 0;
    while (waitpid(child, &status, 0) == -1) {
        if (errno != EINTR) {
            throw std::runtime_error(
                "failed while waiting for MATLAB: "
                + std::string(std::strerror(errno)));
        }
    }
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) {
        throw std::runtime_error(
            "MATLAB was terminated by signal "
            + std::to_string(WTERMSIG(status)));
    }
    throw std::runtime_error("MATLAB ended without an exit status");
#endif
}
} // namespace

void run_matlab_plot(
    const std::filesystem::path& csv_path,
    const std::filesystem::path& image_path) {
    const std::filesystem::path script_directory =
        configured_script_directory();
    const std::filesystem::path plot_script =
        script_directory / "plot_transient.m";
    if (!std::filesystem::is_regular_file(plot_script)) {
        throw std::runtime_error(
            "cannot find MATLAB plot script '" + plot_script.string()
            + "'; set " + kMatlabScriptDirectoryEnvironment);
    }

    const std::string statement =
        "addpath(" + matlab_string_literal(script_directory)
        + "); plot_transient(" + matlab_string_literal(csv_path)
        + ", [], " + matlab_string_literal(image_path) + ")";
    const std::filesystem::path executable = find_matlab_executable();
    const int exit_code = run_process(executable, {"-batch", statement});
    if (exit_code != 0) {
        throw std::runtime_error(
            "MATLAB plotting failed with exit code "
            + std::to_string(exit_code));
    }
}
