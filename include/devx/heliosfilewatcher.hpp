#ifndef __FWATCHER_H
#define __FWATCHER_H

#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <functional>
#include <mutex>

namespace fs = std::filesystem;

/* ================= FILE WATCHER ================= */
class FileWatcher {
public:
    FileWatcher(std::string path, std::string ext, std::function<void()> cb)
        : root_(std::move(path)), ext_(std::move(ext)), callback_(std::move(cb)) {}

    void start() {
        running_ = true;
        std::thread([this]{ loop(); }).detach();
    }

    void stop() {
        running_ = false;
    }

private:
    void loop() {
        std::unordered_map<std::string, fs::file_time_type> last_write;
        bool cooling_down = false;

        while (running_) {
            bool changed = false;

            for (auto& entry : fs::recursive_directory_iterator(root_)) {
                if (!entry.is_regular_file()) continue;
                if (entry.path().extension() != ext_) continue;

                auto path = entry.path().string();
                auto write_time = fs::last_write_time(entry);

                if (!last_write.contains(path) || last_write[path] != write_time) {
                    last_write[path] = write_time;
                    changed = true;
                }
            }

            if (changed && !cooling_down) {
                if (callback_) callback_();
                cooling_down = true;

                // cooldown to prevent memory spike
                std::thread([&cooling_down]{
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                    cooling_down = false;
                }).detach();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }

    std::string root_;
    std::string ext_;
    std::function<void()> callback_;
    std::atomic<bool> running_{false};
};

#endif