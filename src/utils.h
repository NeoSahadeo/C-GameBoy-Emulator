#ifndef UTILS_NEOSAHADEO
#define UTILS_NEOSAHADEO

#include <inttypes.h>
#include <unistd.h>

int read_to_buffer(const char *filename, uint8_t **buffer, size_t *size);

#endif
