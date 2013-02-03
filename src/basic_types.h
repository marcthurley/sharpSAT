/*
 * basic_types.h
 *
 *  Created on: Jun 24, 2012
 *      Author: Marc Thurley
 */

#ifndef BASIC_TYPES_H_
#define BASIC_TYPES_H_

#include <vector>

#include <cstdlib>
#include <sys/time.h> // To seed random generator
#include <iostream>
#include <cstdint>
#include "structures.h"

#include <gmpxx.h>

using namespace std;

#ifdef DEBUG
#define toDEBUGOUT(X) cout << X;
#else
#define toDEBUGOUT(X)
#endif

class StopWatch {
public:

  StopWatch();

  bool timeBoundBroken() {
    timeval actual_time;
    gettimeofday(&actual_time, NULL);
    return actual_time.tv_sec - start_time_.tv_sec > time_bound_;
  }

  bool start() {
    bool ret = gettimeofday(&last_interval_start_, NULL);
    start_time_ = stop_time_ = last_interval_start_;
    return !ret;
  }

  bool stop() {
    return gettimeofday(&stop_time_, NULL) == 0;
  }

  double getElapsedSeconds() {
    timeval r = getElapsedTime();
    return r.tv_sec + (double) r.tv_usec / 1000000;
  }

  bool interval_tick() {
    timeval actual_time;
    gettimeofday(&actual_time, NULL);
    if (actual_time.tv_sec - last_interval_start_.tv_sec
        > interval_length_.tv_sec) {
      gettimeofday(&last_interval_start_, NULL);
      return true;
    }
    return false;
  }

  void setTimeBound(long int seconds) {
    time_bound_ = seconds;
  }
  long int getTimeBound();

private:
  timeval start_time_;
  timeval stop_time_;

  long int time_bound_;

  timeval interval_length_;
  timeval last_interval_start_;

  // if we have started and then stopped the watch, this returns
  // the elapsed time
  // otherwise, time elapsed from start_time_ till now is returned
  timeval getElapsedTime();
};

enum SOLVER_StateT {

  NO_STATE, SUCCESS, TIMEOUT, ABORTED
};

struct SolverConfiguration {

  bool perform_non_chron_back_track = true;
  bool perform_component_caching = true;
  bool perform_failed_lit_test = true;
  bool perform_pre_processing = true;

  unsigned long time_bound_seconds = 100000;
  uint64_t maximum_cache_size_bytes = 0;

  bool verbose = false;

  // quiet = true will override verbose;
  static bool quiet;
};

class DataAndStatistics {
public:
  string input_file_;
  double time_elapsed_ = 0.0;

  SOLVER_StateT exit_state_ = NO_STATE;
  // different variable counts
  // number of variables  and clauses before preprocessing
  unsigned long num_original_variables_ = 0;
  unsigned long num_original_clauses_ = 0;
  unsigned long num_original_binary_clauses_ = 0;
  unsigned long num_original_unit_clauses_ = 0;

  // number of variables remaining
  unsigned long num_variables_ = 0;
  // number of variables that actually occurs in clauses
  unsigned long num_used_variables_ = 0;
  unsigned long num_free_variables_ = 0;

  /// different clause counts

  // number of clauses after preprocessing
  unsigned long num_long_clauses_ = 0;
  unsigned long num_binary_clauses_ = 0;

  unsigned long num_long_conflict_clauses_ = 0;
  unsigned long num_binary_conflict_clauses_ = 0;

  unsigned long times_conflict_clauses_cleaned_ = 0;

  unsigned long num_unit_clauses_ = 0;

  /// number of all decisions made
  unsigned long num_decisions_ = 0;
  /// number of all implications derived
  unsigned long num_implications_ = 0;
  // number of all failed literal detections
  unsigned long num_failed_literals_detected_ = 0;
  unsigned long num_failed_literal_tests_ = 0;
  // number of all conflicts occurred
  unsigned long num_conflicts_ = 0;

