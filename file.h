#ifndef file_H
#define file_H 

#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "chunk.h"

class File
{
private:
	std::queue<std::unique_ptr<Chunk>> fifo;
	std::queue<std::unique_ptr<Chunk>> fifo_free;
	std::ifstream f_in;
	std::mutex m_fifo;
	std::mutex m_fifo_free;
	std::condition_variable cv_fifo;
	std::condition_variable cv_fifo_free;
	size_t max_chunk;
	
	static void thead_read(File *file);	
	
public:
	File() = delete;
	explicit File(const std::string &name, size_t mc = 64);
	~File();
	
	explicit operator bool() const;
	bool copy_to(const std::string &name);
};

#endif
