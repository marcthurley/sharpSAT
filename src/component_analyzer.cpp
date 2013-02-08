/*
 * component_analyzer.cpp
 *
 *  Created on: Feb 7, 2013
 *      Author: mthurley
 */

#include "component_analyzer.h"

void ComponentAnalyzer::initialize(LiteralIndexedVector<Literal> & literals,
    vector<LiteralID> &lit_pool) {

  max_variable_id_ = literals.end_lit().var() - 1;

  variables_seen_ = new CA_SearchState[max_variable_id_ + 1];
  component_search_stack_.reserve(max_variable_id_ + 1);
  var_frequency_scores_.resize(max_variable_id_ + 1, 0);
  variable_occurrence_lists_pool_.clear();
  variable_link_list_offsets_.resize(max_variable_id_ + 1, 0);
  memset(variables_seen_, CA_NIL,
      sizeof(CA_SearchState) * (max_variable_id_ + 1));

  literal_pool_.reserve(lit_pool.size());

  vector<vector<ClauseOfs> > occs_(max_variable_id_ + 1);
  ClauseOfs current_clause_ofs = 0;
  max_clause_id_ = 0;
  unsigned curr_clause_length = 0;
  for (auto it_lit = lit_pool.begin(); it_lit < lit_pool.end(); it_lit++) {
    if (*it_lit == SENTINEL_LIT) {

      if (it_lit + 1 == lit_pool.end()) {
        literal_pool_.push_back(SENTINEL_LIT);
        break;
      }

      max_clause_id_++;
      literal_pool_.push_back(SENTINEL_LIT);
      for (unsigned i = 0; i < CAClauseHeader::overheadInLits(); i++)
        literal_pool_.push_back(0);
      current_clause_ofs = literal_pool_.size();
      getHeaderOf(current_clause_ofs).clause_id = max_clause_id_;
      it_lit += ClauseHeader::overheadInLits();
      curr_clause_length = 0;
    } else {
      assert(it_lit->var() <= max_variable_id_);
      literal_pool_.push_back(*it_lit);
      curr_clause_length++;
      occs_[it_lit->var()].push_back(current_clause_ofs);
    }
  }

  clauses_seen_ = new CA_SearchState[max_clause_id_ + 1];
  memset(clauses_seen_, CA_NIL, sizeof(CA_SearchState) * (max_clause_id_ + 1));
  // the unified link list
  unified_variable_links_lists_pool_.clear();
  unified_variable_links_lists_pool_.push_back(0);
  unified_variable_links_lists_pool_.push_back(0);
  for (unsigned v = 1; v < occs_.size(); v++) {
    variable_link_list_offsets_[v] = unified_variable_links_lists_pool_.size();
    for (auto l : literals[LiteralID(v, false)].binary_links_)
      if (l != SENTINEL_LIT) {
        unified_variable_links_lists_pool_.push_back(l.var());
      }
    for (auto l : literals[LiteralID(v, true)].binary_links_)
      if (l != SENTINEL_LIT) {
        unified_variable_links_lists_pool_.push_back(l.var());
      }
    unified_variable_links_lists_pool_.push_back(0);
    unified_variable_links_lists_pool_.insert(
        unified_variable_links_lists_pool_.end(), occs_[v].begin(),
        occs_[v].end());
    unified_variable_links_lists_pool_.push_back(0);
  }

  // BEGIN CACHE INIT
  //CachedComponent::adjustPackSize(max_variable_id_, max_clause_id_);
  //initializeComponentStack();
}

