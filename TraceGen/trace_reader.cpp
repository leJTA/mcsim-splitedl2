#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <iostream>

struct PTSInstrTrace
{
  uint64_t waddr;
  uint32_t wlen;
  uint64_t raddr;
  uint64_t raddr2;
  uint32_t rlen;
  uint64_t ip;
  uint32_t category;
  bool     isbranch;
  bool     isbranchtaken;
  uint32_t rr0;
  uint32_t rr1;
  uint32_t rr2;
  uint32_t rr3;
  uint32_t rw0;
  uint32_t rw1;
  uint32_t rw2;
  uint32_t rw3;
};

int main() {
  const char* file = "/tmp/trace_pipe0";
  int fd = open(file, O_RDONLY);
  if (fd < 0) {
    std::cout << "Unable to open : " << file << std::endl;
    return -1; /* no point in continuing */
  }
  unsigned count = 0, total = 0;

  while (1) {
    PTSInstrTrace next;
    int i;

    size_t count = read(fd, (char*)&next, sizeof(PTSInstrTrace));
    if (0 == count) break;                  /* end of stream */
    else if (count == sizeof(PTSInstrTrace)) { /* read a 88-byte */
      std::cout << "Read " << sizeof(PTSInstrTrace) << " on " << file << std::endl;
      total++;
    }
    else {
      std::cout << "Error : read " << count << " bytes!!!" << std::endl;
    }
  }

  close(fd);       /* close pipe from read end */
  unlink(file);    /* unlink from the underlying file */
  printf("Received traces: %u\n", total);

  return 0;
}
