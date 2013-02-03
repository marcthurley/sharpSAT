/*
 * component_management.h
 *
 *  Created on: Aug 23, 2012
 *      Author: Marc Thurley
 */

#ifndef COMPONENT_MANAGEMENT_H_
#define COMPONENT_MANAGEMENT_H_

#include <vector>
#include "component_types.h"
#include "containers.h"
#include "stack.h"

using namespace std;

class ComponentCache {
public:

  void init();

  // compute the size in bytes of the component cache from scratch
  // the value is stored in bytes_memory_usage_
  uint64_t recompute_bytes_memory_usage();

  void add_descendant(CacheEntryID compid, CacheEntryID descendantid) {
    assert(descendantid != entry(compid).first_descendant());
    entry(descendantid).set_next_sibling(entry(compid).first_descendant());
    entry(compid).set_first_descendant(descendantid);
  }
  void remove_firstdescendantOf(CacheEntryID compid) {
    CacheEntryID desc = entry(compid).first_descendant();
    if (desc != 0)
      entry(compid).set_first_descendant(entry(desc).next_sibling());
  }
  unsigned long used_memory_MB() {
    return recompute_bytes_memory_usage() / 1000000;
  }

  ComponentCache(SolverConfiguration &conf, DataAndStatistics &statistics);

  ~ComponentCache() {
    for (auto &pbucket : table_)
      if (pbucket != nullptr)
        delete pbucket;
    for (auto &pentry : entry_base_)
      if (pentry != nullptr)
        delete pentry;
  }

  CachedComponent &entry(CacheEntryID id) {
    assert(entry_base_.size() > id);
    assert(entry_base_[id] != nullptr);
    return *entry_base_[id];
  }

  bool hasEntry(CacheEntryID id) {
    assert(entry_base_.size() > id);
    return entry_base_[id];
  }

  // removes the entry id from the hash table
  // but not from the entry base
  inline void removeFromHashTable(CacheEntryID id);

  // we delete the Component with ID id
  // and all its descendants from the cache
  inline void cleanPollutionsInvolving(CacheEntryID id);

  // creates a CCacheEntry in the entry base
  // which contains a packed copy of comp
  // returns the id of the entry created
  // stores in the entry the position of
  // comp which is a part of the component stack
  CacheEntryID createEntryFor(Component &comp, unsigned stack_id);

  // unchecked erase of an entry from entry_base_
  void eraseEntry(CacheEntryID id) {
    statistics_.cache_bytes_memory_usage_ -= entry_base_[id]->SizeInBytes();
    statistics_.sum_size_cached_components_ -= entry_base_[id]->num_variables();
    statistics_.num_cached_components_--;
    delete entry_base_[id];
    entry_base_[id] = nullptr;
    free_entry_base_slots_.push_back(id);
  }


  // store the number in model_count as the model count of CacheEntryID id
  inline void storeValueOf(CacheEntryID id, const mpz_class &model_count);

  // check if the cache contains the modelcount of comp
  // if so, return true and out_model_count contains that model count
  bool requestValueOf(Component &comp, mpz_class &out_model_count);

  bool deleteEntries();

  // delete entries, keeping the descendants tree consistent
  inline void removeFromDescendantsTree(CacheEntryID id);

  // test function to ensure consistency of the descendant tree
  inline void test_descendantstree_consistency();

private:
  unsigned int clip(long unsigned int ofs) {
    return ofs % table_.size();
  }

  CacheBucket &at(unsigned int ofs) {
    if (table_[ofs] == NULL) {
      num_occupied_buckets_++;
      table_[ofs] = new CacheBucket();
    }
    return *table_[ofs];
  }

  bool isBucketAt(unsigned int ofs) {
    return table_[ofs] != NULL;
  }

  vector<CachedComponent *> entry_base_;
  vector<CacheEntryID> free_entry_base_slots_;

  // the actual hash table
  // by means of which the cache is accessed
  vector<CacheBucket *> table_;

  SolverConfiguration &config_;
  DataAndStatistics &statistics_;

  // unsigned long num_buckets_ = 0;
  unsigned long num_occupied_buckets_ = 0;
  unsigned long my_time_ = 0;
};

struct CAClauseHeader {
  unsigned clause_id = 0;
  LiteralID lit_A;
  LiteralID lit_B;

  static unsigned overheadInLits() {
    return (sizeof(CAClauseHeader) - 2 * sizeof(LiteralID)) / sizeof(LiteralID);
  }
};

class ComponentAnalyzer {
public:

  unsigned scoreOf(VariableIndex v) {
    return var_frequency_scores_[v];
  }

  void cacheModelCountOf(unsigned stack_comp_id, const mpz_class &value) {
    if (config_.perform_component_caching)
      cache_.storeValueOf(component_stack_[stack_comp_id]->id(), value);
  }

