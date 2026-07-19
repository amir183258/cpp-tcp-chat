#ifndef PROTOCOL_BUFFER_HPP
#define PROTOCOL_BUFFER_HPP

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

	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	Buffer(Buffer&&) = delete;
	Buffer& operator=(Buffer&&) = delete;

	void clear();
	int capacity() const;
	int size() const;
	char* data();
	void append(const char *other, const int len);
	void consume_full(int fd, const char delim);
	void consume_full(int fd);
	int free_space() const;
	bool is_empty() const;
	bool is_full() const;

};

} // namespace

#endif