  // number of clauses overall learned
  unsigned num_clauses_learned_ = 0;


  /* cache statistics */
  uint64_t num_cache_hits_ = 0;
  uint64_t num_cache_look_ups_ = 0;
  uint64_t sum_cache_hit_sizes_ = 0;

  uint64_t num_cached_components_ = 0;
  uint64_t sum_size_cached_components_ = 0;
  uint64_t sum_memory_size_cached_components_ = 0;

  uint64_t cache_bytes_memory_usage_ = 0;
  /*end statistics */

  mpz_class final_solution_count_ = 0;

  double implicitBCP_miss_rate() {
	  if(num_failed_literal_tests_ == 0) return 0.0;
      return (num_failed_literal_tests_ - num_failed_literals_detected_) / (double) num_failed_literal_tests_;
  }
  unsigned long num_clauses() {
    return num_long_clauses_ + num_binary_clauses_ + num_unit_clauses_;
  }
  unsigned long num_conflict_clauses() {
    return num_long_conflict_clauses_ + num_binary_conflict_clauses_;
  }

  unsigned long clause_deletion_interval() {
    return 10000 + 10 * times_conflict_clauses_cleaned_;
  }

  void set_final_solution_count(const mpz_class &count) {
    // set final_solution_count_ = count * 2^(num_variables_ - num_used_variables_)
    mpz_mul_2exp(final_solution_count_.get_mpz_t (),count.get_mpz_t (), num_variables_ - num_used_variables_);
  }
  const mpz_class &final_solution_count() const {
    return final_solution_count_;
  }

  void incorporateConflictClauseData(const vector<LiteralID> &clause) {
    if (clause.size() == 1)
      num_unit_clauses_++;
    else if (clause.size() == 2)
      num_binary_conflict_clauses_++;
    num_long_conflict_clauses_++;
  }
  void incorporateClauseData(const vector<LiteralID> &clause) {
    if (clause.size() == 1)
      num_unit_clauses_++;
    else if (clause.size() == 2)
      num_binary_clauses_++;
    else
      num_long_clauses_++;
  }

  void print_final_solution_count();
  void writeToFile(const string & file_name);

  void printShort();

  void printShortFormulaInfo() {
    cout << "variables (all/used/free): \t";
    cout << num_variables_ << "/" << num_used_variables_ << "/";
    cout << num_variables_ - num_used_variables_ << endl;

    cout << "clauses (all/long/binary/unit): ";
    cout << num_clauses() << "/" << num_long_clauses_;
    cout << "/" << num_binary_clauses_ << "/" << num_unit_clauses_ << endl;
  }
  unsigned getTime() {
    return num_decisions_;
  }

  double avgCachedSize() {
    if (num_cache_hits_ == 0)
      return 0.0;
    return (double) sum_size_cached_components_
        / (double) num_cached_components_;
  }

  double avgCachedMemSize() {
    if (num_cache_hits_ == 0)
      return 0.0;
    return (double) cache_bytes_memory_usage_ / (double) num_cached_components_;
  }

  double avgCacheHitSize() {
    if (num_cache_hits_ == 0)
      return 0.0;
    return (double) sum_cache_hit_sizes_ / (double) num_cache_hits_;
  }

  long double getAvgComponentSize() {
    return sum_size_cached_components_ / (long double) num_cached_components_;
  }

  unsigned long cached_component_count() {
    return num_cached_components_;
  }

  unsigned long cache_hits() {
    return num_cache_hits_;
  }

  double cache_miss_rate() {
    if(num_cache_look_ups_ == 0) return 0.0;
    return (num_cache_look_ups_ - num_cache_hits_)
        / (double) num_cache_look_ups_;
  }

  long double getAvgCacheHitSize() {
    if(num_cache_hits_ == 0) return 0.0;
    return sum_cache_hit_sizes_ / (long double) num_cache_hits_;
  }
};

#endif /* BASIC_TYPES_H_ */
