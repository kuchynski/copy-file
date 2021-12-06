#ifndef file_H
#define file_H 

#include <memory>
#include <queue>
#include "chunk.h"

class File
{
private:
	std::queue<std::unique_ptr<Chunk>> chunks_free;
	std::queue<std::unique_ptr<Chunk>> chunks_full;
	std::FILE* f_in = nullptr;
	
	static void thead_read(File *file);	
	
public:
	File() = delete;
	explicit File(const std::string &name);
	~File();
	
	explicit operator bool() const;
	bool copy_to(const std::string &name);
};

#endif
