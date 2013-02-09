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
#define   CA_IN_SUP_COMP_UNSEEN  1
#define   CA_SEEN 2
#define   CA_IN_OTHER_COMP  3


class Component;
class StackLevel;


class ComponentArchetype{
public:
  ComponentArchetype(){
  }
  ComponentArchetype(StackLevel &stack_level, Component &super_comp)
    : p_super_comp_(&super_comp), p_stack_level_(&stack_level){
  }

  void reInitialize(StackLevel &stack_level,Component &super_comp){
    p_super_comp_ = &super_comp;
    p_stack_level_ = &stack_level;
    clearArrays();
  }

  Component &super_comp(){
    return *p_super_comp_;
  }

  StackLevel & stack_level(){
    return *p_stack_level_;
  }

  void setVar_in_sup_comp_unseen(VariableIndex v){
    variables_seen_[v] = CA_IN_SUP_COMP_UNSEEN;
  }

  void setClause_in_sup_comp_unseen(ClauseIndex cl){
     clauses_seen_[cl] = CA_IN_SUP_COMP_UNSEEN;
  }

  void setVar_nil(VariableIndex v){
      variables_seen_[v] = CA_NIL;
  }

  void setClause_nil(ClauseIndex cl){
       clauses_seen_[cl] = CA_NIL;
  }

  void setVar_seen(VariableIndex v){
      variables_seen_[v] = CA_SEEN;
    }

    void setClause_seen(ClauseIndex cl){
       clauses_seen_[cl] = CA_SEEN;
    }
  void setVar_in_other_comp(VariableIndex v){
      variables_seen_[v] = CA_IN_OTHER_COMP;
    }

    void setClause_in_other_comp(ClauseIndex cl){
       clauses_seen_[cl] = CA_IN_OTHER_COMP;
    }

    bool var_seen(VariableIndex v){
       return variables_seen_[v] == CA_SEEN;
    }

    bool clause_seen(ClauseIndex cl){
       return  clauses_seen_[cl] == CA_SEEN;
    }

    bool var_nil(VariableIndex v){
       return variables_seen_[v] == CA_NIL;
    }

    bool clause_nil(ClauseIndex cl){
       return  clauses_seen_[cl] == CA_NIL;
    }

    bool var_unseen_in_sup_comp(VariableIndex v){
      return variables_seen_[v] == CA_IN_SUP_COMP_UNSEEN;
    }

    bool clause_unseen_in_sup_comp(ClauseIndex cl){
      return  clauses_seen_[cl] == CA_IN_SUP_COMP_UNSEEN;
    }

    bool var_seen_in_peer_comp(VariableIndex v){
          return variables_seen_[v] == CA_IN_OTHER_COMP;
    }

    bool clause_seen_in_peer_comp(ClauseIndex cl){
          return  clauses_seen_[cl] == CA_IN_OTHER_COMP;
    }
  static void initArrays(unsigned max_variable_id, unsigned max_clause_id){

    variables_seen_ = new CA_SearchState[max_variable_id + 1];
    variables_seen_byte_size_ =  sizeof(CA_SearchState) * (max_variable_id + 1);
    memset(variables_seen_, CA_NIL, variables_seen_byte_size_);

    clauses_seen_ =  new CA_SearchState[max_clause_id + 1];
    clauses_seen_byte_size_ = sizeof(CA_SearchState) * (max_clause_id + 1);
    memset(clauses_seen_, CA_NIL, clauses_seen_byte_size_);
  }

  static void initVariableData(unsigned max_variable_id){
    variables_seen_ = new CA_SearchState[max_variable_id + 1];
    variables_seen_byte_size_ =  sizeof(CA_SearchState) * (max_variable_id + 1);
    memset(variables_seen_, CA_NIL, variables_seen_byte_size_);
  }

  static void initClauseData(unsigned max_clause_id){
    clauses_seen_ =  new CA_SearchState[max_clause_id + 1];
    clauses_seen_byte_size_ = sizeof(CA_SearchState) * (max_clause_id + 1);
    memset(clauses_seen_, CA_NIL, clauses_seen_byte_size_);
  }

  static void clearArrays(){
    memset(variables_seen_, CA_NIL, variables_seen_byte_size_);
    memset(clauses_seen_, CA_NIL, clauses_seen_byte_size_);
  }

private:
  Component *p_super_comp_;
  StackLevel *p_stack_level_;

  static CA_SearchState *variables_seen_;
  static unsigned variables_seen_byte_size_;
  static CA_SearchState *clauses_seen_;
  static unsigned clauses_seen_byte_size_;
};


#endif /* COMPONENT_ARCHETYPE_H_ */
