#include "cpu.h"
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int read_to_buffer(const char *filename, uint8_t **buffer, size_t *size) {
  int fd = open(filename, O_RDONLY);
  if (fd == -1) {
    perror("Failed to open file.");
    exit(EXIT_FAILURE);
  }

  struct stat sb;
  if (stat(filename, &sb) == -1) {
    perror("Stat Error.");
    exit(EXIT_FAILURE);
  }

  if (sb.st_size == 0) {
    perror("Stat size error.");
    exit(EXIT_FAILURE);
  }

  read(fd, *buffer, sb.st_size);
  close(fd);

  *size = sb.st_size;
  return 0;
}
