#include <stdexcept>
#include <cstring>

#include "protocol_buffer.hpp"
#include "net_utils.hpp"

namespace protocol {

void Buffer::clear() {
	size_ = 0;
}

int Buffer::capacity() const {
	return capacity_;
}

int Buffer::size() const {
	return size_;
}

char* Buffer::data() {
	return buffer;
}

void Buffer::append(const char *other, const int len) {
	if (len < 0)
		throw std::invalid_argument("Buffer::append(): len must be non-negative");

	if (len > free_space())
		throw std::length_error("Buffer::append(): not enough free space");

	std::memcpy(buffer + size_, other, len);
	size_ += len;
}

void Buffer::consume_full(int fd, const char delim) {
	int delim_idx;
	while( (delim_idx = find_delim(delim)) >= 0) {
		net::write_full(fd, buffer, delim_idx + 1);
		compact(delim_idx + 1);
	}
}

void Buffer::consume_full(int fd) {
	consume_full(fd, '\n');
}


int Buffer::free_space() const {
	return capacity_ - size_;
}

bool Buffer::is_empty() const {
	return size_ == 0;
}

bool Buffer::is_full() const {
	return size_ == capacity_;
}

// private methods
void Buffer::compact(int len) {
	if (len < 0)
		throw std::invalid_argument("Buffer::compact(): len must be non-negative");

	if (len > size_) {
		size_ = 0;
		return;
	}

	std::memmove(buffer, buffer + len, size_ - len);
	size_ -= len;
}

int Buffer::find_delim(const char delim) {
	for (int i = 0; i < size_; i++)
		if (buffer[i] == delim)
			return i;
	return -1;
}
	
} // namespace
