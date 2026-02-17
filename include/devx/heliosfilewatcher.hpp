#ifndef __FWATCHER_H
#define __FWATCHER_H

#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <thread>
#include <atomic>
#include <chrono>
#include <string>
#include <mutex>
#include <vector>

namespace fs = std::filesystem;

/* ================= FILE WATCHER ================= */
class FileWatcher {
public:
    using Callback = std::function<void(const std::string&)>;

    FileWatcher(std::string path,
                std::initializer_list<std::string> extensions,
                Callback callback,
                std::chrono::milliseconds interval = std::chrono::milliseconds(300),
                std::chrono::milliseconds debounce = std::chrono::milliseconds(800))
        : root_(std::move(path)),
          exts_(extensions),
          callback_(std::move(callback)),
          interval_(interval),
          debounce_(debounce) {}

    ~FileWatcher() {
        stop();
    }

    void start() {
        if (running_) return;

        running_ = true;
        buildInitialSnapshot(); // No trigger here
        watcher_thread_ = std::thread([this] { watchLoop(); });
    }

    void stop() {
        running_ = false;
        if (watcher_thread_.joinable())
            watcher_thread_.join();
    }

private:
    bool matchesExtension(const fs::path& path) const {
        return exts_.contains(path.extension().string());
    }

    void buildInitialSnapshot() {
        std::lock_guard<std::mutex> lock(mutex_);
        files_.clear();

        for (auto& entry : fs::recursive_directory_iterator(root_)) {
            if (!entry.is_regular_file()) continue;
            if (!matchesExtension(entry.path())) continue;

            files_[entry.path().string()] =
                fs::last_write_time(entry);
        }
    }

    void watchLoop() {
        auto last_trigger = std::chrono::steady_clock::now();

        while (running_) {
            bool changed = false;
            std::vector<std::string> changed_exts;

            std::unordered_map<std::string, fs::file_time_type> current_files;

            for (auto& entry : fs::recursive_directory_iterator(root_)) {
                if (!entry.is_regular_file()) continue;
                if (!matchesExtension(entry.path())) continue;

                auto path = entry.path().string();
                auto ext  = entry.path().extension().string();
                auto write_time = fs::last_write_time(entry);

                current_files[path] = write_time;

                if (!files_.contains(path) || files_[path] != write_time) {
                    changed = true;
                    changed_exts.push_back(ext);
                }
            }

            // Detect deleted files
            for (auto& [path, _] : files_) {
                if (!current_files.contains(path)) {
                    auto ext = fs::path(path).extension().string();
                    if (exts_.contains(ext)) {
                        changed = true;
                        changed_exts.push_back(ext);
                    }
                }
            }

            if (changed) {
                auto now = std::chrono::steady_clock::now();

                if (now - last_trigger > debounce_) {
                    {
                        std::lock_guard<std::mutex> lock(mutex_);
                        files_ = std::move(current_files);
                    }

                    if (callback_) {
                        for (const auto& ext : changed_exts) {
                            callback_(ext);  // ONE callback, different behavior by ext
                        }
                    }

                    last_trigger = now;
                }
            }

            std::this_thread::sleep_for(interval_);
        }
    }

private:
    std::string root_;
    std::unordered_set<std::string> exts_;
    Callback callback_;

    std::unordered_map<std::string, fs::file_time_type> files_;

    std::chrono::milliseconds interval_;
    std::chrono::milliseconds debounce_;

    std::atomic<bool> running_{false};
    std::thread watcher_thread_;
    std::mutex mutex_;
};

#endif
