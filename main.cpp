#include <fcntl.h>
#include <unistd.h>

#include <cassert>
#include <iostream>

int main() {
  int fd = open("file.dat", O_WRONLY | O_CREAT);
  assert(fd != -1);
  int status = close(fd);
  assert(status == 0);
  return 0;
}
