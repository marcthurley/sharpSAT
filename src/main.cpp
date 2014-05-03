#include "solver.h"

#include <iostream>

#include <vector>

//#include <malloc.h>
#include <string>

#include <sys/time.h>
#include <sys/resource.h>

using namespace std;


int main(int argc, char *argv[]) {

  string input_file;
  Solver theSolver;


  if (argc <= 1) {
    cout << "Usage: sharpSAT [options] [CNF_File]" << endl;
    cout << "Options: " << endl;
    cout << "\t -noPP  \t turn off preprocessing" << endl;
    cout << "\t -q     \t quiet mode" << endl;
    cout << "\t -t [s] \t set time bound to s seconds" << endl;
    cout << "\t -noCC  \t turn off component caching" << endl;
    cout << "\t -cs [n]\t set max cache size to n MB" << endl;
    cout << "\t -noIBCP\t turn off implicit BCP" << endl;
    cout << "\t" << endl;

    return -1;
  }

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-noCC") == 0)
      theSolver.config().perform_component_caching = false;
    if (strcmp(argv[i], "-noIBCP") == 0)
      theSolver.config().perform_failed_lit_test = false;
    if (strcmp(argv[i], "-noPP") == 0)
      theSolver.config().perform_pre_processing = false;
    else if (strcmp(argv[i], "-q") == 0)
      theSolver.config().quiet = true;
    else if (strcmp(argv[i], "-v") == 0)
      theSolver.config().verbose = true;
    else if (strcmp(argv[i], "-t") == 0) {
      if (argc <= i + 1) {
        cout << " wrong parameters" << endl;
        return -1;
      }
      theSolver.config().time_bound_seconds = atol(argv[i + 1]);
      if (theSolver.config().verbose)
        cout << "time bound set to" << theSolver.config().time_bound_seconds << "s\n";
     } else if (strcmp(argv[i], "-cs") == 0) {
      if (argc <= i + 1) {
        cout << " wrong parameters" << endl;
        return -1;
      }
      theSolver.statistics().maximum_cache_size_bytes_ = atol(argv[i + 1]) * (uint64_t) 1000000;
    } else
      input_file = argv[i];
  }

  theSolver.solve(input_file);

//  cout << sizeof(LiteralID)<<"MALLOC_STATS:" << endl;
//  malloc_stats();

//  rusage ru;
//  getrusage(RUSAGE_SELF,&ru);
//
//   cout << "\nRus: " <<  ru.ru_maxrss*1024 << endl;
//  cout << "\nMALLINFO:" << endl;
//
//  cout << "total " << mallinfo().arena + mallinfo().hblkhd << endl;
//  cout <<  mallinfo().arena << "non-mmapped space allocated from system " << endl;
//  cout <<  mallinfo().ordblks << "number of free chunks " << endl;
//  cout <<  mallinfo().smblks<< "number of fastbin blocks " << endl;
//  cout <<  mallinfo().hblks<< " number of mmapped regions " << endl;
//  cout <<  mallinfo().hblkhd<< "space in mmapped regions " << endl;
//  cout <<  mallinfo().usmblks<< " maximum total allocated space " << endl;
//  cout <<  mallinfo().fsmblks<< "space available in freed fastbin blocks " << endl;
//  cout <<  mallinfo().uordblks<< " total allocated space " << endl;
//  cout <<  mallinfo().fordblks<< "total free space " << endl;
//  cout <<  mallinfo().keepcost<< " top-most, releasable (via malloc_trim) space " << endl;
  return 0;
}
