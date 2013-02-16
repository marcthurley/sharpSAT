/*
 * component_archetype.h
 *
 *  Created on: Feb 9, 2013
 *      Author: mthurley
 */

#ifndef COMPONENT_ARCHETYPE_H_
#define COMPONENT_ARCHETYPE_H_

#include <cstring>

#include "../primitive_types.h"

// State values for variables found during component
// analysis (CA)
typedef unsigned char CA_SearchState;
#define   CA_NIL  0
#define   CA_VAR_IN_SUP_COMP_UNSEEN  1
#define   CA_VAR_SEEN 2
#define   CA_VAR_IN_OTHER_COMP  4

#define   CA_VAR_MASK  7

#define   CA_CL_IN_SUP_COMP_UNSEEN  8
#define   CA_CL_SEEN 16
#define   CA_CL_IN_OTHER_COMP  32
#define   CA_CL_ALL_LITS_ACTIVE  64

#define   CA_CL_MASK  120

class Component;
class StackLevel;

class ComponentArchetype {
public:
  ComponentArchetype() {
  }
  ComponentArchetype(StackLevel &stack_level, Component &super_comp) :
      p_super_comp_(&super_comp), p_stack_level_(&stack_level) {
  }

  void reInitialize(StackLevel &stack_level, Component &super_comp) {
    p_super_comp_ = &super_comp;
    p_stack_level_ = &stack_level;
    clearArrays();
  }

  Component &super_comp() {
    return *p_super_comp_;
  }

  StackLevel & stack_level() {
    return *p_stack_level_;
  }

  void setVar_in_sup_comp_unseen(VariableIndex v) {
    seen_[v] = CA_VAR_IN_SUP_COMP_UNSEEN | (seen_[v] & CA_CL_MASK);
  }

  void setClause_in_sup_comp_unseen(ClauseIndex cl) {
    seen_[cl] = CA_CL_IN_SUP_COMP_UNSEEN | (seen_[cl] & CA_VAR_MASK);
  }

  void setVar_nil(VariableIndex v) {
    seen_[v] &= CA_CL_MASK;
  }

  void setClause_nil(ClauseIndex cl) {
    seen_[cl] &= CA_VAR_MASK;
  }

  void setVar_seen(VariableIndex v) {
    seen_[v] = CA_VAR_SEEN | (seen_[v] & CA_CL_MASK);
  }

  void setClause_seen(ClauseIndex cl) {
    setClause_nil(cl);
    seen_[cl] = CA_CL_SEEN | (seen_[cl] & CA_VAR_MASK);
  }
  void setVar_in_other_comp(VariableIndex v) {
    seen_[v] = CA_VAR_IN_OTHER_COMP | (seen_[v] & CA_CL_MASK);
  }

  void setClause_in_other_comp(ClauseIndex cl) {
    seen_[cl] = CA_CL_IN_OTHER_COMP | (seen_[cl] & CA_VAR_MASK);
  }

  bool var_seen(VariableIndex v) {
    return seen_[v] & CA_VAR_SEEN;
  }

  bool clause_seen(ClauseIndex cl) {
    return seen_[cl] & CA_CL_SEEN;
  }

  bool clause_all_lits_active(ClauseIndex cl) {
    return seen_[cl] & CA_CL_ALL_LITS_ACTIVE;
  }
  void setClause_all_lits_active(ClauseIndex cl) {
    seen_[cl] |= CA_CL_ALL_LITS_ACTIVE;
  }

  bool var_nil(VariableIndex v) {
    return (seen_[v] & CA_VAR_MASK) == 0;
  }

  bool clause_nil(ClauseIndex cl) {
    return (seen_[cl] & CA_CL_MASK) == 0;
  }

  bool var_unseen_in_sup_comp(VariableIndex v) {
    return seen_[v] & CA_VAR_IN_SUP_COMP_UNSEEN;
  }

  bool clause_unseen_in_sup_comp(ClauseIndex cl) {
    return seen_[cl] & CA_CL_IN_SUP_COMP_UNSEEN;
  }

  bool var_seen_in_peer_comp(VariableIndex v) {
    return seen_[v] & CA_VAR_IN_OTHER_COMP;
  }

