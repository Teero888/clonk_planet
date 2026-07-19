#include "C4Group.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <ctime>
#include <zlib.h>
#include <sys/stat.h>

#ifdef _WIN32
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
#endif
#endif

#include <dirent.h>

C4Group::C4Group() {}

C4Group::C4Group(const std::string &path) {
    loadFromFile(path);
}

C4Group::C4Group(const std::vector<uint8_t> &raw_data) {
    loadFromMemory(raw_data);
}

C4Group::C4Group(const uint8_t *data, size_t size) {
    loadFromMemory(data, size);
}

bool C4Group::loadFromDirectory(const std::string &dir_path) {
    C4GroupWriter writer;
    DIR *dir = opendir(dir_path.c_str());
    if (!dir) return false;

    struct dirent *ent;
    while ((ent = readdir(dir)) != nullptr) {
        std::string ename = ent->d_name;
        if (ename == "." || ename == "..") continue;

        std::string full_path = dir_path + "/" + ename;
        struct stat st;
        if (stat(full_path.c_str(), &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                C4Group sub;
                if (sub.loadFromDirectory(full_path)) {
                    writer.addFile(ename, sub.getRawData());
                }
            } else {
                std::ifstream f(full_path, std::ios::binary);
                if (f.is_open()) {
                    std::vector<uint8_t> f_data(st.st_size);
                    f.read(reinterpret_cast<char*>(f_data.data()), st.st_size);
                    f.close();
                    writer.addFile(ename, f_data);
                }
            }
        }
    }
    closedir(dir);

    std::vector<uint8_t> mem_blob = writer.makeMemoryBlob();
    return loadFromMemory(mem_blob);
}

bool C4Group::loadFromFile(const std::string &path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return loadFromDirectory(path);
        }
    }

    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return false;

    f.seekg(0, std::ios::end);
    auto tell_size = f.tellg();
    if (tell_size < 0) return false;
    size_t size = tell_size;
    f.seekg(0, std::ios::beg);

    std::vector<uint8_t> raw_data(size);
    f.read(reinterpret_cast<char*>(raw_data.data()), size);
    f.close();

    return loadFromMemory(raw_data);
}

bool C4Group::loadFromMemory(const std::vector<uint8_t> &raw_data) {
    return loadFromMemory(raw_data.data(), raw_data.size());
}

bool C4Group::loadFromMemory(const uint8_t *data_ptr, size_t size) {
    if (size < 2) return false;

    // Check for Gzip compression (standard 1F 8B or scrambled 1E 8C)
    if ((data_ptr[0] == 0x1f && data_ptr[1] == 0x8b) || 
        (data_ptr[0] == 0x1e && data_ptr[1] == 0x8c)) {
        packed_ = true;
        std::vector<uint8_t> src(data_ptr, data_ptr + size);
        if (src[0] == 0x1e && src[1] == 0x8c) {
            src[0] ^= 1;
            src[1] ^= 7;
        }
        if (!decompressGzip(src, this->data)) {
            std::cerr << "Failed to decompress C4Group gzip data" << std::endl;
            return false;
        }
    } else {
        packed_ = false;
        this->data.assign(data_ptr, data_ptr + size);
    }

    if (this->data.size() < 204) return false;

    std::vector<uint8_t> header(this->data.begin(), this->data.begin() + 204);
    unscramble(header.data(), header.size());

    if (std::memcmp(header.data(), "RedWolf Design GrpFolder", 24) != 0) {
        return false;
    }

    int32_t num_entries = 0;
    std::memcpy(&num_entries, &header[36], 4);

    size_t entry_base = 204;
    size_t file_base = 204 + num_entries * 316;

    if (this->data.size() < file_base) return false;

    entries.clear();
    for (int i = 0; i < num_entries; ++i) {
        const uint8_t *e_ptr = &this->data[entry_base + i * 316];

        char name_buf[261];
        std::memcpy(name_buf, e_ptr, 260);
        name_buf[260] = '\0';
        std::string name(name_buf);

        int32_t packed = 0, child_group = 0;
        std::memcpy(&packed, e_ptr + 260, 4);
        std::memcpy(&child_group, e_ptr + 264, 4);

        int32_t fsize = 0, entry_size = 0, offset = 0;
        std::memcpy(&fsize, e_ptr + 268, 4);
        std::memcpy(&entry_size, e_ptr + 272, 4);
        std::memcpy(&offset, e_ptr + 276, 4);

        int32_t time_val = 0;
        std::memcpy(&time_val, e_ptr + 280, 4);

        C4GroupEntry entry;
        entry.name = name;
        entry.size = fsize;
        entry.entry_size = entry_size;
        entry.offset = file_base + offset;
        entry.packed = (packed != 0);
        entry.is_group = (child_group != 0);
        entry.time = time_val;

        entries.push_back(entry);
    }

    return true;
}

