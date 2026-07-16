#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <zlib.h>
#include <string>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#define strcasecmp _stricmp
#endif

typedef uint8_t BYTE;

#pragma pack(push, 1)
struct C4GroupHeader {
  char id[28];
  int32_t Ver1, Ver2;
  int32_t Entries;
  char Maker[32];
  char Password[32];
  int32_t Creation, Original;
  uint8_t fbuf[92];
};

struct C4GroupEntryCore {
  char FileName[260];
  int32_t Packed, ChildGroup;
  int32_t Size, EntrySize, Offset;
  int32_t Time;
  uint8_t fbuf[32];
};
#pragma pack(pop)

void MemUnscramble(BYTE *bypBuffer, int iSize) {
  int cnt;
  BYTE temp;
  for (cnt = 0; cnt < iSize; cnt++)
    bypBuffer[cnt] ^= 237;
  for (cnt = 0; cnt + 2 < iSize; cnt += 3) {
    temp = bypBuffer[cnt];
    bypBuffer[cnt] = bypBuffer[cnt + 2];
    bypBuffer[cnt + 2] = temp;
  }
}

bool DecompressData(const std::vector<BYTE> &in, std::vector<BYTE> &out, int idx) {
  if (in.size() < 2)
    return false;
  char tmp_gz[256], tmp_out[256];
  snprintf(tmp_gz, sizeof(tmp_gz), "tmp_%d.gz", idx);
  snprintf(tmp_out, sizeof(tmp_out), "tmp_%d", idx);

  FILE *f = fopen(tmp_gz, "wb");
  if (!f)
    return false;
  fwrite(in.data(), 1, in.size(), f);
  fclose(f);

  if (in[0] == 0x1e && in[1] == 0x8c) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "python3 -c \"d=open('%s','rb').read(); "
             "open('%s','wb').write(bytes([d[0]^1, d[1]^7])+d[2:])\" && gzip -df %s",
             tmp_gz, tmp_gz, tmp_gz);
    if (system(cmd) == -1) {}
  } else {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "gzip -df %s", tmp_gz);
    if (system(cmd) == -1) {}
  }

  FILE *fout = fopen(tmp_out, "rb");
  if (!fout) {
    remove(tmp_gz);
    return false;
  }
  out.clear();
  BYTE buf[4096];
  size_t len;
  while ((len = fread(buf, 1, sizeof(buf), fout)) > 0) {
    size_t old = out.size();
    out.resize(old + len);
    memcpy(out.data() + old, buf, len);
  }
  fclose(fout);
  remove(tmp_out);
  remove(tmp_gz);
  return true;
}

void HexDump(const BYTE *data, size_t size) {
  for (size_t i = 0; i < size; i++) {
    if (i % 16 == 0)
      printf("\n%08zx: ", i);
    printf("%02x ", data[i]);
  }
  printf("\n");
}

int main(int argc, char **argv) {
  bool raw_mode = false;
  std::string full_path = "";

  // Parse arguments
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--raw") == 0) {
      raw_mode = true;
    } else {
      full_path = argv[i];
    }
  }

  if (full_path.empty()) {
    fprintf(stderr, "Usage: %s <path/to/resource> [--raw]\n", argv[0]);
    return 1;
  }

  std::vector<std::string> segments;
  size_t start = 0, end;
  while ((end = full_path.find('/', start)) != std::string::npos) {
    segments.push_back(full_path.substr(start, end - start));
    start = end + 1;
  }
  segments.push_back(full_path.substr(start));

  std::vector<BYTE> data;
  FILE *f = fopen(segments[0].c_str(), "rb");
  if (!f) {
    perror("fopen");
    return 1;
  }
  fseek(f, 0, SEEK_END);
  size_t fsize = ftell(f);
  fseek(f, 0, SEEK_SET);
  data.resize(fsize);
  if (fread(data.data(), 1, fsize, f) != fsize) {}
  fclose(f);

  for (size_t s = 0; s < segments.size(); s++) {
    // 1. Decompress if needed
    if (data.size() >= 2 && ((data[0] == 0x1f && data[1] == 0x8b) || (data[0] == 0x1e && data[1] == 0x8c))) {
      std::vector<BYTE> dec;
      if (DecompressData(data, dec, (int)s))
        data = dec;
    }

    // 2. Check if it's a group
    if (data.size() < sizeof(C4GroupHeader))
      break;
    C4GroupHeader head;
    memcpy(&head, data.data(), sizeof(C4GroupHeader));

    MemUnscramble((BYTE *)&head, sizeof(C4GroupHeader));
    if (memcmp(head.id, "RedWolf Design GrpFolder", 24) != 0) {
      memcpy(&head, data.data(), sizeof(C4GroupHeader));
      if (memcmp(head.id, "RedWolf Design GrpFolder", 24) != 0)
        break;
    }

    // 3. Handle last segment
    if (s == segments.size() - 1) {
      if (raw_mode) {
#ifdef _WIN32
        _setmode(_fileno(stdout), _O_BINARY);
#endif
        fwrite(data.data(), 1, data.size(), stdout);
        return 0;
      }

      printf("--- Group: %s ---\n", segments[s].c_str());
      printf("Entries: %d | Maker: %.32s\n", head.Entries, head.Maker);
      size_t offset = sizeof(C4GroupHeader);
      for (int i = 0; i < head.Entries; i++) {
        C4GroupEntryCore entry;
        memcpy(&entry, data.data() + offset, sizeof(C4GroupEntryCore));
        printf("[%2d] %-32.256s | Size: %7d | Offset: %7d\n", i, entry.FileName, entry.Size, entry.Offset);
        offset += sizeof(C4GroupEntryCore);
      }
      return 0;
    }

    // 4. Find next segment
    bool found = false;
    size_t offset = sizeof(C4GroupHeader);
    for (int i = 0; i < head.Entries; i++) {
      C4GroupEntryCore entry;
      memcpy(&entry, data.data() + offset, sizeof(C4GroupEntryCore));
      if (strcasecmp(entry.FileName, segments[s + 1].c_str()) == 0) {
        size_t data_start = sizeof(C4GroupHeader) + head.Entries * sizeof(C4GroupEntryCore) + entry.Offset;
        std::vector<BYTE> next_data(entry.Size);
        memcpy(next_data.data(), data.data() + data_start, entry.Size);
        data = next_data;
        found = true;
        break;
      }
      offset += sizeof(C4GroupEntryCore);
    }
    if (!found) {
      fprintf(stderr, "Segment '%s' not found\n", segments[s + 1].c_str());
      return 1;
    }
  }

  // 5. Final Output (File/Raw Data)
  if (raw_mode) {
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
#endif
    fwrite(data.data(), 1, data.size(), stdout);
    return 0;
  }

  printf("--- File: %s (%zu bytes) ---\n", segments.back().c_str(), data.size());
  size_t dump_size = data.size() > 256 ? 256 : data.size();
  HexDump(data.data(), dump_size);

  if (data.size() > 0) {
    bool printable = true;
    for (size_t i = 0; i < (data.size() < 20 ? data.size() : 20); i++) {
      if (data[i] < 9 || (data[i] > 13 && data[i] < 32)) {
        printable = false;
        break;
      }
    }
    if (printable) {
      int preview_len = data.size() > 1024 ? 1024 : (int)data.size();
      printf("\nAs Text (preview):\n%.*s\n", preview_len, (char *)data.data());
    }
  }

  return 0;
}