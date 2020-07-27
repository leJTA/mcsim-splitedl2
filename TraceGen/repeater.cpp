#include <iostream>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

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
	PTSInstrTrace instr;
	if (write(*(td->fd_ptr), &instr, sizeof(PTSInstrTrace)) < 0) {
		std::cerr << "Unable to write to file " << *(td->fd_ptr) << std::endl;
	}
	pthread_exit(NULL);
}

int main(int argc, char** argv)
{
	if (argc < 3) {
		std::cout << "Usage : " << argv[0] << " [input] [output1] [output2] ... [outputN]" << std::endl;
		return 0;
	}
	
	std::string input;
	int ifd;
	std::vector<std::string> outputs;
	std::vector<int> ofds;
	{
		int i = 1;
		while(i < argc) {
			if (i >= 2) {
				outputs.push_back(argv[i]);
				ofds.push_back(0);
			}
			else {	// i == 1
				input = argv[i];
			}
			++i;
		}
	}
	
	size_t byte_read = 0;
	
	ifd = open(input.c_str(), O_RDONLY);
	if (ifd < 0) {
		std::cerr << "Unable to open input file : " << input << std::endl;
		return -1;
	}
	
	/// Asynchronously open output files
	pthread_t* ths = (pthread_t*)malloc(outputs.size() * sizeof(pthread_t));
	std::vector<Thread_data> tds(outputs.size());
	for (unsigned i = 0; i < outputs.size(); ++i) {
		tds[i].fd_ptr = &ofds[i];
		tds[i].fname = outputs[i].c_str();
		pthread_create(&ths[i], NULL, open_file, (void*)&tds[i]);
	}
	
	/// Join all threads
	for (unsigned i = 0; i < outputs.size(); ++i) {
		pthread_join(ths[i], NULL);
	}
	
	/// (1) Read input file, (2) Write to output files, (3) Repeat
	PTSInstrTrace curr_instr;
	while (true) {
		byte_read = read(ifd, &curr_instr, sizeof(PTSInstrTrace));
		if (byte_read == 0) {
			std::cout << "Terminated" << std::endl;
			break;
		}
		else {
			for (unsigned i = 0; i < outputs.size(); ++i) {
				if (write(ofds[i], &curr_instr, sizeof(PTSInstrTrace)) < 0) {
					std::cerr << "Unable to write to file " << ofds[i] << std::endl;
				}
			}
		}
	}
	
	close(ifd);
	for (unsigned i = 0; i < outputs.size(); ++i) {
		close(ofds[i]);
	}
}

