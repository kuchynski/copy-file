#ifndef file_H
#define file_H 

#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "chunk.h"
#include <semaphore.h>

class File
{
private:
	std::queue<std::unique_ptr<Chunk>> chunks;
	std::FILE* f_in = nullptr;
	std::condition_variable cv;
	std::mutex m;
	std::mutex m2;
	sem_t sem;
	
	static void thead_read(File *file);	
	
public:
	File() = delete;
	explicit File(const std::string &name);
	~File();
	
	explicit operator bool() const;
	bool copy_to(const std::string &name);
};

#endif
