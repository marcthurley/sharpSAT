/*
 * basic_types.cpp
 *
 *  Created on: Jun 24, 2012
 *      Author: Marc Thurley
 */
#include "basic_types.h"

#include <iostream>
#include <fstream>
#include <math.h>

bool SolverConfiguration::quiet = false;

StopWatch::StopWatch() {
  interval_length_.tv_sec = 60;
  gettimeofday(&last_interval_start_, NULL);
  start_time_ = stop_time_ = last_interval_start_;
}

timeval StopWatch::getElapsedTime() {
  timeval r;
  timeval other_time = stop_time_;
  if (stop_time_.tv_sec == start_time_.tv_sec
      && stop_time_.tv_usec == start_time_.tv_usec)
    gettimeofday(&other_time, NULL);
  long int ad = 0;
  long int bd = 0;

  if (other_time.tv_usec < start_time_.tv_usec) {
    ad = 1;
    bd = 1000000;
  }
  r.tv_sec = other_time.tv_sec - ad - start_time_.tv_sec;
  r.tv_usec = other_time.tv_usec + bd - start_time_.tv_usec;
  return r;
}

void DataAndStatistics::print_final_solution_count() {
  cout << final_solution_count_.get_str();
}

void DataAndStatistics::writeToFile(const string & file_name) {
  ofstream out(file_name, ios_base::app);
  unsigned pos = input_file_.find_last_of("/\\");
  out << "<tr>" << endl;
  out << "<td>" << input_file_.substr(pos + 1) << "</td>" << endl;
  out << "<td>" << num_original_variables_ << "</td>" << endl;
  out << "<td>" << num_original_clauses_ << "</td>" << endl;
  out << "<td>" << num_decisions_ << "</td>" << endl;
  out << "<td>" << time_elapsed_ << "</td>" << endl;

  string s = final_solution_count_.get_str();
  if (final_solution_count_ == 0)
    s = "UNSAT";
  out << "<td>" << s << "</td>" << endl;
  out << "</tr>" << endl;
}

void DataAndStatistics::printShort() {
  if (exit_state_ == TIMEOUT) {
    cout << endl << " TIMEOUT !" << endl;
    return;
  }
  cout << endl << endl;
  cout << "variables (total / active / free)\t" << num_variables_ << "/"
      << num_used_variables_ << "/" << num_variables_ - num_used_variables_
      << endl;
  cout << "clauses (removed) \t\t\t" << num_original_clauses_ << " ("
      << num_original_clauses_ - num_clauses() << ")" << endl;
  cout << "decisions \t\t\t\t" << num_decisions_ << endl;
  cout << "conflicts \t\t\t\t" << num_conflicts_ << endl;
  cout << "conflict clauses (all/bin/unit) \t";
  cout << num_conflict_clauses();
  cout << "/" << num_binary_conflict_clauses_ << "/" << num_unit_clauses_
      << endl << endl;
  cout << "failed literals found by implicit BCP \t " <<
  		  num_failed_literals_detected_ << endl;;

    cout << "implicit BCP miss rate \t " << implicitBCP_miss_rate()*100 << "%";
    cout << endl;
  cout << "cache size " << cache_bytes_memory_usage_ / 1000000 << "MB\t"
      << endl;
  cout << "cache (stores / hits) \t\t\t" << num_cached_components_ << "/"
      << num_cache_hits_ << endl;
  cout << "cache miss rate " << cache_miss_rate() * 100 << "%" << endl;
  cout << " avg. variable count (stores / hits) \t" << getAvgComponentSize()
      << "/" << getAvgCacheHitSize() << endl << endl;
  cout << "\n# solutions " << endl;

  print_final_solution_count();

  cout << "\n# END" << endl << endl;
  cout << "time: " << time_elapsed_ << "s\n\n";
}
