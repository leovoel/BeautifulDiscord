#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <process.hpp>
#include <asar.hpp>
#include <iostream>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <algorithm>

namespace fs = boost::filesystem;

void replace(std::string& str, const std::string& from, const std::string& to) {
    auto pos = str.find(from);
    if(pos == str.npos)
        return;
    str.replace(pos, from.length(), to);
}

struct discord_process {
    std::vector<bd::process> processes;
    std::string filename;
    fs::path exe;

    discord_process(fs::path exe): filename(exe.filename().string()), exe(std::move(exe)) {}

    void launch() {
#if BD_POSIX
        auto cmd = '"' + exe.string() + "\" > /dev/null 2>&1 &";
#else
        auto cmd = "start /b cmd /c call \"" + exe.string() + "\" >NUL 2>&1";
#endif
        std::system(cmd.c_str());
    }

    fs::path resources() {
#if BD_OSX
        // OS X has a different resources path
        // Application directory is under <[EXE].app/Contents/MacOS/[EXE]>
        // where [EXE] is Discord Canary, Discord PTB, etc
        // Resources directory is under </Applications/[EXE].app/Contents/Resources/app.asar>
        // So we need to fetch the folder based on the executable path.
        auto filename = exe.filename();
        auto _exe = filename;
        filename.replace_extension(".app");
        return fs::path("/Applications") / filename / "Contents/Resources";
#else
        return exe.parent_path() / "resources";
#endif
    }

    void terminate() {
        std::sort(processes.begin(), processes.end(), [](const bd::process& lhs, const bd::process& rhs) {
            return lhs.id < rhs.id;
        });

        for(auto&& p : processes) {
            try {
                p.kill();
            }
            catch(const bd::process_error& e) {
                std::cerr << "warning: could not terminate PID " << p.id << ": " << e.what() << '\n';
            }
        }
    }
};

struct argument_parser {
    argument_parser() = default;

    using arguments = std::vector<std::string>;

    void parse(int argc, char** argv) {
        name = argv[0];
        arguments args(argv + 1, argv + argc);

        for(unsigned i = 0; i < args.size(); ++i) {
            auto&& arg = args[i];
            if(arg.find("--help") == 0) {
                show_help();
            }
            else if(arg.find("--version") == 0) {
                show_version();
            }
            else if(arg.find("--css") == 0) {
                parse_css(args, arg, i);
            }
            else if(arg.find("--revert") == 0) {
                revert = true;
            }
        }
    }

    void show_help() {
        show_usage(std::cout);
        std::cout << std::endl;
        std::cout << "Unpacks Discord and adds CSS hot-reloading.\n\n"
                     "Discord has to be open for this to work. When this tool is ran,\n"
                     "Discord will close and then be relaunched when the tool completes.\n\n";

        std::cout << "optional arguments:\n"
                     "  -h, --help     shows this message and exits\n"
                     "  -v, --version  shows the executable version and exits\n"
                     "  --css <file>   location of the CSS file to load\n"
                     "  --revert       reverts any changes done to discord\n";

        std::exit(EXIT_SUCCESS);
    }

    void show_version() {
        std::cout << name.filename().replace_extension().string() << " 1.0.0" << std::endl;
        std::exit(EXIT_SUCCESS);
    }

