#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	if (argc < 2) {
		printf("Usage : %s [N]\n", argv[0]);
		exit(1);
	}
	const size_t sz = atoi(argv[1]);
	printf("Allocating array of size : %d\n", sz);
	long arr[sz];
	int i = 0, j = 0;
	for (j = 0; j < 4; ++j) {
		for (i = 0; i < sz; ++i) {
			arr[i] = 13;
		}
	}
	return 0;
}
