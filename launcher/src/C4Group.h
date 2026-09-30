#pragma once

#include <string>
#include <vector>
#include <cstdint>

// value of C4GroupHeader::Original for files shipped by RedWolf Design (engine/src/C4Group.cpp)
constexpr int32_t C4GroupOriginal = 1234567;

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
    // entry lookup (case insensitive), nullptr if missing
    const C4GroupEntry *findEntry(const std::string &name) const;
    bool hasEntry(const std::string &name) const { return findEntry(name) != nullptr; }

    const std::vector<C4GroupEntry>& getEntries() const { return entries; }
    bool isPacked() const { return packed_; }
    // group header: author shown in the launcher and the "original file" flag of RedWolf's packs
    const std::string &getMaker() const { return maker_; }
    bool isOriginal() const { return original_; }
    int32_t getCreation() const { return creation_; }
    const std::vector<uint8_t>& getRawData() const { return data; }

public:
    static void unscramble(uint8_t *data, size_t size);
    static void scramble(uint8_t *data, size_t size);
    static bool decompressGzip(const std::vector<uint8_t> &src, std::vector<uint8_t> &dest);

private:
    std::vector<C4GroupEntry> entries;
    std::vector<uint8_t> data;
    bool packed_ = false;
    std::string maker_;
    bool original_ = false;
    int32_t creation_ = 0;
};

class C4GroupWriter {
public:
    struct WriteEntry {
        std::string name;
        std::vector<uint8_t> data;
        bool packed = false;
        bool child_group = false;
        uint32_t time = 0;
    };

    C4GroupWriter();
    // copies maker, creation and original flag of an existing group
    void setHeaderFrom(const C4Group &source_grp);
    void setMaker(const std::string &m) { maker = m; }
    void setOriginal(bool o) { original = o; }

    void addFile(const std::string &name, const std::vector<uint8_t> &data, bool packed = false);
    void addEntry(const WriteEntry &entry) { entries.push_back(entry); }
    void addFromGroup(const C4Group &source_grp, const std::vector<std::string> &exclude = {});
    bool writeToFile(const std::string &path, bool compress = false);
    std::vector<uint8_t> makeMemoryBlob();

    // C4Group file names that are child groups
    static bool isGroupName(const std::string &name);

private:
    std::vector<WriteEntry> entries;
    std::string maker;
    int32_t creation = 0;
    bool original = false;

    std::vector<uint8_t> makeHeader();
    std::vector<uint8_t> makeEntryCore(const WriteEntry &entry, uint32_t offset);
    static bool compressGzip(const std::vector<uint8_t> &src, std::vector<uint8_t> &dest);
};
