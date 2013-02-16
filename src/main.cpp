#include "solver.h"

#include <iostream>

using namespace std;

//
//void test(){
//  /// some test related to bug currently being investigated
//  cout << endl;
//  for(int max_diff = 0; max_diff < 257; max_diff++){
//    unsigned bits_per_diff = log2(max_diff) + 1;
//    unsigned diff_max = 0;
//    for (unsigned int i = 0; i < bits_per_diff; i++)
//      diff_max = (diff_max << 1) + 1;
//    cout << "(" << max_diff << "," << bits_per_diff << "," << diff_max << ")" << endl;
//  }
//
//  cout << endl;
//}

int main(int argc, char *argv[]) {

 // test();

  string input_file;
  Solver theSolver;

  if (argc <= 1) {
    cout << "Usage: sharpSAT [options] [CNF_File]" << endl;
    cout << "Options: " << endl;
    cout << "\t -noPP  \t turn off preprocessing" << endl;
    //cout << "\t -noNCB \t turn off nonchronological backtracking" << endl;
    cout << "\t -q     \t quiet mode" << endl;
    cout << "\t -t [s] \t set time bound to s seconds" << endl;
    cout << "\t -noCC  \t turn off component caching" << endl;
    cout << "\t -cs [n]\t set max cache size to n MB" << endl;
    cout << "\t -noIBCP\t turn off implicit BCP" << endl;
    cout << "\t" << endl;

    return -1;
  }

  for (int i = 1; i < argc; i++) {
//    if (strcmp(argv[i], "-noNCB") == 0)
//      theSolver.config().perform_non_chron_back_track = false;
    if (strcmp(argv[i], "-noCC") == 0)
      theSolver.config().perform_component_caching = false;
    if (strcmp(argv[i], "-noIBCP") == 0)
      theSolver.config().perform_failed_lit_test = false;
    if (strcmp(argv[i], "-noPP") == 0)
      theSolver.config().perform_pre_processing = false;
    else if (strcmp(argv[i], "-q") == 0)
      theSolver.config().quiet = true;
    else if (strcmp(argv[i], "-verbose") == 0)
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
  return 0;
}
