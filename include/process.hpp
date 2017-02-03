#pragma once

/**
 * Things I need:
 *
 * - Enumerating a process
 * - Terminating a process
 * - Get process executable name
 * - Get process path
 */

#include "platform.hpp"

#include <boost/filesystem.hpp>
#include <boost/system.hpp>
#include <vector>
#include <string>
#include <utility>
#include <stdexcept>
#include <iterator>
#include <algorithm>
#include <memory>
#include <ctime>
#include <cctype>

#if BD_WINDOWS

#ifndef WIN23_LEAN_AND_MEAN
#define WIN23_LEAN_AND_MEAN
#endif

#ifdef PSAPI_VERSION
#undef PSAPI_VERSION
#endif

#define PSAPI_VERSION 1
#include <windows.h>
#include <psapi.h>
#endif // windows

#if BD_POSIX
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif // POSIX

#if BD_OSX
#include <sys/syslimits.h>
#include <libproc.h>
#endif

namespace bd {
namespace fs = ::boost::filesystem;
struct process_error : public std::runtime_error {
    process_error(std::string s): std::runtime_error(std::move(s)) {}
};

#if BD_WINDOWS
struct handle_deleter {
    using pointer = HANDLE;

    void operator()(HANDLE handle) const noexcept {
        CloseHandle(handle);
    }
};

using handle_ptr = std::unique_ptr<HANDLE, handle_deleter>;

struct process_impl {
    using id_type = DWORD;

    fs::path exe(id_type pid) {
        handle_ptr handle{OpenProcess(PROCESS_QUERY_INFORMATION, false, pid)};
        if(handle == nullptr) {
            throw process_error("could not fetch executable name for process");
        }
        wchar_t r[MAX_PATH];
        DWORD ret = GetProcessImageFileNameW(handle.get(), r, MAX_PATH);
        if(ret == 0) {
            throw process_error("could not fetch executable name for process");
        }

        return { std::wstring(r, ret) };
    }

    void kill(id_type pid) {
        handle_ptr handle{OpenProcess(PROCESS_TERMINATE, false, pid)};
        if(handle == nullptr || !TerminateProcess(handle.get(), 0)) {
            throw process_error("could not terminate this process");
        }
    }

    void terminate(id_type pid) {
        kill(pid);
    }

    // double create_time(id_type pid) {
    //     if(pid == 0 || pid == 4) {
    //         return boot_time();
    //     }

    //     handle_ptr handle{OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pid)};
    //     if(handle == nullptr) {
    //         throw process_error("process not found");
    //     }

    //     FILETIME created, ignore1, ignore2, ignore3;
    //     if(!GetProcessTimes(handle.get(), &created, &ignore1, &ignore2, &ignore3)) {
    //         throw process_error("could not fetch process creation time");
    //     }

    //     long long unix_time = static_cast<long long>(created.dwHighDateTime) << 32;
    //     unix_time += created.dwLowDateTime - 116444736000000000LL;
    //     unix_time /= 10000000;
    //     return unix_time;
    // }

    // double boot_time() {
    //     FILETIME filetime;
    //     GetSystemTimeAsFileTime(&filetime);

    //     // thanks to:
    //     // https://johnstewien.wordpress.com/2008/05/01/time-format-conversions
    //     long long ll = (static_cast<long long>(filetime.dwHighDateTime) << 32) + filetime.dwLowDateTime;
    //     time_t pt = (ll - 116444736000000000ULL) / 10000000ULL;

    //     auto uptime = GetTickCount64() / 1000.00;
    //     return static_cast<double>(pt) - uptime;
    // }
};
#elif BD_POSIX
struct process_impl {
    using id_type = pid_t;

    void send_signal(id_type pid, int sig) {
        if(pid != 0) {
            if(::kill(pid, sig) == -1) {
                throw process_error("could not send this signal to the process");
            }
        }
    }

