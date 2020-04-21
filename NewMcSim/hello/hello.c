#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "test.h"

int main (int argc, char** argv) {
    int(*func)();
    void* handle = dlopen("libtest.so", RTLD_LAZY);
    char* error;

    if (!handle) {
       fprintf(stderr, "%s\n", dlerror());
       exit(EXIT_FAILURE);
    }
    func = (int(*)())dlsym(handle, "get_value");

    error = dlerror();
    if (error != NULL) {
       fprintf(stderr, "%s\n", error);
       exit(EXIT_FAILURE);
    }
	 	 
    printf("Result : %d\n", (*func)());
    dlclose(handle);
    return 0;
}