  bool clause_seen_in_peer_comp(ClauseIndex cl) {
    return seen_[cl] & CA_CL_IN_OTHER_COMP;
  }
  static void initArrays(unsigned max_variable_id, unsigned max_clause_id) {
    unsigned seen_size = max_variable_id + 1;
    if (max_clause_id + 1 > seen_size)
      seen_size = max_clause_id + 1;
    seen_ = new CA_SearchState[seen_size];
    seen_byte_size_ = sizeof(CA_SearchState) * (seen_size);

    clearArrays();
  }

  static void clearArrays() {
    memset(seen_, CA_NIL, seen_byte_size_);
  }

private:
  Component *p_super_comp_;
  StackLevel *p_stack_level_;

  static CA_SearchState *seen_;
  static unsigned seen_byte_size_;
};

//class ComponentArchetype{
//public:
//  ComponentArchetype(){
//  }
//  ComponentArchetype(StackLevel &stack_level, Component &super_comp)
//    : p_super_comp_(&super_comp), p_stack_level_(&stack_level){
//  }
//
//  void reInitialize(StackLevel &stack_level,Component &super_comp){
//    p_super_comp_ = &super_comp;
//    p_stack_level_ = &stack_level;
//    clearArrays();
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
//  void setVar_in_sup_comp_unseen(VariableIndex v){
//    seen_[v] &= CA_CL_MASK;
//    seen_[v] |= CA_VAR_IN_SUP_COMP_UNSEEN;
//  }
//
//  void setClause_in_sup_comp_unseen(ClauseIndex cl){
//    seen_[cl] &= CA_VAR_MASK;
//    seen_[cl] |= CA_CL_IN_SUP_COMP_UNSEEN;
//  }
//
//  void setVar_nil(VariableIndex v){
//      seen_[v] &= CA_CL_MASK;
//  }
//
//  void setClause_nil(ClauseIndex cl){
//       seen_[cl] &= CA_VAR_MASK;
//  }
//
//  void setVar_seen(VariableIndex v){
//      setVar_nil(v);
//      seen_[v] |= CA_VAR_SEEN;
//    }
//
//    void setClause_seen(ClauseIndex cl){
//      setClause_nil(cl);
//      seen_[cl] |= CA_CL_SEEN;
//    }
//  void setVar_in_other_comp(VariableIndex v){
//      setVar_nil(v);
//      seen_[v] |= CA_VAR_IN_OTHER_COMP;
//    }
//
//    void setClause_in_other_comp(ClauseIndex cl){
//      setClause_nil(cl);
//       seen_[cl] |= CA_CL_IN_OTHER_COMP;
//    }
//
//    bool var_seen(VariableIndex v){
//       return seen_[v] & CA_VAR_SEEN;
//    }
//
//    bool clause_seen(ClauseIndex cl){
//       return  seen_[cl] & CA_CL_SEEN;
//    }
//
//   bool clause_all_lits_active(ClauseIndex cl){
//           return  seen_[cl] & CA_CL_ALL_LITS_ACTIVE;
//   }
//   void setClause_all_lits_active(ClauseIndex cl){
//          seen_[cl] |= CA_CL_ALL_LITS_ACTIVE;
//   }
//
//    bool var_nil(VariableIndex v){
//       return (seen_[v] & CA_VAR_MASK) == 0;
//    }
//
//    bool clause_nil(ClauseIndex cl){
//       return  (seen_[cl] & CA_CL_MASK) == 0;
//    }
//
//    bool var_unseen_in_sup_comp(VariableIndex v){
//      return seen_[v] & CA_VAR_IN_SUP_COMP_UNSEEN;
//    }
//
//    bool clause_unseen_in_sup_comp(ClauseIndex cl){
//      return  seen_[cl] & CA_CL_IN_SUP_COMP_UNSEEN;
//    }
//
//    bool var_seen_in_peer_comp(VariableIndex v){
//          return seen_[v] & CA_VAR_IN_OTHER_COMP;
//    }
//
//    bool clause_seen_in_peer_comp(ClauseIndex cl){
//          return  seen_[cl] & CA_CL_IN_OTHER_COMP;
//    }
//  static void initArrays(unsigned max_variable_id, unsigned max_clause_id){
//    unsigned seen_size = max_variable_id + 1;
//    if(max_clause_id + 1 > seen_size)
//         seen_size = max_clause_id + 1;
//    seen_ = new CA_SearchState[seen_size];
//    seen_byte_size_ =  sizeof(CA_SearchState) * (seen_size);
//
//    clearArrays();
//  }
//
//
//  static void clearArrays(){
//    memset(seen_, CA_NIL, seen_byte_size_);
//  }
//
//private:
//  Component *p_super_comp_;
//  StackLevel *p_stack_level_;
//
//  static CA_SearchState *seen_;
//  static unsigned seen_byte_size_;
//};

