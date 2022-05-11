#ifndef chunk_H
#define chunk_H 

#include <array>
#include <fstream>

#define CHUNK_SIZE 4096*16

class Chunk
{
private:
	// The data is stored here
	std::array<char, CHUNK_SIZE> buf;
	size_t size_filled = 0;
public:
	bool read(std::ifstream &f)
	{
		const auto current = f.tellg();
		f.seekg(0, std::ios::end);
		const auto size_file = f.tellg() - current;
		f.seekg(current, std::ios::beg);
		size_filled = (size_file < CHUNK_SIZE)? size_file : CHUNK_SIZE;
		f.read(&buf[0], size_filled);
		return size_filled > 0;
	}
	
	bool write(std::ofstream &f) const
	{
		f.write(&buf[0], size_filled);
		return true;
	}
	
	size_t size() const { return size_filled;}
};

#endif  
