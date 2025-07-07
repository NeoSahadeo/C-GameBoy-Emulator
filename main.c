#include "cpu.h"
#include "screen.h"
#include "utils.h"
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define TARGET_FPS 3

// Nanoseconds per frame
#define FRAME_TIME_NS (1000000000 / TARGET_FPS)

long long current_time_ns() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

void game_loop(CPU *cpu) {
  long long last_time = current_time_ns();
  long long accumulator = 0;

  while (1) {
    long long now = current_time_ns();
    long long elapsed = now - last_time;
    last_time = now;

    accumulator += elapsed;

    // Run the update and render steps if enough time has passed
    while (accumulator >= FRAME_TIME_NS) {
      // Update game/emulator state here
      step(cpu);

      // Render frame here

      accumulator -= FRAME_TIME_NS;
    }

    // Calculate how much time is left in the frame and sleep
    long long frame_end_time = last_time + FRAME_TIME_NS - accumulator;
    long long sleep_time_ns = frame_end_time - current_time_ns();

    if (sleep_time_ns > 0) {
      // usleep takes microseconds, so convert ns to us
      usleep(sleep_time_ns / 1000);
    }
  }
}

int main(void) {
  CPU cpu;
  const char *filename = "./roms/sgb_boot.bin";
  size_t file_size = 0;

  uint8_t *memory = calloc(65536, sizeof(uint8_t)); // 64KiB
  uint8_t *vram = calloc(8192, sizeof(uint8_t)); // 8 KiB VRAM (0x8000 - 0x9FFF)

  initialize_cpu(&cpu, memory);
  read_to_buffer(filename, &cpu.memory, &file_size);

  for (int x = 0; x < 10; x++) {
  }

  printf("\n");
  printf("BOOT ROM: \n");
  for (size_t i = 0; i < file_size; i++) {
    printf("%02x ", cpu.memory[i]);
  }
  printf("\n");

  game_loop(&cpu);

  return 0;
}
