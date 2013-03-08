/*
 * alt_component_analyzer.cpp
 *
 *  Created on: Mar 5, 2013
 *      Author: mthurley
 */


#include "alt_component_analyzer.h"



void AltComponentAnalyzer::initialize(LiteralIndexedVector<Literal> & literals,
    vector<LiteralID> &lit_pool) {

  max_variable_id_ = literals.end_lit().var() - 1;

  search_stack_.reserve(max_variable_id_ + 1);
  var_frequency_scores_.resize(max_variable_id_ + 1, 0);
  variable_occurrence_lists_pool_.clear();
  variable_link_list_offsets_.clear();
  variable_link_list_offsets_.resize(max_variable_id_ + 1, 0);



  vector<vector<ClauseOfs> > occs_(max_variable_id_ + 1);
  vector<vector<unsigned> > occ_clauses_(max_variable_id_ + 1);
  vector<vector<unsigned> > occ_ternary_clauses_(max_variable_id_ + 1);

  vector<unsigned> tmp;
  //ClauseOfs current_clause_ofs = 0;
  max_clause_id_ = 0;
  unsigned curr_clause_length = 0;
  auto it_curr_cl_st = lit_pool.begin();

  for (auto it_lit = lit_pool.begin(); it_lit < lit_pool.end(); it_lit++) {
    if (*it_lit == SENTINEL_LIT) {

      if (it_lit + 1 == lit_pool.end()) {
        //literal_pool_.push_back(SENTINEL_LIT);
        break;
      }

      max_clause_id_++;
//      literal_pool_.push_back(SENTINEL_LIT);
//      for (unsigned i = 0; i < CAClauseHeader::overheadInLits(); i++)
//        literal_pool_.push_back(0);
    //  current_clause_ofs = literal_pool_.size();
    //  getHeaderOf(current_clause_ofs).clause_id = max_clause_id_;
      it_lit += ClauseHeader::overheadInLits();
      it_curr_cl_st = it_lit + 1;
      // cout << " ("<< curr_clause_length << endl;
      curr_clause_length = 0;

//      assert(map_clause_id_to_ofs_.size() == max_clause_id_);
//      map_clause_id_to_ofs_.push_back(current_clause_ofs);

    } else {
      assert(it_lit->var() <= max_variable_id_);
      //literal_pool_.push_back(*it_lit);
      curr_clause_length++;

      getClause(tmp,it_curr_cl_st, *it_lit);

      if(tmp.size() != 2){
        occs_[it_lit->var()].push_back(max_clause_id_);
        occs_[it_lit->var()].push_back(occ_clauses_[it_lit->var()].size());
        occ_clauses_[it_lit->var()].insert(occ_clauses_[it_lit->var()].end(),
    		       tmp.begin(), tmp.end());
        occ_clauses_[it_lit->var()].push_back(SENTINEL_LIT.raw());
      }
      else {
    	  occ_ternary_clauses_[it_lit->var()].push_back(max_clause_id_);
    	  occ_ternary_clauses_[it_lit->var()].insert(occ_ternary_clauses_[it_lit->var()].end(),
    	      		       tmp.begin(), tmp.end());
      }
    }
  }

  ComponentArchetype::initArrays(max_variable_id_, max_clause_id_);
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
               unified_variable_links_lists_pool_.end(),
               occ_ternary_clauses_[v].begin(),
               occ_ternary_clauses_[v].end());

   unified_variable_links_lists_pool_.push_back(0);


   for(auto it = occs_[v].begin(); it != occs_[v].end(); it+=2){
	   unified_variable_links_lists_pool_.push_back(*it);
	   unified_variable_links_lists_pool_.push_back(*(it + 1) +(occs_[v].end() - it));
   }

   unified_variable_links_lists_pool_.push_back(0);

   unified_variable_links_lists_pool_.insert(
           unified_variable_links_lists_pool_.end(),
           occ_clauses_[v].begin(),
           occ_clauses_[v].end());

//    if(v <= 10){
//    	for(auto l = variable_link_list_offsets_[v]; l < unified_variable_links_lists_pool_.size(); l++)
//    		cout << unified_variable_links_lists_pool_[l] << " ";
//    	cout << endl;
//    }
  }


}


