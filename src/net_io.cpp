#include <system_error>
#include <cerrno>

#include <sys/types.h>
#include <unistd.h>

#include "net_utils.hpp"

namespace net {

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

} // namespace
