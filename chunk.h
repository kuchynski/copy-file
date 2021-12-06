#ifndef chunk_H
#define chunk_H 

#include <array>
#include <cstdio>

#define CHUNK_SIZE 4096

class Chunk
{
private:
	std::array<char, CHUNK_SIZE> buf;
	size_t size_filled = 0;
public:
	size_t read(std::FILE* f)
	{
		size_filled = std::fread(&buf[0], sizeof(char), CHUNK_SIZE, f);
		return size_filled;
	}
	
	bool write(std::FILE* f) const
	{
		auto size_written = std::fwrite(&buf[0], size_filled, 1, f);
		return size_written == size_filled;
	}
	
	size_t size() const { return size_filled;}
};

#endif  
