#include "debug.h"
#include <iostream>

using namespace std;

namespace utils {

bool DbgHelper::flag = false;
uint64_t DbgHelper::counter_ = 0;

}//namespace

#if defined(__linux__)

#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

namespace utils {

void print_trace()
{
  void *array[20];
  size_t size;
  char **strings;
  size_t i;

  size = backtrace (array, 20);
  strings = backtrace_symbols (array, size);

  printf ("Obtained %zd stack frames.\n", size);

  for (i = 0; i < size; i++)
     printf ("%s\n", strings[i]);

  free (strings);
}


void handler(int sig) {
  void *array[20];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 20);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

void register_handler() {
	signal(SIGSEGV, handler);
	signal(SIGABRT, handler);
}

}//namespace
#else
void utils::register_handler(){}
#endif
