#include "solver.h"

#include <iostream>

using namespace std;

// TODO change log - write up somewhere else
// 1. redesigned everything:
// 1.. Solver is now fully 64bit - especially the cache may now store more than 4GB
//     of data
//
// 1a. disentangled component analysis and the SAT solver part
//     this allows for simpler algorithms and datastructures
// 1b. the whole redesign reduced the number of code lines by about 1/3
//
// 2. new clause learning algorithms oriented on new SAT solvers
//    incorporating conflict clause minimization
// 3. Implicit BCP has been sped up since for every failed literal l found
//    we assert now every literal corresponding to a UIP of the conflict
//    caused by l, and add a UIP clause for that
// 4. Decision Heuristics:
//    since several industrial formulas have variables representing clauses
//    of other variables e.g. x <-> y OR z OR w
//    we found that moving them (i.e. x) effectively to the front
//    gives significant speedups
//    EXAMPLE: bmc-ibm-10.cnf goes from timeout after >9hours
//             down to 750 seconds to solve
// 4a. this heuristic is still no understood and does not always have
//    positive effects. Sometimes pushing the x's to the end of all vars
//    has better effects, e.g. in ibm-12.
//    have to understand why this difference exists
// 4b. NOTE it has severely bad effects on the whole satisfiable part of the ssa suite
// 5. instances where caching of UNSAT components seems to yield
//    significant speedup are the satisfiable ones of the SSA suite
// 6. changed the decision heursitics to floating point values, and changed
//    the VSADS weighting
// 7. As it does not make sense to use only finite precision, the GMP package is
//    now a prerequisite for sharpSAT. This also has to do with the fact that we use
//    integer arithmetic now.
// 8. Caching of UNSAT components is NOT performed as there has not been noticed a
//    positive effect of this
// 9. A new coding of the components which frequently saves 30 - 50% of cache memory
// 10. reduced the memory overhead of cached components to less than 40%
// BUGS:
// 1.  corrected a bu in the use of the GMP bignumber package that caused (among others)
//     wrong numbers on bmc-ibm-5, bmc-ibm-10, etc.
// 2.  several out-of-memory problems with the old sharpSAT have been resolved
//     cache now does not store new cache entries if not at least 100 MB of free ram
//     exist
// 2a. hence cache size is not automatically maximum ALL RAM, the
//     "-cs" program option is now only to make sure that the cache does not
//     grow too much

int main(int argc, char *argv[]) {

  string input_file;
  Solver theSolver;

  if (argc <= 1) {
    cout << "Usage: sharpSAT [options] [CNF_File]" << endl;
    cout << "Options: " << endl;
    cout << "\t -noPP  \t turn off preprocessing" << endl;
    cout << "\t -noNCB \t turn off nonchronological backtracking" << endl;
    cout << "\t -q     \t quiet mode" << endl;
    cout << "\t -t [s] \t set time bound to s seconds" << endl;
    cout << "\t -noCC  \t turn off component caching" << endl;
    cout << "\t -cs [n]\t set max cache size to n MB" << endl;
    cout << "\t -noIBCP\t turn off implicit BCP" << endl;
    cout << "\t" << endl;

    return -1;
  }

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-noNCB") == 0)
      theSolver.config().perform_non_chron_back_track = false;
    if (strcmp(argv[i], "-noCC") == 0)
      theSolver.config().perform_component_caching = false;
    if (strcmp(argv[i], "-noIBCP") == 0)
      theSolver.config().perform_failed_lit_test = false;
    if (strcmp(argv[i], "-noPP") == 0)
      theSolver.config().perform_pre_processing = false;
    else if (strcmp(argv[i], "-q") == 0)
      SolverConfiguration::quiet = true;
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
      theSolver.config().maximum_cache_size_bytes = atol(argv[i + 1]) * 1000000;
    } else
      input_file = argv[i];
  }

  theSolver.solve(input_file);
  return 0;
}