    void show_error(const char* err) {
        show_usage(std::cerr);
        std::cerr << "error: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }

    template<typename Stream>
    void show_usage(Stream& stream) {
        stream << "usage: " << name.filename().replace_extension().string() << " [--css <file>] [--revert]\n";
    }

    void parse_css(const arguments& args, const std::string& arg, unsigned index) {
        // check if it is --css="location here"
        auto pos = arg.find('=', 5);
        if(pos != arg.npos) {
            if(pos + 1 > arg.size()) {
                show_error("missing file for --css");
            }

            css = arg.substr(pos + 1);
        }
        else {
            if(index + 1 > args.size()) {
                show_error("missing file for --css");
            }

            css = args[index + 1];
        }

        css = fs::absolute(css);
    }
public:
    fs::path css;
    fs::path name;
    bool revert = false;
};

discord_process get_discord() {
    std::vector<discord_process> executables;

    for(auto&& process : bd::enumerate_processes()) {
        try {
            auto exe = process.exe();
            auto name = exe.filename().string();

            if(name.find("Discord") == 0 && name.find("Helper") == name.npos) {
                auto it = std::find_if(executables.begin(), executables.end(), [&name](const discord_process& e) {
                    return name == e.filename;
                });

                if(it == executables.end()) {
                    executables.emplace_back(std::move(exe));
                    executables.back().processes.push_back(std::move(process));
                }
                else {
                    it->processes.push_back(std::move(process));
                }
            }
        }
        catch(const bd::process_error&) {
            // no problem
        }
    }

    if(executables.empty()) {
        throw std::runtime_error("cannot find Discord executable running");
    }

    if(executables.size() == 1) {
        auto&& ret = executables.back();
        std::cout << "info: found " << ret.filename << '\n';
        return ret;
    }

    std::cout << "info: found " << executables.size() << " different versions of discord\n";
    for(unsigned i = 0; i < executables.size(); ++i) {
        std::cout << i << ": found " << executables[i].filename << '\n';
    }

    while(true) {
        std::cout << "discord executable to use (number): ";
        int index;
        if(!(std::cin >> index) || index < 0 || static_cast<unsigned>(index) >= executables.size()) {
            std::cout << "invalid index passed\n";
            continue;
        }

        return executables[index];
    }
}

void revert() {
    boost::system::error_code ec;
    fs::remove_all("app/", ec);
    if(ec) {
        std::cout << "no changes to revert\n";
        return;
    }

    fs::rename("original_app.asar", "app.asar");
    std::cout << "reverted changes, no more CSS hot-reload :(\n";
}

void extract_asar() {
    try {
        bd::asar a("app.asar");
        try {
            a.extract("app/");
        }
        catch(const bd::already_extracted& e) {
            std::cout << "asar already extracted, overwrite? [Y/n]: ";
            char c;
            if(!(std::cin >> c) || c == 'n' || c == 'N') {
                std::cout << "exiting\n";
                std::exit(EXIT_SUCCESS);
            }
            boost::system::error_code ec; // ignored
            fs::remove_all("app/", ec);
            a.extract("app/");
        }
    }
    catch(const boost::system::system_error& e) {
        std::cerr << "error: " << e.what() << '\n';
        std::exit(EXIT_FAILURE);
    }
    catch(const std::exception& e) {
        std::cout << "warning: app.asar not found\n";
        return;
    }

    fs::rename("app.asar", "original_app.asar");
}

void extract(const fs::path& css) {
    extract_asar();
    fs::current_path(fs::current_path() / "app");

    if(!fs::exists(css)) {
        fs::ofstream out(css);
        out << "/* put your custom css here. */\n";
    }

    const char injection[] = R"script(
window._fs = require("fs");
window._fileWatcher = null;
window._styleTag = null;

window.setupCSS = function(path) {
  var customCSS = window._fs.readFileSync(path, "utf-8");
  if(window._styleTag === null) {
    window._styleTag = document.createElement("style");
    document.head.appendChild(window._styleTag);
  }
  window._styleTag.innerHTML = customCSS;
  if(window._fileWatcher === null) {
    window._fileWatcher = window._fs.watch(path, { encoding: "utf-8" },
      function(eventType, filename) {
        if(eventType === "change") {
          var changed = window._fs.readFileSync(path, "utf-8");
          window._styleTag.innerHTML = changed;
        }
      }
    );
  }
};

window.tearDownCSS = function() {
  if(window._styleTag !== null) { window._styleTag.innerHTML = ""; }
  if(window._fileWatcher !== null) { window._fileWatcher.close(); window._fileWatcher = null; }
};

window.applyAndWatchCSS = function(path) {
  window.tearDownCSS();
  window.setupCSS(path);
};

)script";

    auto injection_path = fs::absolute("cssInjection.js");

    {
        std::ofstream out("cssInjection.js");
        out.write(injection, sizeof(injection) - 1);
        out << "window.applyAndWatchCSS('" << css.generic_string() << "');\n";

        if(!out) {
            throw std::runtime_error("could not write cssInjection.js");
        }
    }

    const char reload[] = R"reload(
mainWindow.webContents.on('dom-ready', function () {
  mainWindow.webContents.executeJavaScript(
)reload";

    std::ostringstream ss;

    {
        std::ifstream in("index.js");
        ss << in.rdbuf();
    }

    auto data = ss.str();
    ss.str(""); // clear it

    ss.write(reload, sizeof(reload) - 1);
    ss << "_fs2.default.readFileSync('" << injection_path.generic_string() << "', 'utf-8')\n"
    ");});";

    replace(data, "mainWindow.webContents.on('dom-ready', function () {});", ss.str());

    std::ofstream index("index.js");
    index.write(&data[0], data.size());

    if(!index) {
        throw std::runtime_error("could not write to index.js");
    }

    std::cout << "\nall done!\n"
                 "you may now edit your css file and it will automatically be reloaded whenever"
                 "\nit is saved.\n";
}

int main(int argc, char** argv) {
    try {
        argument_parser args;
        args.parse(argc, argv);

        auto discord = get_discord();
        auto resources = discord.resources();

        fs::current_path(resources);
        discord.terminate();

        if(args.revert) {
            revert();
        }
        else {
            if(args.css.empty()) {
                args.css = resources / "discord-custom.css";
            }

            args.css = fs::weakly_canonical(args.css);
            args.css.make_preferred();
            std::cout << "using css at location: " << args.css << '\n';
            extract(args.css);
        }

        std::cout << "relaunching discord now...\n";
        discord.launch();
        return EXIT_SUCCESS;
    }
    catch(const std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
