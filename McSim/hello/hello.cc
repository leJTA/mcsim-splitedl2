#include <iostream>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

using namespace std;

int main(int argc, char** argv)
{
  cout << "Hello, world!" << getpid() << endl;
  return 0;
}
