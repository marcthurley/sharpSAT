/*
 * component_analyzer.h
 *
 *  Created on: Feb 7, 2013
 *      Author: mthurley
 */

#ifndef COMPONENT_ANALYZER_H_
#define COMPONENT_ANALYZER_H_



#include "statistics.h"
#include "component_types/component.h"
#include "component_types/base_packed_component.h"
#include "component_types/component_archetype.h"



#include <vector>
#include <cmath>
#include <gmpxx.h>
#include "containers.h"
#include "stack.h"

using namespace std;




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
  ComponentAnalyzer(DataAndStatistics &statistics,
        LiteralIndexedVector<TriValue> & lit_values) :
        statistics_(statistics), literal_values_(lit_values) {
  }

  unsigned scoreOf(VariableIndex v) {
    return var_frequency_scores_[v];
  }

  ComponentArchetype &current_archetype(){
    return archetype_;
  }

  void initialize(LiteralIndexedVector<Literal> & literals,
      vector<LiteralID> &lit_pool);


  bool isUnseenAndActive(VariableIndex v){
    assert(v <= max_variable_id_);
   // return variables_seen_[v] == CA_IN_SUP_COMP_UNSEEN;
    return archetype_.var_unseen_in_sup_comp(v);
  }

  void setSeenAndStoreInSearchStack(VariableIndex v){
    assert(isActive(v));
    search_stack_.push_back(v);
    //variables_seen_[v] = CA_SEEN;
    archetype_.setVar_seen(v);
  }


  void setupAnalysisContext(StackLevel &top, Component & super_comp){
     archetype_.reInitialize(top,super_comp);

     for (auto vt = super_comp.varsBegin(); *vt != varsSENTINEL; vt++)
       if (isActive(*vt)) {
        // variables_seen_[*vt] = CA_IN_SUP_COMP_UNSEEN;
         archetype_.setVar_in_sup_comp_unseen(*vt);
         var_frequency_scores_[*vt] = 0;
       }

     for (auto itCl = super_comp.clsBegin(); *itCl != clsSENTINEL; itCl++)
       //if(!isSatisfied())
       //if(!isSatisfiedByFirstTwoLits(map_clause_id_to_ofs_[*itCl]))
         archetype_.setClause_in_sup_comp_unseen(*itCl);
  }

  // returns true, iff the component found is non-trivial
  bool exploreRemainingCompOf(VariableIndex v) {
    assert(archetype_.var_unseen_in_sup_comp(v));
    recordComponentOf(v);

    if (search_stack_.size() == 1) {
      archetype_.stack_level().includeSolution(2);
      archetype_.setVar_in_other_comp(v);
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

  vector<unsigned> map_clause_id_to_ofs_;
  vector<unsigned> variable_link_list_offsets_;
  LiteralIndexedVector<TriValue> & literal_values_;

  vector<unsigned> var_frequency_scores_;


 // Component *p_super_comp_;
 // StackLevel *p_top_;

 // CA_SearchState *clauses_seen_;
 // CA_SearchState *variables_seen_;

  ComponentArchetype  archetype_;


  vector<VariableIndex> search_stack_;

  bool isResolved(const LiteralID lit) {
    return literal_values_[lit] == F_TRI;
  }

  bool isSatisfied(const LiteralID lit) {
    return literal_values_[lit] == T_TRI;
  }

  bool isSatisfiedByFirstTwoLits(ClauseOfs cl_ofs) {
      return isSatisfied(getHeaderOf(cl_ofs).lit_A) || isSatisfied(getHeaderOf(cl_ofs).lit_B);
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


  unsigned log2(unsigned v){
     // taken from
     // http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogLookup
     static const char LogTable256[256] =
     {
     #define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
         -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
         LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
         LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
     };

     unsigned r;     // r will be lg(v)
     register unsigned int t, tt; // temporaries

     if (tt = (v >> 16))
     {
       r = (t = (tt >> 8)) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
     }
     else
     {
       r = (t = (v >> 8)) ? 8 + LogTable256[t] : LogTable256[v];
     }
     return r;
   }

};


Component *ComponentAnalyzer::makeComponentFromArcheType(){
           Component *p_new_comp = new Component();
           p_new_comp->reserveSpace(search_stack_.size(),
               archetype_.super_comp().numLongClauses());
         //  p_new_comp->pck_clause_data_.clear();
         //  p_new_comp->pck_clause_data_.reserve(archetype_.super_comp().numLongClauses());

           for (auto v_it = archetype_.super_comp().varsBegin(); *v_it != varsSENTINEL; v_it++)
             if (archetype_.var_seen(*v_it)) { //we have to put a var into our component
               p_new_comp->addVar(*v_it);
               archetype_.setVar_in_other_comp(*v_it);
             }
           p_new_comp->closeVariableData();
           for (auto it_cl = archetype_.super_comp().clsBegin(); *it_cl != clsSENTINEL; it_cl++)
             if (archetype_.clause_seen(*it_cl)) {
               p_new_comp->addCl(*it_cl);

           //   if(!archetype_.clause_all_lits_active(*it_cl))
           //        p_new_comp->pck_clause_data_.push_back(*it_cl);

               archetype_.setClause_in_other_comp(*it_cl);
             }
           p_new_comp->closeClauseData();
           // p_new_comp->pck_clause_data_.push_back(clsSENTINEL);
           return p_new_comp;
}


//
//Component *ComponentAnalyzer::makeComponentFromArcheType(){
//            Component *p_new_comp = new Component();
//            p_new_comp->reserveSpace(search_stack_.size(),
//                archetype_.super_comp().numLongClauses());
//
//            unsigned max_var_diff = 0;
//            unsigned max_clause_diff = 0;
//            //p_new_comp->max_var_diff_ = 0;
//
//
//            auto vfirst_it = archetype_.super_comp().varsBegin();
//            while (!archetype_.var_seen(*vfirst_it))
//              ++vfirst_it;
//            auto vprev_it = vfirst_it;
//            //assert(*vfirst_it != varsSENTINEL);
//            p_new_comp->addVar(*vfirst_it);
//            archetype_.setVar_in_other_comp(*vfirst_it);
//            for (auto v_it = vfirst_it + 1; *v_it != varsSENTINEL; v_it++)
//              if (archetype_.var_seen(*v_it)) { //we have to put a var into our component
//                //num_variables++;
//                p_new_comp->addVar(*v_it);
//                archetype_.setVar_in_other_comp(*v_it);
//                if (*v_it - *vprev_it > max_var_diff)
//                  max_var_diff = *v_it - *vprev_it;
//                vprev_it = v_it;
//              }
//
//            p_new_comp->closeVariableData();
//
//
//            auto cfirst_it = archetype_.super_comp().clsBegin();
//            while (!archetype_.clause_seen(*cfirst_it) && (*cfirst_it != clsSENTINEL))
//              ++cfirst_it;
//            auto cprev_it = cfirst_it;
//
//            if (*cfirst_it != clsSENTINEL) {
//              p_new_comp->addCl(*cfirst_it);
//              archetype_.setClause_in_other_comp(*cfirst_it);
//              for (auto it_cl = cfirst_it + 1; *it_cl != clsSENTINEL; it_cl++)
//                if (archetype_.clause_seen(*it_cl)) {
//                  p_new_comp->addCl(*it_cl);
//                  archetype_.setClause_in_other_comp(*it_cl);
//                  if (*it_cl - *cprev_it > max_clause_diff)
//                    max_clause_diff = *it_cl - *cprev_it;
//                  cprev_it = it_cl;
//                }
//            }
//            p_new_comp->closeClauseData();
//
//            p_new_comp->bits_per_var_diff_ = log2(max_var_diff + 1) + 1;
//            p_new_comp->bits_per_clause_diff_ = log2(max_clause_diff + 1) + 1;
//
//           return p_new_comp;
//}

//
//Component *ComponentAnalyzer::makeComponentFromArcheType(){
//            Component *p_new_comp = new Component();
//            p_new_comp->reserveSpace(search_stack_.size(),
//                archetype_.super_comp().numLongClauses());
//
//            unsigned  max_var_diff = 0;
//
//
//            auto vfirst_it = archetype_.super_comp().varsBegin();
//            while (!archetype_.var_seen(*vfirst_it))
//              ++vfirst_it;
//            auto vprev_it = vfirst_it;
//            //assert(*vfirst_it != varsSENTINEL);
//            p_new_comp->addVar(*vfirst_it);
//            archetype_.setVar_in_other_comp(*vfirst_it);
//            for (auto v_it = vfirst_it + 1; *v_it != varsSENTINEL; v_it++)
//              if (archetype_.var_seen(*v_it)) { //we have to put a var into our component
//                //num_variables++;
//                p_new_comp->addVar(*v_it);
//                archetype_.setVar_in_other_comp(*v_it);
//                if (*v_it - *vprev_it > max_var_diff)
//                  max_var_diff = *v_it - *vprev_it;
//                vprev_it = v_it;
//              }
//
//            p_new_comp->closeVariableData();
//
//            unsigned max_clause_diff = 0;
//
//            auto cfirst_it = archetype_.super_comp().clsBegin();
//            while (!archetype_.clause_seen(*cfirst_it) && (*cfirst_it != clsSENTINEL))
//              ++cfirst_it;
//            auto cprev_it = cfirst_it;
//
//            if (*cfirst_it != clsSENTINEL) {
//              p_new_comp->addCl(*cfirst_it);
//              archetype_.setClause_in_other_comp(*cfirst_it);
//              for (auto it_cl = cfirst_it + 1; *it_cl != clsSENTINEL; it_cl++)
//                if (archetype_.clause_seen(*it_cl)) {
//                  p_new_comp->addCl(*it_cl);
//                  archetype_.setClause_in_other_comp(*it_cl);
//                  if (*it_cl - *cprev_it > max_clause_diff)
//                   max_clause_diff = *it_cl - *cprev_it;
//                  cprev_it = it_cl;
//                }
//            }
//
//           p_new_comp->closeClauseData();
//
//
//
//           /////////////////////////////
//           /////////////////////////////
//           unsigned bits_per_var_diff = (unsigned int) ceil(
//                  log((double) max_var_diff + 1) / log(2.0));
//
//            if(bits_per_var_diff == 0)
//                 bits_per_var_diff = 1;
//
//            unsigned bits_per_clause_diff = (unsigned int) ceil(
//                 log((double) max_clause_diff + 1) / log(2.0));
//
//
//            unsigned data_size = (BasePackedComponent::bits_per_variable() + 5 + BasePackedComponent::bits_per_clause() + 5
//                + (p_new_comp->num_variables() - 1) * bits_per_var_diff
//                + (p_new_comp->numLongClauses() - 1) * bits_per_clause_diff) / BasePackedComponent::bits_per_block()
//                + 3;
//
//            unsigned * p = p_new_comp->packed_data_ = (unsigned*) malloc(sizeof(unsigned) * data_size);
//
//            *p = bits_per_var_diff;
//            unsigned int bitpos = 5;
//
//            *p |= *p_new_comp->varsBegin() << bitpos;
//            bitpos += BasePackedComponent::bits_per_variable();
//            unsigned hashkey_vars = *p_new_comp->varsBegin();
//
//            for (auto it = p_new_comp->varsBegin() + 1; *it != varsSENTINEL; it++) {
//              *p |= ((*it) - *(it - 1)) << bitpos;
//              bitpos += bits_per_var_diff;
//              hashkey_vars = hashkey_vars * 3 + ((*it) - *(it - 1));
//              if (bitpos >= BasePackedComponent::bits_per_block()) {
//                bitpos -= BasePackedComponent::bits_per_block();
//                *(++p) = (((*it) - *(it - 1)) >> (bits_per_var_diff - bitpos));
//              }
//            }
//            if (bitpos > 0)
//              p++;
//            p_new_comp->packed_clause_ofs_ = p - p_new_comp->packed_data_;
//
//            unsigned hashkey_clauses = *p_new_comp->clsBegin();
//            if (*p_new_comp->clsBegin()) {
//              *p = bits_per_clause_diff;
//              bitpos = 5;
//              *p |= *p_new_comp->clsBegin() << bitpos;
//              bitpos += BasePackedComponent::bits_per_clause();
//              for (auto jt = p_new_comp->clsBegin() + 1; *jt != clsSENTINEL; jt++) {
//                *p |= ((*jt - *(jt - 1)) << (bitpos));
//                bitpos += bits_per_clause_diff;
//                hashkey_clauses = hashkey_clauses * 3 + (*jt - *(jt - 1));
//                if (bitpos >= BasePackedComponent::bits_per_block()) {
//                  bitpos -= BasePackedComponent::bits_per_block();
//                  *(++p) = ((*jt - *(jt - 1)) >> (bits_per_clause_diff - bitpos));
//                }
//              }
//              if (bitpos > 0)
//                p++;
//            }
//            *p = 0;
//            p_new_comp->hashkey_ = hashkey_vars + (((unsigned long) hashkey_clauses) << 16);
//
//
//
//
//
//
//           return p_new_comp;
//}



#endif /* COMPONENT_ANALYZER_H_ */