//class ComponentArchetype{
//public:
//  ComponentArchetype(){
//  }
//  ComponentArchetype(StackLevel &stack_level, Component &super_comp)
//    : p_super_comp_(&super_comp), p_stack_level_(&stack_level){
//  }
//
//  void reInitialize(StackLevel &stack_level,Component &super_comp){
//    p_super_comp_ = &super_comp;
//    p_stack_level_ = &stack_level;
//    clearArrays();
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
//  void setVar_in_sup_comp_unseen(VariableIndex v){
//    variables_seen_[v] &= CA_CL_MASK;
//    variables_seen_[v] |= CA_VAR_IN_SUP_COMP_UNSEEN;
//  }
//
//  void setClause_in_sup_comp_unseen(ClauseIndex cl){
//    clauses_seen_[cl] &= CA_VAR_MASK;
//    clauses_seen_[cl] |= CA_CL_IN_SUP_COMP_UNSEEN;
//  }
//
//  void setVar_nil(VariableIndex v){
//      variables_seen_[v] &= CA_CL_MASK;
//  }
//
//  void setClause_nil(ClauseIndex cl){
//       clauses_seen_[cl] &= CA_VAR_MASK;
//  }
//
//  void setVar_seen(VariableIndex v){
//      setVar_nil(v);
//      variables_seen_[v] |= CA_VAR_SEEN;
//    }
//
//    void setClause_seen(ClauseIndex cl){
//      setClause_nil(cl);
//      clauses_seen_[cl] |= CA_CL_SEEN;
//    }
//  void setVar_in_other_comp(VariableIndex v){
//      setVar_nil(v);
//      variables_seen_[v] |= CA_VAR_IN_OTHER_COMP;
//    }
//
//    void setClause_in_other_comp(ClauseIndex cl){
//      setClause_nil(cl);
//       clauses_seen_[cl] |= CA_CL_IN_OTHER_COMP;
//    }
//
//    bool var_seen(VariableIndex v){
//       return variables_seen_[v] & CA_VAR_SEEN;
//    }
//
//    bool clause_seen(ClauseIndex cl){
//       return  clauses_seen_[cl] & CA_CL_SEEN;
//    }
//
//   bool clause_all_lits_active(ClauseIndex cl){
//           return  clauses_seen_[cl] & CA_CL_ALL_LITS_ACTIVE;
//   }
//   void setClause_all_lits_active(ClauseIndex cl){
//          clauses_seen_[cl] |= CA_CL_ALL_LITS_ACTIVE;
//   }
//
//    bool var_nil(VariableIndex v){
//       return (variables_seen_[v] & CA_VAR_MASK) == 0;
//    }
//
//    bool clause_nil(ClauseIndex cl){
//       return  (clauses_seen_[cl] & CA_CL_MASK) == 0;
//    }
//
//    bool var_unseen_in_sup_comp(VariableIndex v){
//      return variables_seen_[v] & CA_VAR_IN_SUP_COMP_UNSEEN;
//    }
//
//    bool clause_unseen_in_sup_comp(ClauseIndex cl){
//      return  clauses_seen_[cl] & CA_CL_IN_SUP_COMP_UNSEEN;
//    }
//
//    bool var_seen_in_peer_comp(VariableIndex v){
//          return variables_seen_[v] & CA_VAR_IN_OTHER_COMP;
//    }
//
//    bool clause_seen_in_peer_comp(ClauseIndex cl){
//          return  clauses_seen_[cl] & CA_CL_IN_OTHER_COMP;
//    }
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
//
//  static void clearArrays(){
//    memset(variables_seen_, CA_NIL, variables_seen_byte_size_);
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

#endif /* COMPONENT_ARCHETYPE_H_ */
