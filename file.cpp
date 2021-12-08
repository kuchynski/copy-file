
#include <thread>
#include <iostream>
#include <chrono>

#include "file.h"

using namespace std::literals::chrono_literals;

void File::thead_read(File *file)
{
	bool it_is_not_last_chunk = false;
	// Step 3. Read
	fseek(file->f_in, 0, SEEK_SET);
	
	do {
	// Step 3.1.1 Get free chunk 
		std::unique_ptr<Chunk> chunk;
		file->m_fifo.lock();
		if(file->fifo_free.size()) {
			chunk = std::move(file->fifo_free.front());
			file->fifo_free.pop();
		}
		file->m_fifo.unlock();
	// Step 3.1.2 or create it
		if(chunk == nullptr) {
			if(file->fifo.size() < file->max_chunk)
				chunk = std::make_unique<Chunk>();
			else
				std::this_thread::sleep_for(1ms);
		}

		if(chunk) {
	// Step 3.2 Read file
	//			bytes_read == 0 means the end of the file
			it_is_not_last_chunk = chunk->read(file->f_in);
	// Step 3.3 Push chunk to the fifo
			file->m_fifo.lock();
			file->fifo.push(std::move(chunk));
			file->m_fifo.unlock();
	// Step 3.4 Inform the Write thread about it
			sem_post(&file->sem);
		}
	// 			and go read again
	}while(it_is_not_last_chunk);
}
	
File::File(const std::string &name, size_t mc) : max_chunk(mc)
{
	// Step 1.1 Open the source file
	f_in = fopen(name.c_str(), "rb");
	sem_init(&sem, 0, 0);
}
File::operator bool() const
{
	return f_in != nullptr;
}

bool File::copy_to(const std::string &name)
{
	// Step 1.2 Open the destination file
	std::FILE* f_out = fopen(name.c_str(), "wb");
	size_t total_size = 0;	
	
		
	if(f_in && f_out) {
		const auto time_begin = std::chrono::high_resolution_clock::now();

	// Step 2. Start Read thread
		std::thread t{thead_read, this};
		
		while(1) {
	// Step 4. Write
	// Step 4.1 Wait for the chunk
			sem_wait(&sem);
			std::unique_ptr<Chunk> chunk;
			m_fifo.lock();
			if(fifo.size()) {
				chunk = std::move(fifo.front());
				fifo.pop();
			}
			m_fifo.unlock();
			
			if(chunk) { // it looks like always true here
	// Step 4.2 Check if it is the end
				const auto size_copied = chunk->size();
				if(size_copied == 0) {
					break;
				}
				total_size += size_copied;
	// Step 4.3 Write to the output file
				chunk->write(f_out);
	// Step 4.4 Push a chunk to the Free Fifo. This chunk will be used again and again
				m_fifo.lock();
				fifo_free.push(std::move(chunk));
				m_fifo.unlock();
			}	
		}
		
		fclose(f_out);
		
	// Step 5. Let's calculate a performance, maybe it's not so bad
		const auto time_end = std::chrono::high_resolution_clock::now();
		const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_begin).count();
		if(duration) { // no duration - no result
			const auto MICROSECONDS_PER_SECOND = 1000000;
			const auto speed_bytes = MICROSECONDS_PER_SECOND * total_size / duration;
			const auto BYTES_PER_KILOBYTE = 1024;
			auto speed_Kbytes = speed_bytes / BYTES_PER_KILOBYTE;
			if(speed_Kbytes > BYTES_PER_KILOBYTE)
				std::cout << "speed " << speed_Kbytes / BYTES_PER_KILOBYTE << " MB/s" << std::endl;
			else
				std::cout << "speed " << speed_Kbytes << " KB/s" << std::endl;
		}
		
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
