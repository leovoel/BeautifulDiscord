#pragma once

#include "jsonpp.hpp"
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>

#include <boost/filesystem.hpp>

namespace bd {
namespace fs = ::boost::filesystem;

struct byte {
    char x;
    byte() {}

    operator char() noexcept {
        return x;
    }

    operator char&() noexcept {
        return x;
    }

    operator const char&() const noexcept {
        return x;
    }
};

struct already_extracted : public std::exception {
    const char* what() const noexcept override { return "file extracted already"; }
};

struct asar {
    asar(const char* path): in(path, std::ios::binary) {
        if(!in.is_open()) {
            throw std::runtime_error("could not open file.");
        }

        // discard the first 12 bytes since we don't care about them
        in.ignore(12);
        unsigned char bytes[4];
        if(!in.read(reinterpret_cast<char*>(bytes), sizeof(bytes))) {
            throw std::runtime_error("could not get header string size");
        }

        // little endian 32-bit integer
        int size = (bytes[0] << 0) | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
        std::string str;
        str.reserve(size);

        if(!in.read(&str[0], size)) {
            throw std::runtime_error("could not fetch header JSON");
        }

        auto&& v = json::parse(str);
        header = v.as<json::object>();
        base_offset = round_up(16 + size, 4);
    }

    void extract(const fs::path& path) {
        if(fs::exists(path)) {
            throw already_extracted();
        }

        extract_directory(".", header.at("files").get<json::object>(), path);
    }

private:
    json::object header;
    std::ifstream in;
    int base_offset;

    static int round_up(int from, int multiple) noexcept {
        return (from + multiple - 1) & ~(multiple - 1);
    }

    void extract_directory(const fs::path& source, const json::object& files, const fs::path& path) {
        auto dest = path / source;

        fs::create_directories(dest);

        for(auto&& elem : files) {
            auto p = source / elem.first;
            auto&& info = elem.second.get<json::object>();

            auto it = info.find("files");
            if(it != info.end()) {
                extract_directory(p, it->second.get<json::object>(), path);
                continue;
            }

            extract_file(p, info, path);
        }
    }

    void extract_file(const fs::path& source, const json::object& info, const fs::path& dest) {
        auto offset = base_offset + std::stoi(info.at("offset").get<std::string>());
        in.seekg(offset);

        std::vector<byte> bytes(info.at("size").get<double>());
        in.read(reinterpret_cast<char*>(bytes.data()), bytes.size());

        auto dest_path = (dest / source).string();
        std::ofstream out(dest_path);
        out.write(reinterpret_cast<char*>(bytes.data()), bytes.size());
    }
};
} // bd
