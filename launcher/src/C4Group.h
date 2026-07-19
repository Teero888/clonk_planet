#pragma once

#include <string>
#include <vector>
#include <cstdint>

struct C4GroupEntry {
    std::string name;
    uint32_t size = 0;
    uint32_t entry_size = 0;
    uint32_t offset = 0;
    bool packed = false;
    bool is_group = false;
    uint32_t time = 0;
};

class C4Group {
public:
    C4Group();
    explicit C4Group(const std::string &path);
    explicit C4Group(const std::vector<uint8_t> &raw_data);
    C4Group(const uint8_t *data, size_t size);

    bool loadFromFile(const std::string &path);
    bool loadFromDirectory(const std::string &dir_path);
    bool loadFromMemory(const std::vector<uint8_t> &raw_data);
    bool loadFromMemory(const uint8_t *data, size_t size);

    std::vector<uint8_t> getFile(const std::string &name) const;
    std::string getFileAsString(const std::string &name) const;

    const std::vector<C4GroupEntry>& getEntries() const { return entries; }
    bool isPacked() const { return packed_; }
    const std::vector<uint8_t>& getRawData() const { return data; }

public:
    static void unscramble(uint8_t *data, size_t size);
    static void scramble(uint8_t *data, size_t size);
    static bool decompressGzip(const std::vector<uint8_t> &src, std::vector<uint8_t> &dest);

private:
    std::vector<C4GroupEntry> entries;
    std::vector<uint8_t> data;
    bool packed_ = false;
};

class C4GroupWriter {
public:
    struct WriteEntry {
        std::string name;
        std::vector<uint8_t> data;
        bool packed = false;
        uint32_t time = 0;
    };

    C4GroupWriter();
    void addFile(const std::string &name, const std::vector<uint8_t> &data, bool packed = false);
    void addFromGroup(const C4Group &source_grp, const std::vector<std::string> &exclude = {});
    bool writeToFile(const std::string &path, bool compress = false);
    std::vector<uint8_t> makeMemoryBlob();

private:
    std::vector<WriteEntry> entries;
    std::string maker;

    std::vector<uint8_t> makeHeader();
    std::vector<uint8_t> makeEntryCore(const WriteEntry &entry, uint32_t offset, bool &child_group);
    static bool compressGzip(const std::vector<uint8_t> &src, std::vector<uint8_t> &dest);
};