Component *ComponentAnalyzer::recordRemainingCompFor(StackLevel &top,
      VariableIndex v, Component &super_comp) {
  Component *p_new_comp = nullptr;
  assert(variables_seen_[v] == CA_IN_SUP_COMP);

  recordComponentOf(v);

  if (component_search_stack_.size() == 1) {
    top.includeSolution(2);
    variables_seen_[v] = CA_IN_OTHER_COMP;
  } else {
    /////////////////////////////////////////////////
    // BEGIN store variables and clauses in component_stack_.back()
    // protocol is: variables first, then clauses
    /////////////////////////////////////////////////
    p_new_comp = new Component();
    p_new_comp->reserveSpace(component_search_stack_.size(),
        super_comp.numLongClauses());

    for (auto v_it = super_comp.varsBegin(); *v_it != varsSENTINEL; v_it++)
      if (variables_seen_[*v_it] == CA_SEEN) { //we have to put a var into our component
        p_new_comp->addVar(*v_it);
        variables_seen_[*v_it] = CA_IN_OTHER_COMP;
      }
    p_new_comp->closeVariableData();

    for (auto it_cl = super_comp.clsBegin(); *it_cl != clsSENTINEL; it_cl++)
      if (clauses_seen_[*it_cl] == CA_SEEN) {
        p_new_comp->addCl(*it_cl);
        clauses_seen_[*it_cl] = CA_IN_OTHER_COMP;
      }
    p_new_comp->closeClauseData();
    /////////////////////////////////////////////////
    // END store variables in resComp
    /////////////////////////////////////////////////
  }
  return p_new_comp;
}

void ComponentAnalyzer::recordComponentOf(const VariableIndex var) {

  component_search_stack_.clear();
  component_search_stack_.push_back(var);

  variables_seen_[var] = CA_SEEN;

  vector<VariableIndex>::const_iterator itVEnd;

  for (auto vt = component_search_stack_.begin();
      vt != component_search_stack_.end(); vt++) {
    // the for-loop is applicable here because componentSearchStack.capacity() == countAllVars()
    //BEGIN traverse binary clauses
    assert(isActive(*vt));
    unsigned *pvar = beginOfLinkList(*vt);
    for (; *pvar; pvar++) {
      assert(*pvar <= max_variable_id_);
      if (variables_seen_[*pvar] == CA_IN_SUP_COMP) {
        assert(isActive(*pvar));
        component_search_stack_.push_back(*pvar);
        variables_seen_[*pvar] = CA_SEEN;
        var_frequency_scores_[*pvar]++;
        var_frequency_scores_[*vt]++;
      }
    }
    //END traverse binary clauses

    // start traversing links to long clauses
    // not that that list starts right after the 0 termination of the prvious list
    // hence  pcl_ofs = pvar + 1
    for (auto pcl_ofs = pvar + 1; *pcl_ofs != SENTINEL_CL; pcl_ofs++) {
      ClauseIndex clID = getClauseID(*pcl_ofs);
      if (clauses_seen_[clID] == CA_IN_SUP_COMP) {
        itVEnd = component_search_stack_.end();
        for (auto itL = beginOfClause(*pcl_ofs); *itL != SENTINEL_LIT; itL++) {
          assert(itL->var() <= max_variable_id_);
          if (variables_seen_[itL->var()] == CA_NIL) { //i.e. the variable is not active
            if (isResolved(*itL))
              continue;
            //BEGIN accidentally entered a satisfied clause: undo the search process
            while (component_search_stack_.end() != itVEnd) {
              assert( component_search_stack_.back() <= max_variable_id_);
              variables_seen_[component_search_stack_.back()] = CA_IN_SUP_COMP;
              component_search_stack_.pop_back();
            }
            clauses_seen_[clID] = CA_NIL;
            for (auto itX = beginOfClause(*pcl_ofs); itX != itL; itX++) {
              if (var_frequency_scores_[itX->var()] > 0)
                var_frequency_scores_[itX->var()]--;
            }
            //END accidentally entered a satisfied clause: undo the search process
            break;
          } else {
            var_frequency_scores_[itL->var()]++;
            if (variables_seen_[itL->var()] == CA_IN_SUP_COMP) {
              variables_seen_[itL->var()] = CA_SEEN;
              component_search_stack_.push_back(itL->var());
            }
          }
        }
        if (clauses_seen_[clID] == CA_NIL)
          continue;
        clauses_seen_[clID] = CA_SEEN;
      }
    }
  }
}