  Component & superComponentOf(StackLevel &lev) {
    assert(component_stack_.size() > lev.super_component());
    return *component_stack_[lev.super_component()];
  }

  unsigned component_stack_size() {
    return component_stack_.size();
  }

  void cleanRemainingComponentsOf(StackLevel &top) {
    while (component_stack_.size() > top.remaining_components_ofs()) {
      if (cache_.hasEntry(component_stack_.back()->id()))
        cache_.entry(component_stack_.back()->id()).eraseComponentStackID();
      delete component_stack_.back();
      component_stack_.pop_back();
    }
    assert(top.remaining_components_ofs() <= component_stack_.size());
  }

  Component & currentRemainingComponentOf(StackLevel &top) {
    assert(component_stack_.size() > top.currentRemainingComponent());
    return *component_stack_[top.currentRemainingComponent()];
  }

  // checks for the next yet to explore remaining component of top
  // returns true if a non-trivial non-cached component
  // has been found and is now stack_.TOS_NextComp()
  // returns false if all components have been processed;
  bool findNextRemainingComponentOf(StackLevel &top) {
    // record Remaining Components if there are none!
    if (component_stack_.size() <= top.remaining_components_ofs())
      recordRemainingCompsFor(top);
    assert(!top.branch_found_unsat());
    if (top.hasUnprocessedComponents())
      return true;
    // if no component remains
    // make sure, at least that the current branch is considered SAT
    top.includeSolution(1);
    return false;
  }

  bool recordRemainingCompsFor(StackLevel &top);

  ComponentAnalyzer(SolverConfiguration &config, DataAndStatistics &statistics,
      LiteralIndexedVector<TriValue> & lit_values) :
      config_(config), statistics_(statistics), cache_(config, statistics), literal_values_(
          lit_values) {
  }

  void initialize(LiteralIndexedVector<Literal> & literals,
      vector<LiteralID> &lit_pool);

  ComponentCache &cache() {
    return cache_;
  }

  void removeAllCachePollutionsOf(StackLevel &top);

private:

  SolverConfiguration &config_;
  DataAndStatistics &statistics_;

  vector<Component *> component_stack_;
  ComponentCache cache_;

  // the id of the last clause
  // not that clause ID is the clause number,
  // different from the offset of the clause in the literal pool
  unsigned max_clause_id_ = 0;
  unsigned max_variable_id_ = 0;

  // pool of clauses as lists of LiteralIDs
  // Note that with a clause begin position p we have
  // the clause ID at position p-1
  vector<LiteralID> literal_pool_;

  // this contains clause offsets of the clauses
  // where each variable occurs in;
  vector<ClauseOfs> variable_occurrence_lists_pool_;

  // this is a new idea,
  // for every variable we have a list
  // 0 binarylinks 0 occurrences 0
  // this should give better cache behaviour,
  // because all links of one variable (binary and nonbinray) are found
  // in one contiguous chunk of memory
  vector<unsigned> unified_variable_links_lists_pool_;

  vector<unsigned> variable_link_list_offsets_;
  LiteralIndexedVector<TriValue> & literal_values_;

  vector<unsigned> var_frequency_scores_;

  CA_SearchState *clauses_seen_;
  CA_SearchState *variables_seen_;

  vector<VariableIndex> component_search_stack_;

  bool isResolved(const LiteralID lit) {
    return literal_values_[lit] == F_TRI;
  }

  bool isSatisfied(const LiteralID lit) {
    return literal_values_[lit] == T_TRI;
  }

  bool isActive(const VariableIndex v) {
    return literal_values_[LiteralID(v, true)] == X_TRI;
  }

  unsigned getClauseID(ClauseOfs cl_ofs) {
    return reinterpret_cast<CAClauseHeader *>(&literal_pool_[cl_ofs
        - CAClauseHeader::overheadInLits()])->clause_id;
  }

  CAClauseHeader &getHeaderOf(ClauseOfs cl_ofs) {
    return *reinterpret_cast<CAClauseHeader *>(&literal_pool_[cl_ofs
        - CAClauseHeader::overheadInLits()]);
  }

  unsigned *beginOfLinkList(VariableIndex v) {
    return &unified_variable_links_lists_pool_[variable_link_list_offsets_[v]];
  }

  vector<LiteralID>::iterator beginOfClause(ClauseOfs cl_ofs) {
    return literal_pool_.begin() + cl_ofs;
  }

  // stores all information about the component of var
  // in variables_seen_, clauses_seen_ and
  // component_search_stack
  // we have an isolated variable iff
  // after execution component_search_stack.size()==1
  void recordComponentOf(const VariableIndex var);

  void initializeComponentStack();
};

#include "component_management-inl.h"

#endif /* COMPONENT_MANAGEMENT_H_ */
