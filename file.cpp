
#include <thread>
#include <iostream>
#include <chrono>

#include "file.h"

void File::thead_read(File *file)
{
	fseek(file->f_in, 0, SEEK_SET);
	size_t bytes_read = 1;
	
	do {
		auto chunk = std::make_unique<Chunk>();
		bytes_read = chunk->read(file->f_in);
		file->chunks.push(std::move(chunk));
		sem_post(&file->sem);
	}while(bytes_read);
}
	
File::File(const std::string &name)
{
	f_in = fopen(name.c_str(), "rb");
	sem_init(&sem, 0, 0);
}
File::operator bool() const
{
	return f_in != nullptr;
}

bool File::copy_to(const std::string &name)
{
	std::FILE* f_out = fopen(name.c_str(), "wb");
	size_t total_size = 0;
		
	if(f_in && f_out) {
		auto time_begin = std::chrono::high_resolution_clock::now();
		std::thread t{thead_read, this};
		
		while(1) {
			sem_wait(&sem);
			std::unique_ptr<Chunk> chunk;
			if(chunks.size()) {
				chunk = std::move(chunks.front());
				chunks.pop();
				auto size_copied = chunk->size();
				if(size_copied == 0) {
					break;
				}
				total_size += size_copied;
				chunk->write(f_out);
			}	
		}
		
		fclose(f_out);
		
		auto time_end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_begin).count();
		
		std::cout << "copied " << total_size << " bytes " << duration << std::endl;
		t.join();
		return true;
	}		
		
	return false;
}
	
File::~File()
{
	if(f_in)
		fclose(f_in);
}
