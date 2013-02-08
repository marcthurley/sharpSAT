/*
 * component_management.h
 *
 *  Created on: Aug 23, 2012
 *      Author: Marc Thurley
 */

#ifndef COMPONENT_MANAGEMENT_H_
#define COMPONENT_MANAGEMENT_H_



#include "component_types/component.h"
#include "component_cache.h"
//#include "component_analyzer.h"

#include <vector>
#include <gmpxx.h>
#include "containers.h"
#include "stack.h"

using namespace std;


// State values for variables found during component
// analysis (CA)
typedef unsigned char CA_SearchState;
#define   CA_NIL  0
#define   CA_IN_SUP_COMP  1
#define   CA_SEEN 2
#define   CA_IN_OTHER_COMP  3


struct CAClauseHeader {
  unsigned clause_id = 0;
  LiteralID lit_A;
  LiteralID lit_B;

  static unsigned overheadInLits() {
    return (sizeof(CAClauseHeader) - 2 * sizeof(LiteralID)) / sizeof(LiteralID);
  }
};

class ComponentManager {
public:
  ComponentManager(SolverConfiguration &config, DataAndStatistics &statistics,
        LiteralIndexedVector<TriValue> & lit_values) :
        config_(config), statistics_(statistics), cache_(config, statistics),
       // ana_(config,statistics,lit_values),
        literal_values_(lit_values) {
  }

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


//  bool new_recordRemainingCompsFor(StackLevel &top){
//    Component & super_comp = superComponentOf(top);
//    unsigned new_comps_start_ofs = component_stack_.size();
//
//    ana_.preAnalysisSetUp(top, super_comp);
//    Component *p_new_comp = nullptr;
//    for (auto vt = super_comp.varsBegin(); *vt != varsSENTINEL; vt++)
//    if(ana_.needsToBeExplored(*vt)){
//      p_new_comp = ana_.recordRemainingCompFor(top,*vt,super_comp);
//      if(p_new_comp)
//        if (!cache_.manageNewComponent(top, *p_new_comp, super_comp.id(),
//                         component_stack_.size())) {
//                       component_stack_.push_back(p_new_comp);
//                     }
//    }
//
//
//    top.set_unprocessed_components_end(component_stack_.size());
//
//      assert(new_comps_start_ofs <= component_stack_.size());
//
//      // sort the remaining components for processing
//      for (unsigned i = new_comps_start_ofs; i < component_stack_.size(); i++)
//        for (unsigned j = i + 1; j < component_stack_.size(); j++) {
//          if (component_stack_[i]->num_variables()
//              < component_stack_[j]->num_variables())
//            swap(component_stack_[i], component_stack_[j]);
//        }
//  }

  void initialize(LiteralIndexedVector<Literal> & literals,
      vector<LiteralID> &lit_pool);

  void gatherStatistics(){
	  statistics_.cache_bytes_memory_usage_ =
	     cache_.recompute_bytes_memory_usage();
  }

  void removeAllCachePollutionsOf(StackLevel &top);

private:

  SolverConfiguration &config_;
  DataAndStatistics &statistics_;

  vector<Component *> component_stack_;
  ComponentCache cache_;
 // ComponentAnalyzer ana_;

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



#endif /* COMPONENT_MANAGEMENT_H_ */