void AltComponentAnalyzer::recordComponentOf(const VariableIndex var) {

  search_stack_.clear();
  search_stack_.push_back(var);

  archetype_.setVar_seen(var);

  for (auto vt = search_stack_.begin();
      vt != search_stack_.end(); vt++) {
    //BEGIN traverse binary clauses
    assert(isActive(*vt));
    unsigned *p = beginOfLinkList(*vt);
    for (; *p; p++) {
      if(isUnseenAndActive(*p)){
        setSeenAndStoreInSearchStack(*p);
        var_frequency_scores_[*p]++;
        var_frequency_scores_[*vt]++;
      }
    }
    //END traverse binary clauses

    p++;
    for (; *p ; p+=3) {
      if(archetype_.clause_unseen_in_sup_comp(*p)){
        const LiteralID * pstart_cls = reinterpret_cast<const LiteralID *>(p + 1);
        doThreeClauseB(*vt,*p, pstart_cls);
      }
    }
//    //END traverse ternary clauses
    //pvar++;
    // start traversing links to long clauses
    // not that that list starts right after the 0 termination of the prvious list
    // hence  pcl_ofs = pvar + 1

    for (p++; *p ; p +=2) {
      if(archetype_.clause_unseen_in_sup_comp(*p)){
        auto itVEnd = search_stack_.end();
        bool all_lits_active = true;
        LiteralID * pstart_cls = reinterpret_cast<LiteralID *>(p + 1 + *(p+1));
        for (auto itL = pstart_cls; *itL != SENTINEL_LIT; itL++) {
          assert(itL->var() <= max_variable_id_);
          if(archetype_.var_nil(itL->var())){
            assert(!isActive(*itL));
            all_lits_active = false;
            if (isResolved(*itL))
              continue;
            //BEGIN accidentally entered a satisfied clause: undo the search process
            while (search_stack_.end() != itVEnd) {
              assert(search_stack_.back() <= max_variable_id_);
              archetype_.setVar_in_sup_comp_unseen(search_stack_.back());
              search_stack_.pop_back();
            }
            archetype_.setClause_nil(*p);
            while(*itL != SENTINEL_LIT)
              if(isActive(*(--itL)))
                var_frequency_scores_[itL->var()]--;
            //END accidentally entered a satisfied clause: undo the search process
            break;
          } else {
            assert(isActive(*itL));
            var_frequency_scores_[itL->var()]++;
            if(isUnseenAndActive(itL->var()))
              setSeenAndStoreInSearchStack(itL->var());
          }
        }

        if (!archetype_.clause_nil(*p)){
          var_frequency_scores_[*vt]++;
          archetype_.setClause_seen(*p,all_lits_active);
        }
      }
    }
  }
}

//void AltComponentAnalyzer::recordComponentOf(const VariableIndex var) {
//
//  search_stack_.clear();
//  search_stack_.push_back(var);
//
//  archetype_.setVar_seen(var);
//
//  for (auto vt = search_stack_.begin(); vt != search_stack_.end(); vt++) {
//    // the for-loop is applicable here because componentSearchStack.capacity() == countAllVars()
//    //BEGIN traverse binary clauses
//    assert(isActive(*vt));
//    const unsigned *p = beginOfLinkList(*vt);
//    for (; *p; p++) {
//      if(isUnseenAndActive(*p)){
//        setSeenAndStoreInSearchStack(*p);
//        var_frequency_scores_[*p]++;
//        var_frequency_scores_[*vt]++;
//      }
//    }
//    //END traverse binary clauses
//
//    //BEGIN traverse ternary clauses
////    for (; *q ; q+=3, t+=2){
////
////       	const LiteralID * const qstart_cls = reinterpret_cast<const LiteralID *>(q + 1);
////       	const LiteralID * const tstart_cls = reinterpret_cast<const LiteralID *>(t + 1 + *(t+1));
//////       	if(archetype_.clause_unseen_in_sup_comp(*t)){
//////       	           doThreeClause(*vt,*t, tstart_cls);
//////       	}
////       	if(archetype_.clause_unseen_in_sup_comp(*q)){
////       	       	           doThreeClauseB(*vt,*q, qstart_cls);
////       	}
////
////    }
//    p++;
//    for (; *p ; p+=3) {
////         if(archetype_.clause_unseen_in_sup_comp(*p)){
////           const LiteralID * pstart_cls = reinterpret_cast<const LiteralID *>(p + 1);
////           doThreeClauseB(*vt,*p, pstart_cls);
////         }
//    }
////    //END traverse ternary clauses
//    p++;
//    for (; *p != SENTINEL_CL; p+=2) {
//      //ClauseIndex clID = *pcl_ofs;
//      if(archetype_.clause_unseen_in_sup_comp(*p)){
//        auto itVEnd = search_stack_.end();
//        bool all_lits_active = true;
//        const LiteralID * pstart_cls = reinterpret_cast<const LiteralID *>(p + 1 + *(p+1));
//        for (auto itL = pstart_cls; *itL != SENTINEL_LIT; itL++) {
//          assert(itL->var() <= max_variable_id_);
//          if(archetype_.var_nil(itL->var())){
//            assert(!isActive(*itL));
//            all_lits_active = false;
//            if (isResolved(*itL))
//              continue;
//            //BEGIN accidentally entered a satisfied clause: undo the search process
//            while (search_stack_.end() != itVEnd) {
//              assert(search_stack_.back() <= max_variable_id_);
//              archetype_.setVar_in_sup_comp_unseen(search_stack_.back());
//              search_stack_.pop_back();
//            }
//            archetype_.setClause_nil(*p);
//            // for (auto itX = beginOfClause(*(pcl_ofs+1)); itX != itL; itX++) {
//            while(*itL != SENTINEL_LIT)
//              if(isActive(*(--itL)))
//                var_frequency_scores_[itL->var()]--;
//            //END accidentally entered a satisfied clause: undo the search process
//            break;
//          } else {
//            assert(isActive(*itL));
//            var_frequency_scores_[itL->var()]++;
//            if(isUnseenAndActive(itL->var()))
//              setSeenAndStoreInSearchStack(itL->var());
//          }
//        }
//
//        if (!archetype_.clause_nil(*p)){
//          var_frequency_scores_[*vt]++;
//          archetype_.setClause_seen(*p,all_lits_active);
//        }
//      }
//    }
//   }
//}


