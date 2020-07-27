#include <iostream>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <stdint.h>
const size_t buffer_size = 488064466; // = 10GB */

struct PTSInstrTrace
{
  uint64_t waddr;
  uint32_t wlen;
  uint64_t raddr;
  uint64_t raddr2;
  uint32_t rlen;
  uint64_t ip;
  uint32_t category;
  bool     isbranch;
  bool     isbranchtaken;
  uint32_t rr0;
  uint32_t rr1;
  uint32_t rr2;
  uint32_t rr3;
  uint32_t rw0;
  uint32_t rw1;
  uint32_t rw2;
  uint32_t rw3;
};

struct Thread_data {
	int* fd_ptr;
	const char* fname;
};

void* open_file(void* args)
{
	Thread_data* td = (Thread_data*)args;
	*(td->fd_ptr) = open(td->fname, O_WRONLY);
	if (*(td->fd_ptr) < 0) {
		std::cerr << "Unable to open output file : " << td->fname << std::endl;
	}
	else {
		std::cout << "Successfully opened " << *(td->fd_ptr) << std::endl;
	}
	pthread_exit(NULL);
}

int main(int argc, char** argv)
{
	if (argc < 3) {
		std::cout << "Usage : " << argv[0] << " [input] [output1] [output2] ... [outputN]" << std::endl;
		return 0;
	}
	
	std::vector<PTSInstrTrace> buffer(buffer_size);
	std::vector<size_t> rptrs ;				// read pointers
	size_t wptr = 0; 								// write pointer
	std::string input; 							// input filename
	std::vector<std::string> outputs; 		// output filenames
	int ifd; 										// input file descriptor
	int num_fds = argc - 2;
	struct pollfd* ofds = new struct pollfd[num_fds];// output file descriptors
	{
		int i = 1;
		while(i < argc) {
			if (i >= 2) {
				outputs.push_back(argv[i]);
				rptrs.push_back(0);
				ofds[i - 2].fd = 0;
				ofds[i - 2].events = 0;
				ofds[i - 2].events |= POLLOUT;
			}
			else {	// i == 1
				input = argv[i];
			}
			++i;
		}
	}
	
	PTSInstrTrace instr;
	size_t byte_read = 0;
	/// open input file
	ifd = open(input.c_str(), O_RDONLY);
	if (ifd < 0) {
		std::cerr << "Unable to open input file : " << input << std::endl;
		return -1;
	}
	else {
		std::cout << "Opened : " << ifd << std::endl;
	}
	
	/// Asynchronously open output files
	pthread_t* ths = new pthread_t[num_fds];
	std::vector<Thread_data> tds(num_fds);
	for (int i = 0; i < num_fds; ++i) {
		tds[i].fd_ptr = &ofds[i].fd;
		tds[i].fname = outputs[i].c_str();
		pthread_create(&ths[i], NULL, open_file, (void*)&tds[i]);
	}
	
	/// Join all threads
	for (int i = 0; i < num_fds; ++i) {
		pthread_join(ths[i], NULL);
	}
	
	bool buffer_full = false;
	bool buffer_empty = true;
	/// Main loop
	while (true) {
		if (poll(ofds, num_fds, -1) > 0) {
			if (!buffer_full) {
				byte_read = read(ifd, &instr, sizeof(PTSInstrTrace)); 		/// (1) read from source
				if (byte_read > 0) {
					buffer[wptr] = instr;					/// (2) write into the buffer
					wptr = (wptr + 1) % buffer_size; // advance the write pointer
				}
				else if (buffer_empty) {
					std::cout << "Terminated" << std::endl;
					break;
				}
			}
			bool full_flag = false;
			buffer_empty = true;
			for (int i = 0; i < num_fds; ++i) {
				if (rptrs[i] == wptr) {
					continue;
				}
				buffer_empty = false;
				
				if (ofds[i].revents & POLLOUT) { 			/// (3) write to outputs
					if (write(ofds[i].fd, &buffer[rptrs[i]], sizeof(PTSInstrTrace)) < 0) {
						std::cerr << "Unable to write to file" << ofds[i].fd << std::endl;
						break;
					}
					rptrs[i] = (rptrs[i] + 1) % buffer_size; // advance the read pointer
				}
				
				if ((wptr + 1) % buffer_size == rptrs[i]) {
					full_flag = true;
				}
				buffer_full = full_flag;
			}
			if (buffer_empty) { // buffer empty => no more data
				std::cout << "Buffer empty, exit." << std::endl;
				break;
			}
		}
		else {
			std::cerr << "Error polling" << std::endl;
		}
	}
	
	/// Close all file descriptors
	close(ifd);
	for (unsigned i = 0; i < outputs.size(); ++i) {
		close(ofds[i].fd);
	}
}

