/*
 * component_analyzer.h
 *
 *  Created on: Feb 7, 2013
 *      Author: mthurley
 */

#ifndef COMPONENT_ANALYZER_H_
#define COMPONENT_ANALYZER_H_




#include "component_types/component.h"
#include "basic_types.h"


#include <vector>
#include <gmpxx.h>
#include "containers.h"
#include "stack.h"

using namespace std;


// State values for variables found during component
// analysis (CA)
typedef unsigned char CA_SearchState;
#define   CA_NIL  0
#define   CA_IN_SUP_COMP_UNSEEN  1
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


//class ComponentArchetype{
//public:
//  ComponentArchetype(Component &super_comp,StackLevel &stack_level)
//    : p_super_comp_(&super_comp), p_stack_level_(&stack_level){
//  }
//
//  Component &super_comp(){
//    return *p_super_comp_;
//  }
//
//  StackLevel & stack_level(){
//    return *p_stack_level_;
//  }
//
//  static void initArrays(unsigned max_variable_id, unsigned max_clause_id){
//
//    variables_seen_ = new CA_SearchState[max_variable_id + 1];
//    variables_seen_byte_size_ =  sizeof(CA_SearchState) * (max_variable_id + 1);
//    memset(variables_seen_, CA_NIL, variables_seen_byte_size_);
//
//    clauses_seen_ =  new CA_SearchState[max_clause_id + 1];
//    clauses_seen_byte_size_ = sizeof(CA_SearchState) * (max_clause_id + 1);
//    memset(clauses_seen_, CA_NIL, clauses_seen_byte_size_);
//  }
//
//private:
//  Component *p_super_comp_;
//  StackLevel *p_stack_level_;
//
//  static CA_SearchState *variables_seen_;
//  static unsigned variables_seen_byte_size_;
//  static CA_SearchState *clauses_seen_;
//  static unsigned clauses_seen_byte_size_;
//};

class ComponentAnalyzer {
public:
  ComponentAnalyzer(SolverConfiguration &config, DataAndStatistics &statistics,
        LiteralIndexedVector<TriValue> & lit_values) :
        config_(config), statistics_(statistics), literal_values_(lit_values) {
  }

  unsigned scoreOf(VariableIndex v) {
    return var_frequency_scores_[v];
  }

  void initialize(LiteralIndexedVector<Literal> & literals,
      vector<LiteralID> &lit_pool);


  bool isUnseenAndActive(VariableIndex v){
    assert(v <= max_variable_id_);
    return variables_seen_[v] == CA_IN_SUP_COMP_UNSEEN;
  }

  void setSeenAndStoreInSearchStack(VariableIndex v){
    assert(isActive(v));
    search_stack_.push_back(v);
    variables_seen_[v] = CA_SEEN;
  }


  void setupAnalysisContext(StackLevel &top, Component & super_comp){
     memset(clauses_seen_, CA_NIL, sizeof(CA_SearchState) * (max_clause_id_ + 1));
     memset(variables_seen_, CA_NIL,
         sizeof(CA_SearchState) * (max_variable_id_ + 1));
     p_top_ = &top;
     p_super_comp_ = &super_comp;

     for (auto vt = super_comp.varsBegin(); *vt != varsSENTINEL; vt++)
       if (isActive(*vt)) {
         variables_seen_[*vt] = CA_IN_SUP_COMP_UNSEEN;
         var_frequency_scores_[*vt] = 0;
       }

     for (auto itCl = super_comp.clsBegin(); *itCl != clsSENTINEL; itCl++)
       clauses_seen_[*itCl] = CA_IN_SUP_COMP_UNSEEN;
  }

  // returns true, iff the component found is non-trivial
  bool exploreRemainingCompOf(VariableIndex v) {
    assert(variables_seen_[v] == CA_IN_SUP_COMP_UNSEEN);

    recordComponentOf(v);

    if (search_stack_.size() == 1) {
      p_top_->includeSolution(2);
      variables_seen_[v] = CA_IN_OTHER_COMP;
      return false;
    }
    return true;
  }

  inline Component *makeComponentFromArcheType();

  unsigned max_clause_id(){
     return max_clause_id_;
  }
  unsigned max_variable_id(){
    return max_variable_id_;
  }
private:

  SolverConfiguration &config_;
  DataAndStatistics &statistics_;

  // the id of the last clause
  // note that clause ID is the clause number,
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


  Component *p_super_comp_;
  StackLevel *p_top_;

  CA_SearchState *clauses_seen_;
  CA_SearchState *variables_seen_;

  vector<VariableIndex> search_stack_;

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


};


Component *ComponentAnalyzer::makeComponentFromArcheType(){
         Component *p_new_comp = new Component();
         p_new_comp->reserveSpace(search_stack_.size(),
             p_super_comp_->numLongClauses());

         for (auto v_it = p_super_comp_->varsBegin(); *v_it != varsSENTINEL; v_it++)
           if (variables_seen_[*v_it] == CA_SEEN) { //we have to put a var into our component
             p_new_comp->addVar(*v_it);
             variables_seen_[*v_it] = CA_IN_OTHER_COMP;
           }
         p_new_comp->closeVariableData();

         for (auto it_cl = p_super_comp_->clsBegin(); *it_cl != clsSENTINEL; it_cl++)
           if (clauses_seen_[*it_cl] == CA_SEEN) {
             p_new_comp->addCl(*it_cl);
             clauses_seen_[*it_cl] = CA_IN_OTHER_COMP;
           }
         p_new_comp->closeClauseData();
         return p_new_comp;
  }

#endif /* COMPONENT_ANALYZER_H_ */
