#pragma once
#include <indicators/progress_spinner.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include "parser.hpp"
#include <indicators/termcolor.hpp>
#include <indicators/cursor_control.hpp>
#include <atomic>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <functional>

using namespace indicators;

using namespace std::chrono;
struct PageIRInfo  {
    bool isbackend;
    AST_NODE *IR;
    bool useclient;
};

// ---------- Stage Runner ----------
inline long RunStage(const std::string& message, std::function<void()> task) {
    
    indicators::show_console_cursor(false);
    ProgressSpinner spinner{
        option::Stream{std::cout}, 
        option::PostfixText{" " + message},
        option::ForegroundColor{Color::yellow},
        option::ShowPercentage{false},
        option::SpinnerStates{
            std::vector<std::string>{
                "⠋","⠙","⠹","⠸","⠼",
                "⠴","⠦","⠧","⠇","⠏"
            }
        },
        option::FontStyles{
            std::vector<FontStyle>{FontStyle::bold}
        }
    };

    std::atomic<bool> done = false;

    auto start = high_resolution_clock::now();

    std::thread spin_thread([&]() {
        while (!done.load()) {
            spinner.tick();
            std::this_thread::sleep_for(std::chrono::milliseconds(40));
        }
    });

    // Run actual work
    task();

    done = true;
    spin_thread.join();

    auto end = high_resolution_clock::now();
    long duration = duration_cast<milliseconds>(end - start).count();

    spinner.set_option(option::ForegroundColor{Color::green});
    spinner.set_option(option::PrefixText{"✔"});
    spinner.set_option(option::ShowSpinner{false});
    spinner.set_option(option::PostfixText{
        " " + message + " (" + std::to_string(duration) + "ms)"
    });

    spinner.mark_as_completed();
    indicators::show_console_cursor(true);
    return duration;
}

inline long RunHotReloadStage(bool& buildErr, std::string& errStr, std::function<void()> task) {
    indicators::show_console_cursor(false);
    ProgressSpinner spinner{
        option::Stream{std::cout}, 
        option::PostfixText{" Rebuilding..."},
        option::ForegroundColor{Color::cyan},
        option::ShowPercentage{false},
        option::SpinnerStates{
            std::vector<std::string>{
                "⠋","⠙","⠹","⠸","⠼",
                "⠴","⠦","⠧","⠇","⠏"
            }
        }
    };

    std::atomic<bool> done = false;
    auto start = high_resolution_clock::now();

    std::thread spin_thread([&]() {
        while (!done.load()) {
            spinner.tick();
            std::this_thread::sleep_for(std::chrono::milliseconds(40));
        }
    });

    std::ostringstream oss;
    std::streambuf* oldCerr = std::cerr.rdbuf(oss.rdbuf());

    try {
        task();
        buildErr = false;
    } catch (const std::exception& e) {
        std::cerr.rdbuf(oldCerr);

        std::cerr << "err: " << oss.str() << std::endl;
        // Capture all output to errStr
        errStr = oss.str();
        buildErr = true;  
    }

    done = true;
    spin_thread.join();

    auto end = high_resolution_clock::now();
    long duration = duration_cast<milliseconds>(end - start).count();

    if (buildErr) {
        spinner.set_option(option::ForegroundColor{Color::red});
        spinner.set_option(option::PrefixText{"✖"});
        spinner.set_option(option::ShowSpinner{false});
        spinner.set_option(option::PostfixText{
            " Build failed (" + std::to_string(duration) + "ms)"
        });
    } else {
        spinner.set_option(option::ForegroundColor{Color::green});
        spinner.set_option(option::PrefixText{"✔"});
        spinner.set_option(option::ShowSpinner{false});
        spinner.set_option(option::PostfixText{
            " Rebuilt successfully (" + std::to_string(duration) + "ms)"
        });
    }

    spinner.mark_as_completed();
    indicators::show_console_cursor(true);
    return duration;
}