std::vector<uint8_t> C4Group::getFile(const std::string &name) const {
    std::string lower_name = name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);

    for (const auto &e : entries) {
        std::string cur_name = e.name;
        std::transform(cur_name.begin(), cur_name.end(), cur_name.begin(), ::tolower);
        if (cur_name == lower_name) {
            if (e.offset + e.size <= data.size()) {
                return std::vector<uint8_t>(data.begin() + e.offset, data.begin() + e.offset + e.size);
            }
        }
    }
    return {};
}

std::string C4Group::getFileAsString(const std::string &name) const {
    auto file_data = getFile(name);
    return std::string(reinterpret_cast<const char*>(file_data.data()), file_data.size());
}

void C4Group::unscramble(uint8_t *data, size_t size) {
    if (size < 2) return;
    for (size_t i = 0; i < size - 2; i += 3) {
        std::swap(data[i], data[i+2]);
    }
    for (size_t i = 0; i < size; ++i) {
        data[i] ^= 237;
    }
}

void C4Group::scramble(uint8_t *data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        data[i] ^= 237;
    }
    if (size < 2) return;
    for (size_t i = 0; i < size - 2; i += 3) {
        std::swap(data[i], data[i+2]);
    }
}

bool C4Group::decompressGzip(const std::vector<uint8_t> &src, std::vector<uint8_t> &dest) {
    if (src.size() < 18) return false;

    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = src.size();
    strm.next_in = const_cast<uint8_t*>(src.data());

    if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK) return false;

    dest.resize(src.size() * 3);
    strm.avail_out = dest.size();
    strm.next_out = dest.data();

    while (true) {
        int ret = inflate(&strm, Z_NO_FLUSH);
        if (ret == Z_STREAM_END) {
            dest.resize(strm.total_out);
            inflateEnd(&strm);
            return true;
        }
        if (ret != Z_OK) {
            inflateEnd(&strm);
            return false;
        }
        size_t old_size = dest.size();
        dest.resize(old_size * 2);
        strm.avail_out = dest.size() - old_size;
        strm.next_out = dest.data() + old_size;
    }
}

C4GroupWriter::C4GroupWriter() : maker("Gemini Launcher") {}

void C4GroupWriter::addFile(const std::string &name, const std::vector<uint8_t> &data, bool packed) {
    WriteEntry entry;
    entry.name = name;
    entry.data = data;
    entry.packed = packed;
    entry.time = std::time(nullptr);
    entries.push_back(entry);
}

void C4GroupWriter::addFromGroup(const C4Group &source_grp, const std::vector<std::string> &exclude) {
    for (const auto &e : source_grp.getEntries()) {
        bool skip = false;
        for (const auto &ex : exclude) {
            std::string cur_ex = ex;
            std::string cur_name = e.name;
            std::transform(cur_ex.begin(), cur_ex.end(), cur_ex.begin(), ::tolower);
            std::transform(cur_name.begin(), cur_name.end(), cur_name.begin(), ::tolower);
            if (cur_name == cur_ex) {
                skip = true;
                break;
            }
        }
        if (!skip) {
            auto file_data = source_grp.getFile(e.name);
            if (!file_data.empty()) {
                addFile(e.name, file_data, e.packed);
            }
        }
    }
}

