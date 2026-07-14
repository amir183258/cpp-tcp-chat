#include <system_error>
#include <cerrno>

#include <unistd.h>
#include <sys/types.h>

#include "net_utils.hpp"

namespace net {

void close_fd(int fd) {
	if (close(fd) == -1)
		throw std::system_error(errno, std::generic_category(), "close() failed");
}

ssize_t read_fd(int fd, void *ptr, size_t nbytes) {
	ssize_t n = read(fd, ptr, nbytes);
	if (n == -1)
		throw std::system_error(errno, std::generic_category(), "read() failed");
	return n;
}

void write_fd(int fd, void *ptr, size_t nbytes) {
	if (write(fd, ptr, nbytes) != nbytes)
		throw std::system_error(errno, std::generic_category(), "write() failed");
}

} // namespace
