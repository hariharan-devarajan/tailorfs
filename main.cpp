#include <fcntl.h>
#include <unistd.h>

#include <cassert>
#include <iostream>
#include <vector>

int main() {
  int fd = open("file.dat", O_RDWR | O_CREAT);
  assert(fd != -1);
  const size_t request_size = 1024;
  auto write_buf = std::vector<char>(request_size, 'w');
  ssize_t written_bytes = write(fd, write_buf.data(), request_size);
  assert(request_size == written_bytes);
  int status = lseek(fd, 0, SEEK_SET);
  assert(status == 0);
  auto read_buf = std::vector<char>(request_size, 'r');
  ssize_t read_bytes = read(fd, read_buf.data(), request_size);
  assert(request_size == read_bytes);
  for (int i = 0; i < request_size; ++i) {
    assert(write_buf[i] == read_buf[i]);
  }
  status = close(fd);
  assert(status == 0);
  return 0;
}
