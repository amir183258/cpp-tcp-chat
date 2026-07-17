#include <system_error>
#include <cerrno>

#include <sys/types.h>
#include <unistd.h>

#include "net_utils.hpp"

namespace net {

// write
static ssize_t writen(int fd, const void *vptr, size_t n) {
	size_t nleft;
	ssize_t nwritten;
	const char *ptr;

	ptr = reinterpret_cast<const char*>(vptr);
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = ::write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0; // and call write() again
			else
				return -1;
		}

		nleft -= nwritten;
		ptr += nwritten;
	}

	return n;
}

void write_full(int fd, void *ptr, size_t nbytes) {
	if (writen(fd, ptr, nbytes) != nbytes)
		throw std::system_error(errno, std::generic_category(), "write() failed");
}

// read
static ssize_t readn(int fd, void *vptr, size_t n) {
	size_t nleft;
	ssize_t nread;
	char *ptr;

	ptr = reinterpret_cast<char *>(vptr);
	nleft = n;
	while (nleft > 0) {
		if ( (nread = ::read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR)
				nread = 0; // and call read() again
			else
				return -1;
		}
		else if (nread == 0)
			break; // EOF


		nleft -= nread;
		ptr += nread;
	}

	return n - nleft; // return >= 0
}

ssize_t read_full(int fd, void *ptr, size_t nbytes) {
	ssize_t n;
	if ( (n = readn(fd, ptr, nbytes)) < 0)
		throw std::system_error(errno, std::generic_category(), "read() failed");

	return n;
}

} // namespace
