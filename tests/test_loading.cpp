#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <zlib.h>

typedef uint8_t BYTE;

void MemUnscramble(BYTE *bypBuffer, int iSize) {
  int cnt;
  BYTE temp;
  for (cnt = 0; cnt + 2 < iSize; cnt += 3) {
    temp = bypBuffer[cnt];
    bypBuffer[cnt] = bypBuffer[cnt + 2];
    bypBuffer[cnt + 2] = temp;
  }
  for (cnt = 0; cnt < iSize; cnt++)
    bypBuffer[cnt] ^= 237;
}

void MemUnscramble2(BYTE *bypBuffer, int iSize) {
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

int main(int argc, char **argv) {
  if (argc < 2)
    return 1;
  const char *filename = argv[1];

  std::vector<BYTE> data;

  FILE *f = fopen(filename, "rb");
  BYTE magic[2];
  fread(magic, 1, 2, f);
  fclose(f);

  if (magic[0] == 0x1f && magic[1] == 0x8b) {
    printf("Decompressing Gzip...\n");
    gzFile g = gzopen(filename, "rb");
    BYTE buf[4096];
    int len;
    while ((len = gzread(g, buf, sizeof(buf))) > 0) {
      size_t old = data.size();
      data.resize(old + len);
      memcpy(data.data() + old, buf, len);
    }
    gzclose(g);
  } else if (magic[0] == 0x1e && magic[1] == 0x8c) {
    printf("Decompressing Scrambled Gzip...\n");
    char cmd[512];
    sprintf(cmd,
            "cp %s %s.tmp.gz && python3 -c \"d=open('%s.tmp.gz','rb').read(); "
            "open('%s.tmp.gz','wb').write(bytes([d[0]^1, d[1]^7])+d[2:])\" && "
            "gzip -df %s.tmp.gz",
            filename, filename, filename, filename, filename);
    system(cmd);
    char tmpname[512];
    sprintf(tmpname, "%s.tmp", filename);
    f = fopen(tmpname, "rb");
    BYTE buf[4096];
    size_t len;
    while ((len = fread(buf, 1, sizeof(buf), f)) > 0) {
      size_t old = data.size();
      data.resize(old + len);
      memcpy(data.data() + old, buf, len);
    }
    fclose(f);
  } else {
    f = fopen(filename, "rb");
    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    data.resize(fsize);
    fread(data.data(), 1, fsize, f);
    fclose(f);
  }

  size_t fsize = data.size();
  printf("Searching in decompressed %s (%zu bytes)...\n", filename, fsize);

  BYTE entry0[64];
  memcpy(entry0, data.data() + 204, 64);
  MemUnscramble(entry0, 64);
  printf("Entry 0 (offset 204) unscrambled: %.64s\n", entry0);
  printf("Entry 0 hex: ");
  for (int i = 0; i < 32; i++)
    printf("%02x ", entry0[i]);
  printf("\n");

  for (int i = 0; i < (int)fsize - 64; i++) {
    BYTE buf[65];
    memset(buf, 0, 65);

    // Order 1: Swap then XOR
    memcpy(buf, data.data() + i, 64);
    MemUnscramble(buf, 64);
    if (memcmp(buf, "RedWolf", 7) == 0 || strstr((char *)buf, ".txt") || strstr((char *)buf, ".c4s") || strstr((char *)buf, ".c4f")) {
      printf("Found at offset %d (Swap then XOR): %.48s\n", i, buf);
    }

    // Order 2: XOR then Swap
    memcpy(buf, data.data() + i, 64);
    MemUnscramble2(buf, 64);
    if (memcmp(buf, "RedWolf", 7) == 0 || memcmp(buf, "Title.txt", 9) == 0 || memcmp(buf, "Scenario.txt", 12) == 0) {
      printf("Found at offset %d (XOR then Swap): %.32s\n", i, buf);
    }

    // Order 3: Pure XOR 237
    memcpy(buf, data.data() + i, 64);
    for (int j = 0; j < 64; j++)
      buf[j] ^= 237;
    if (memcmp(buf, "RedWolf", 7) == 0 || memcmp(buf, "Title.txt", 9) == 0 || memcmp(buf, "Scenario.txt", 12) == 0) {
      printf("Found at offset %d (Pure XOR): %.32s\n", i, buf);
    }
  }

  return 0;
}