    void kill(id_type pid) {
        send_signal(pid, SIGKILL);
    }

    void terminate(id_type pid) {
        send_signal(pid, SIGTERM);
    }

    fs::path exe(id_type pid) {
#if BD_LINUX
        boost::system::error_code ec;
        fs::path ret = fs::read_symlink(fs::path("/proc") / std::to_string(pid) / fs::path("exe"), ec);
        if(ec) {
            throw process_error("could not fetch process executable name");
        }
        return ret;
#elif BD_OSX
        char buf[PATH_MAX];
        int ret = proc_pidpath(pid, &buf, sizeof(buf));
        if(ret == 0) {
            throw process_error("could not fetch process executable name");
        }

        return { buf };
#endif
    }
};
#endif

struct process {
    using id_type = process_impl::id_type;
    process(id_type id) noexcept: id(id) {}

    fs::path exe() {
        return impl.exe(id);
    }

    void kill() {
        return impl.kill(id);
    }

    // idea: is_running with GetExitCodeProcess
public:
    id_type id;
private:
    process_impl impl;
};

// for use only in range-based for or algorithms/containers
// since this is a lazy generator that generates processes
// there is no support for operator->
template<typename State>
struct state_iterator {
    using value_type = process;
    using reference = value_type&;

    state_iterator(State* ptr) noexcept: state(ptr) {}

    bool operator==(const state_iterator& other) const noexcept {
        if(state == nullptr) {
            return other.state == nullptr;
        }
        return state->compare(other.state);
    }

    bool operator!=(const state_iterator& other) const noexcept {
        return !(*this == other);
    }

    state_iterator& operator++() noexcept {
        if(state) {
            state->next();
        }
        return *this; /* no op */
    }

    value_type operator*() {
        if(state) {
            return state->get();
        }

        throw process_error("could not fetch process");
    }
private:
    State* state;
};

#if BD_WINDOWS
struct process_state {
    process_state() {
        if(!EnumProcesses(processes, sizeof(processes), &process_count)) {
            throw process_error("cannot retrieve a list of processes.");
        }

        process_count /= sizeof(DWORD);

        // find acceptable position
        for(; index < process_count; ++index) {
            if(processes[index] != 0) {
                break;
            }
        }
    }

    void next() {
        do {
            ++index;
            if(processes[index] != 0) {
                break;
            }
        }
        while(index < process_count);
    }

    bool compare(const process_state* other) const {
        // check if we're doing it == end
        if(other == nullptr) {
            return index >= process_count;
        }
        // we're not so
        return index == other->index;
    }

    process get() {
        return { processes[index] };
    }
private:
    DWORD processes[1024];
    DWORD process_count;
    unsigned index = 0;
};
#endif // windows

#if BD_LINUX
struct process_state {
    process_state() {
        boost::system::error_code ec;
        begin = fs::directory_iterator(fs::path("/proc"), ec);
        if(ec) {
            throw process_error("cannot retrieve a list of processes.");
        }
    }

    void next() {
        for(; begin != end; ++begin) {
            try {
                auto p = begin->string();
                std::string::size_type pos;
                current_id = static_cast<pid_t>(std::stoi(p, &pos));

                if(pos != p.size()) {
                    continue;
                }
            }
            catch(...) {
                continue;
            }
        }
    }

    bool compare(const process_state* other) const noexcept {
        if(other == nullptr) {
            return begin == end;
        }
        return begin == other.begin;
    }

    process get() {
        return { current_id };
    }
private:
    fs::directory_iterator begin;
    fs::directory_iterator end;

    pid_t current_id;
};
#endif // linux

using process_iterator = state_iterator<process_state>;

struct process_range {
    process_range() = default;

    process_iterator begin() noexcept {
        return { &state };
    }

    process_iterator end() noexcept {
        return { nullptr };
    }
private:
    process_state state;
};

inline process_range enumerate_processes() {
    return {};
}
} // bd
