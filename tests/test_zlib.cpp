#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <zlib.h>

typedef uint8_t BYTE;

int main(int argc, char **argv) {
  if (argc < 2)
    return 1;
  const char *filename = argv[1];

  gzFile g = gzopen(filename, "rb");
  if (!g) {
    printf("gzopen failed\n");
    return 1;
  }

  BYTE buf[16];
  int len = gzread(g, buf, 16);
  printf("Read %d bytes via gzread:\n", len);
  for (int i = 0; i < len; i++)
    printf("%02x ", buf[i]);
  printf("\n");
  gzclose(g);

  return 0;
}
