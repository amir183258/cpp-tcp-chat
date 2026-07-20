#ifndef PROTOCOL_BUFFER_HPP
#define PROTOCOL_BUFFER_HPP

#include <string>

namespace protocol {

static constexpr int buffer_size = 1024;

class Buffer {
private:
	char buffer[buffer_size];

	int capacity_ = buffer_size;
	int size_ = 0;

	void compact(int len);
	int find_delim(const char delim);
public:
	Buffer() = default;

	Buffer(const Buffer&);
	Buffer& operator=(const Buffer&);
	Buffer(Buffer&&);
	Buffer& operator=(Buffer&&);

	void clear();
	int capacity() const;
	int size() const;
	void increase_size(int n);
	char* data();
	void append(const char *other, const int len);
	std::string consume_once(const char delim);
	std::string consume_once();
	std::string consume_full(const char delim);
	std::string consume_full();
	void consume_once(int fd, const char delim);
	void consume_once(int fd);
	void consume_full(int fd, const char delim);
	void consume_full(int fd);
	int free_space() const;
	bool is_empty() const;
	bool is_full() const;

};

} // namespace

#endif
