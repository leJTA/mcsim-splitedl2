#include <iostream>
#include <fstream>
#include <stdint.h>
#include <cstdlib>
using namespace std;

struct PTSInstrTrace {
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

int main(int argc, char** argv)
{
	if (argc < 3) {
		std::cout << "Usage : " << argv[0] << " [num_inst] [step]" << std::endl;
		exit(1);
	}
	int num_inst = atoi(argv[1]);
	int step = atoi(argv[2]);
	PTSInstrTrace curr_instr;
	curr_instr.waddr = 0x7fff9b21c968;
	curr_instr.wlen  = 8;
	curr_instr.raddr = 0x0;
	curr_instr.raddr2 = 0x0;
	curr_instr.rlen  = 0;
	curr_instr.ip    = 0x41849d;
	curr_instr.category = 0;
	curr_instr.isbranch = false;
	curr_instr.isbranchtaken = false;
	curr_instr.rr0 = 0;
	curr_instr.rr1 = 0;
	curr_instr.rr2 = 0;
	curr_instr.rr3 = 0;
	curr_instr.rw0 = 0;
	curr_instr.rw1 = 0;
	curr_instr.rw2 = 0;
	curr_instr.rw3 = 0;

	// std::cout << sizeof(PTSInstrTrace) << std::endl;
	std::ofstream ofs("prog.trace", ios::binary);
	if (!ofs.is_open()) {
		std::cerr << "Unable to open output file" << std::endl;
		return 1;
	}

	ofs.write(reinterpret_cast<char*>(&curr_instr), sizeof(PTSInstrTrace));

	curr_instr.waddr = 0x0;
	curr_instr.wlen = 0;
	curr_instr.raddr2 = 0x7fff9b21c968;
	// curr_instr.ip = 0x7fff9b21c968;
	for (int i = 0; i < num_inst; ++i) {
		ofs.write(reinterpret_cast<char*>(&curr_instr), sizeof(PTSInstrTrace));
		curr_instr.ip += step * 0x4;
	}
	return 0;
}