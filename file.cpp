
#include <thread>
#include <iostream>
#include <chrono>

#include "file.h"

void File::thead_read(File *file)
{
}
	
File::File(const std::string &name)
{
	f_in = fopen(name.c_str(), "rb");
}
File::operator bool() const
{
	return f_in != nullptr;
}

bool File::copy_to(const std::string &name)
{
	std::cout << "copy_to " << std::endl;
	std::FILE* f_out = fopen(name.c_str(), "wb");
	size_t total_size = 0;
		
	if(f_in && f_out) {
		size_t size_copied;
		auto time_begin = std::chrono::high_resolution_clock::now();
//		std::thread t{thead_read, this};
		
		Chunk chunk;
		
		while (size_copied = chunk.read(f_in)) {
			total_size += size_copied;
			chunk.write(f_out);
		}
		
		fclose(f_out);
		
		auto time_end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_begin).count();
		
		std::cout << "copied " << total_size << " bytes " << duration << std::endl;
		//t.join();
		return true;
	}		
		
	return false;
}
	
File::~File()
{
	if(f_in)
		fclose(f_in);
}
