
#include <thread>
#include <iostream>
#include <chrono>

#include "file.h"

using namespace std::literals::chrono_literals;

void File::thead_read(File *file)
{
	bool it_is_not_the_last_chunk = true;
	// Step 3. Read
	file->f_in.seekg(0, std::ios::beg);
	
	do {
	// Step 3.1 Get free chunk 
		std::unique_lock<std::mutex> lk(file->m_fifo_free);
		file->cv_fifo_free.wait(lk, [file]{return !file->fifo_free.empty();});
		auto chunk = std::move(file->fifo_free.front());
		file->fifo_free.pop();
		lk.unlock();

	// Step 3.2 Read file
		it_is_not_the_last_chunk = chunk->read(file->f_in);

	// Step 3.3 Push chunk to the fifo
		std::unique_lock<std::mutex> lck{file->m_fifo};
		file->fifo.push(std::move(chunk));

	// Step 3.4 Inform the Write thread about it
		file->cv_fifo.notify_one();
	// 			and go read again
	}while(it_is_not_the_last_chunk);
}
	
File::File(const std::string &name, size_t mc) : max_chunk(mc)
{
	// Step 1.1 Open the source file
	f_in.open(name, std::ios::in | std::ios::binary);
	while(fifo_free.size() < max_chunk) {
		auto chunk = std::make_unique<Chunk>();
		fifo_free.push(std::move(chunk));
	}		
}
File::operator bool() const
{
	return f_in.is_open() && fifo_free.size();
}

bool File::copy_to(const std::string &name)
{
	// Step 1.2 Open the destination file
	std::ofstream f_out(name, std::ios::out | std::ios::binary);
	size_t total_size = 0;	
	
		
	if(f_in.is_open() && f_out.is_open()) {
		const auto time_begin = std::chrono::high_resolution_clock::now();

	// Step 2. Start Read thread
		std::thread t{thead_read, this};
		
		while(1) {
	// Step 4. Write
	// Step 4.1 Wait for the chunk
			std::unique_lock<std::mutex> lk{m_fifo};
			cv_fifo.wait(lk, [this]{return !fifo.empty();});
			auto chunk = std::move(fifo.front());
			fifo.pop();
			lk.unlock();

	// Step 4.2 Check if it is the end
			const auto size_copied = chunk->size();
			if(size_copied == 0)
				break;
			total_size += size_copied;

	// Step 4.3 Write to the output file
			chunk->write(f_out);

	// Step 4.4 Push a chunk to the Free Fifo. This chunk will be used again and again
			std::unique_lock<std::mutex> lck{m_fifo_free};
			fifo_free.push(std::move(chunk));
			cv_fifo_free.notify_one();
		}
		
		f_out.close();
		
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
	if(f_in.is_open())
		f_in.close();
}
