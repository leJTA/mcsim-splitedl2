#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	const size_t sz = 131072;
	long arr[sz];
	int i = 0, j = 0;
	for (j = 0; j < 4; ++j) { 
		for (i = 0; i < sz; ++i) {
			arr[(i + 1028 * 16) % sz] = 13;
		}
	}
	return 0;
}