//void AltComponentAnalyzer::recordComponentOf(const VariableIndex var) {
//
//  search_stack_.clear();
//  search_stack_.push_back(var);
//
//  archetype_.setVar_seen(var);
//
//  for (auto vt = search_stack_.begin();
//      vt != search_stack_.end(); vt++) {
//    // the for-loop is applicable here because componentSearchStack.capacity() == countAllVars()
//    //BEGIN traverse binary clauses
//    assert(isActive(*vt));
//    unsigned *pvar = beginOfLinkList(*vt);
//    for (; *pvar; pvar++) {
//      if(isUnseenAndActive(*pvar)){
//        setSeenAndStoreInSearchStack(*pvar);
//        var_frequency_scores_[*pvar]++;
//        var_frequency_scores_[*vt]++;
//      }
//    }
//    //END traverse binary clauses
//
//    //BEGIN traverse ternary clauses
//    auto pcl_ofs = pvar + 1;
////    for (; *pcl_ofs != SENTINEL_CL; pcl_ofs+=3)
////    	if(archetype_.clause_unseen_in_sup_comp(*pcl_ofs)){
////    		LiteralID litA = *reinterpret_cast<LiteralID *>(pcl_ofs + 1);
////    		LiteralID litB = *reinterpret_cast<LiteralID *>(pcl_ofs + 2);
////    		if(isSatisfied(litA) || isSatisfied(litA))
////    			archetype_.setClause_nil(*pcl_ofs);
////    		else {
////    			assert(!archetype_.clause_nil(*pcl_ofs));
////    			if(isActive(litA))
////    			 var_frequency_scores_[litA.var()]++;
////    			if(isActive(litB))
////    			 var_frequency_scores_[litB.var()]++;
////    			var_frequency_scores_[*vt]++;
////    			if(isUnseenAndActive(litA.var()))
////    				setSeenAndStoreInSearchStack(litA.var());
////    			if(isUnseenAndActive(litB.var()))
////    				setSeenAndStoreInSearchStack(litB.var());
////    			archetype_.setClause_seen(*pcl_ofs,isActive(litA) & isActive(litA));
////    		}
////    	}
////    for (; *pcl_ofs != SENTINEL_CL; pcl_ofs+=3)
////      if(archetype_.clause_unseen_in_sup_comp(*pcl_ofs)){
////    	        auto itVEnd = search_stack_.end();
////    	        bool all_lits_active = true;
////    	        LiteralID * pstart_cls = reinterpret_cast<LiteralID *>(pcl_ofs + 1);
////    	        for (auto itL = pstart_cls; itL != pstart_cls + 2; itL++) {
////    	          assert(itL->var() <= max_variable_id_);
////    	          if(archetype_.var_nil(itL->var())){
////    	            assert(!isActive(*itL));
////    	            all_lits_active = false;
////    	            if (isResolved(*itL))
////    	              continue;
////    	            //BEGIN accidentally entered a satisfied clause: undo the search process
////    	            while (search_stack_.end() != itVEnd) {
////    	              assert(search_stack_.back() <= max_variable_id_);
////    	              archetype_.setVar_in_sup_comp_unseen(search_stack_.back());
////    	              search_stack_.pop_back();
////    	            }
////    	            archetype_.setClause_nil(*pcl_ofs);
////    	           // for (auto itX = beginOfClause(*(pcl_ofs+1)); itX != itL; itX++) {
////    	            while(*itL != SENTINEL_LIT)
////    	           	  if(isActive(*(--itL)))
////    	           	    var_frequency_scores_[itL->var()]--;
////    	            //END accidentally entered a satisfied clause: undo the search process
////    	            break;
////    	          } else {
////    	            assert(isActive(*itL));
////    	            var_frequency_scores_[itL->var()]++;
////    	            if(isUnseenAndActive(itL->var()))
////    	              setSeenAndStoreInSearchStack(itL->var());
////    	          }
////    	        }
////
////    	        if (!archetype_.clause_nil(*pcl_ofs)){
////    	          var_frequency_scores_[*vt]++;
////    	          archetype_.setClause_seen(*pcl_ofs,all_lits_active);
////    	        }
////    	      }
//
//    for (; *pcl_ofs != SENTINEL_CL; pcl_ofs+=3) {
//       	//ClauseIndex clID = *pcl_ofs;
//    	assert(*pcl_ofs);
//    	assert(*(pcl_ofs+1));
//    	assert(*(pcl_ofs+2));
//
//         if(archetype_.clause_unseen_in_sup_comp(*pcl_ofs)){
//           LiteralID * pstart_cls = reinterpret_cast<LiteralID *>(pcl_ofs + 1);
//           doThreeClause(*vt,*pcl_ofs, pstart_cls);
//         }
//    }
//
//
//   /* for (; *pcl_ofs != SENTINEL_CL; pcl_ofs+=3) {
//
//    }*/
//    	// END traverse ternary clauses
//
//    // start traversing links to long clauses
//    // not that that list starts right after the 0 termination of the prvious list
//    // hence  pcl_ofs = pvar + 1
//    pcl_ofs+=3;
//    for (; *pcl_ofs != SENTINEL_CL; pcl_ofs+=2) {
//     	//ClauseIndex clID = *pcl_ofs;
//    	assert(!archetype_.clause_unseen_in_sup_comp(*pcl_ofs));
//       if(archetype_.clause_unseen_in_sup_comp(*pcl_ofs)){
//         LiteralID * pstart_cls = reinterpret_cast<LiteralID *>(pcl_ofs + 1 + *(pcl_ofs+1));
//         doThreeClause(*vt,*pcl_ofs,pstart_cls);
//       }
//     }
////    for (; *pcl_ofs != SENTINEL_CL; pcl_ofs+=2) {
////    	//ClauseIndex clID = *pcl_ofs;
////      if(archetype_.clause_unseen_in_sup_comp(*pcl_ofs)){
////        auto itVEnd = search_stack_.end();
////        bool all_lits_active = true;
////        LiteralID * pstart_cls = reinterpret_cast<LiteralID *>(pcl_ofs + 1 + *(pcl_ofs+1));
////        for (auto itL = pstart_cls; *itL != SENTINEL_LIT; itL++) {
////          assert(itL->var() <= max_variable_id_);
////          if(archetype_.var_nil(itL->var())){
////            assert(!isActive(*itL));
////            all_lits_active = false;
////            if (isResolved(*itL))
////              continue;
////            //BEGIN accidentally entered a satisfied clause: undo the search process
////            while (search_stack_.end() != itVEnd) {
////              assert(search_stack_.back() <= max_variable_id_);
////              archetype_.setVar_in_sup_comp_unseen(search_stack_.back());
////              search_stack_.pop_back();
////            }
////            archetype_.setClause_nil(*pcl_ofs);
////           // for (auto itX = beginOfClause(*(pcl_ofs+1)); itX != itL; itX++) {
////            while(*itL != SENTINEL_LIT)
////           	  if(isActive(*(--itL)))
////           	    var_frequency_scores_[itL->var()]--;
////            //END accidentally entered a satisfied clause: undo the search process
////            break;
////          } else {
////            assert(isActive(*itL));
////            var_frequency_scores_[itL->var()]++;
////            if(isUnseenAndActive(itL->var()))
////              setSeenAndStoreInSearchStack(itL->var());
////          }
////        }
////
////        if (!archetype_.clause_nil(*pcl_ofs)){
////          var_frequency_scores_[*vt]++;
////          archetype_.setClause_seen(*pcl_ofs,all_lits_active);
////        }
////      }
////    }
//  }
//}