std::vector<uint8_t> C4GroupWriter::makeHeader() {
    std::vector<uint8_t> header(204, 0);
    const char *id_str = "RedWolf Design GrpFolder";
    std::memcpy(header.data(), id_str, 24);

    int32_t ver1 = 1, ver2 = 2, num_entries = entries.size();
    std::memcpy(&header[28], &ver1, 4);
    std::memcpy(&header[32], &ver2, 4);
    std::memcpy(&header[36], &num_entries, 4);

    std::string m = maker.substr(0, 31);
    std::memcpy(&header[40], m.c_str(), m.size());

    int32_t time_val = std::time(nullptr);
    int32_t orig_val = 1;
    std::memcpy(&header[104], &time_val, 4);
    std::memcpy(&header[108], &orig_val, 4);

    C4Group::scramble(header.data(), header.size());
    return header;
}

std::vector<uint8_t> C4GroupWriter::makeEntryCore(const WriteEntry &entry, uint32_t offset, bool &child_group) {
    std::vector<uint8_t> core(316, 0);
    std::string name = entry.name.substr(0, 259);
    std::memcpy(core.data(), name.c_str(), name.size());

    int32_t packed = entry.packed ? 1 : 0;
    std::string lower_name = entry.name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);

    child_group = false;
    if (lower_name.size() >= 4) {
        std::string ext = lower_name.substr(lower_name.size() - 4);
        if (ext == ".c4p" || ext == ".c4f" || ext == ".c4s" || ext == ".c4d" || ext == ".c4v") {
            child_group = true;
        }
    }
    int32_t child_val = child_group ? 1 : 0;
    int32_t size = entry.data.size();
    int32_t entry_size = size;
    int32_t time_val = entry.time;

    std::memcpy(&core[260], &packed, 4);
    std::memcpy(&core[264], &child_val, 4);
    std::memcpy(&core[268], &size, 4);
    std::memcpy(&core[272], &entry_size, 4);
    std::memcpy(&core[276], &offset, 4);
    std::memcpy(&core[280], &time_val, 4);

    return core;
}

std::vector<uint8_t> C4GroupWriter::makeMemoryBlob() {
    std::vector<uint8_t> output;
    std::vector<std::vector<uint8_t>> entry_cores;
    uint32_t current_offset = 0;

    for (const auto &entry : entries) {
        bool child_group = false;
        entry_cores.push_back(makeEntryCore(entry, current_offset, child_group));
        current_offset += entry.data.size();
    }

    auto header = makeHeader();
    output.insert(output.end(), header.begin(), header.end());

    for (const auto &core : entry_cores) {
        output.insert(output.end(), core.begin(), core.end());
    }

    for (const auto &entry : entries) {
        output.insert(output.end(), entry.data.begin(), entry.data.end());
    }
    return output;
}

bool C4GroupWriter::writeToFile(const std::string &path, bool compress) {
    std::vector<uint8_t> output = makeMemoryBlob();

    if (compress) {
        std::vector<uint8_t> compressed;
        if (!compressGzip(output, compressed)) return false;

        // Apply packed marker
        compressed[0] ^= 1;
        compressed[1] ^= 7;

        std::ofstream f(path, std::ios::binary);
        if (!f.is_open()) return false;
        f.write(reinterpret_cast<const char*>(compressed.data()), compressed.size());
        f.close();
    } else {
        std::ofstream f(path, std::ios::binary);
        if (!f.is_open()) return false;
        f.write(reinterpret_cast<const char*>(output.data()), output.size());
        f.close();
    }

    return true;
}

bool C4GroupWriter::compressGzip(const std::vector<uint8_t> &src, std::vector<uint8_t> &dest) {
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = src.size();
    strm.next_in = const_cast<uint8_t*>(src.data());

    if (deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) return false;

    dest.resize(src.size() + 100);
    strm.avail_out = dest.size();
    strm.next_out = dest.data();

    int ret = deflate(&strm, Z_FINISH);
    if (ret != Z_STREAM_END) {
        deflateEnd(&strm);
        return false;
    }
    dest.resize(strm.total_out);
    deflateEnd(&strm);
    return true;
}
