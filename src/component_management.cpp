/*
 * component_management.cpp
 *
 *  Created on: Aug 23, 2012
 *      Author: Marc Thurley
 */

#include "component_management.h"

#include <algorithm>
#include <sys/sysinfo.h>
#include <cstdint>

ComponentCache::ComponentCache(SolverConfiguration &conf,
		DataAndStatistics &statistics) :
		config_(conf), statistics_(statistics) {
}

void ComponentCache::init() {
	table_.clear();
	entry_base_.clear();
	entry_base_.reserve(2000000);
	entry_base_.push_back(new CachedComponent()); // dummy Element
	table_.resize(900001, NULL);
	free_entry_base_slots_.clear();
	free_entry_base_slots_.reserve(10000);

	struct sysinfo info;
	sysinfo(&info);

	uint64_t free_ram =
		info.freeram *(uint64_t) info.mem_unit;
	uint64_t max_cache_bound = 95 * (free_ram / 100);

	if (config_.maximum_cache_size_bytes == 0) {
		config_.maximum_cache_size_bytes = max_cache_bound;
	}

	if (config_.maximum_cache_size_bytes > free_ram) {
		cout << endl <<" WARNING: Maximum cache size larger than free RAM available" << endl;
		cout << " Free RAM " << free_ram / 1000000 << "MB" << endl;
	}

	cout << "Maximum cache size:\t"
			<< config_.maximum_cache_size_bytes / 1000000 << " MB" << endl
			<< endl;

	recompute_bytes_memory_usage();
}

CacheEntryID ComponentCache::createEntryFor(Component &comp,
		unsigned stack_id) {
	my_time_++;
	CacheEntryID id;

	if (statistics_.cache_bytes_memory_usage_
			>= config_.maximum_cache_size_bytes) {
		deleteEntries();
	}

	assert(
			statistics_.cache_bytes_memory_usage_ < config_.maximum_cache_size_bytes);
	if (free_entry_base_slots_.empty()) {
		if (entry_base_.capacity() == entry_base_.size()) {
			entry_base_.reserve(2 * entry_base_.size());
		}
		entry_base_.push_back(new CachedComponent(comp));
		id = entry_base_.size() - 1;
	} else {
		id = free_entry_base_slots_.back();
		assert(id < entry_base_.size());
		assert(entry_base_[id] == nullptr);
		free_entry_base_slots_.pop_back();
		entry_base_[id] = new CachedComponent(comp);
	}
	entry_base_[id]->setComponentStackID(stack_id);
	entry_base_[id]->set_creation_time(my_time_);

	assert(entry_base_[id]->first_descendant() == 0);
	assert(entry_base_[id]->next_sibling() == 0);
	statistics_.cache_bytes_memory_usage_ += entry_base_[id]->SizeInBytes();
	statistics_.sum_size_cached_components_ += entry_base_[id]->num_variables();
	statistics_.num_cached_components_++;

#ifdef DEBUG
    for (unsigned u = 2; u < entry_base_.size(); u++)
          if (entry_base_[u] != nullptr) {
            assert(entry_base_[u]->father() != id);
            assert(entry_base_[u]->first_descendant() != id);
            assert(entry_base_[u]->next_sibling() != id);
          }
#endif
	return id;
}

void ComponentCache::test_descendantstree_consistency() {
	for (unsigned id = 2; id < entry_base_.size(); id++)
		if (entry_base_[id] != nullptr) {
			CacheEntryID act_child = entry(id).first_descendant();
			while (act_child) {
				CacheEntryID next_child = entry(act_child).next_sibling();
				assert(entry(act_child).father() == id);

				act_child = next_child;
			}
			CacheEntryID father = entry(id).father();
			CacheEntryID act_sib = entry(father).first_descendant();
#ifndef NDEBUG
			bool found = false;
#endif
			while (act_sib) {
				CacheEntryID next_sib = entry(act_sib).next_sibling();
#ifndef NDEBUG
				if (act_sib == id)
					found = true;
#endif
				act_sib = next_sib;
			}
			assert(found);
		}
}

uint64_t ComponentCache::recompute_bytes_memory_usage() {
	statistics_.cache_bytes_memory_usage_ = sizeof(ComponentCache)
			+ sizeof(CacheBucket *) * table_.capacity();
	for (auto pbucket : table_)
		if (pbucket != nullptr)
			statistics_.cache_bytes_memory_usage_ +=
					pbucket->getBytesMemoryUsage();
	for (auto pentry : entry_base_)
		if (pentry != nullptr) {
			statistics_.cache_bytes_memory_usage_ += pentry->SizeInBytes();
		}
	return statistics_.cache_bytes_memory_usage_;
}

bool ComponentCache::requestValueOf(Component &comp, mpz_class &rn) {
	CachedComponent &packedcomp = entry(comp.id());

	unsigned int v = clip(packedcomp.hashkey());
	if (!isBucketAt(v))
		return false;

	CachedComponent *pcomp;
	statistics_.num_cache_look_ups_++;

	for (auto it = table_[v]->begin(); it != table_[v]->end(); it++) {
		pcomp = &entry(*it);
		if (packedcomp.hashkey() == pcomp->hashkey()
				&& pcomp->equals(packedcomp)) {
			statistics_.num_cache_hits_++;
			statistics_.sum_cache_hit_sizes_ += pcomp->num_variables();
			rn = pcomp->model_count();
			//pComp->set_creation_time(my_time_);
			return true;
		}
	}
	return false;
}

bool ComponentCache::deleteEntries() {
	assert(
			statistics_.cache_bytes_memory_usage_ >= config_.maximum_cache_size_bytes);

	vector<double> scores;
	for (auto it = entry_base_.begin() + 1; it != entry_base_.end(); it++)
		if (*it != nullptr && (*it)->deletable()) {
			scores.push_back((double) (*it)->creation_time());
		}
	sort(scores.begin(), scores.end());
	double cutoff = scores[scores.size() / 2];

	//cout << "cutoff" << cutoff  << " entries: "<< entry_base_.size()<< endl;

	// first : go through the EntryBase and mark the entries to be deleted as deleted (i.e. EMPTY
	// note we start at index 2,
	// since index 1 is the whole formula,
	// should always stay here!
	for (unsigned id = 2; id < entry_base_.size(); id++)
		if (entry_base_[id] != nullptr && entry_base_[id]->deletable()) {
			double as = (double) entry_base_[id]->creation_time();
			if (as <= cutoff) {
				removeFromDescendantsTree(id);
				eraseEntry(id);
			}
		}
	// then go through the Hash Table and erase all Links to empty entries
	for (auto pbucket : table_)
		if (pbucket != nullptr) {
			for (auto bt = pbucket->rbegin(); bt != pbucket->rend(); bt++) {
				if (entry_base_[*bt] == nullptr) {
					*bt = pbucket->back();
					pbucket->pop_back();
				}
			}
		}
#ifdef DEBUG
	test_descendantstree_consistency();
#endif

	statistics_.sum_size_cached_components_ = 0;
	for (unsigned id = 2; id < entry_base_.size(); id++)
		if (entry_base_[id] != nullptr) {
			statistics_.sum_size_cached_components_ +=
					entry_base_[id]->num_variables();
		}

	statistics_.num_cached_components_ = entry_base_.size();
	statistics_.cache_bytes_memory_usage_ = recompute_bytes_memory_usage();

	//cout << " \t entries: "<< entry_base_.size() - free_entry_base_slots_.size()<< endl;
	return true;
}

void ComponentAnalyzer::initialize(LiteralIndexedVector<Literal> & literals,
		vector<LiteralID> &lit_pool) {

	cache_.init();
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
	memset(clauses_seen_, CA_NIL,
			sizeof(CA_SearchState) * (max_clause_id_ + 1));
	// the unified link list
	unified_variable_links_lists_pool_.clear();
	unified_variable_links_lists_pool_.push_back(0);
	unified_variable_links_lists_pool_.push_back(0);
	for (unsigned v = 1; v < occs_.size(); v++) {
		variable_link_list_offsets_[v] =
				unified_variable_links_lists_pool_.size();
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
	CachedComponent::adjustPackSize(max_variable_id_, max_clause_id_);
	initializeComponentStack();
}

bool ComponentAnalyzer::recordRemainingCompsFor(StackLevel &top) {
	Component & super_comp = superComponentOf(top);
	static mpz_class tmp_model_count;

	memset(clauses_seen_, CA_NIL,
			sizeof(CA_SearchState) * (max_clause_id_ + 1));
	memset(variables_seen_, CA_NIL,
			sizeof(CA_SearchState) * (max_variable_id_ + 1));

	for (auto vt = super_comp.varsBegin(); *vt != varsSENTINEL; vt++) {
		if (isActive(*vt)) {
			variables_seen_[*vt] = CA_IN_SUP_COMP;
			var_frequency_scores_[*vt] = 0;
		}
	}

	for (auto itCl = super_comp.clsBegin(); *itCl != clsSENTINEL; itCl++)
		clauses_seen_[*itCl] = CA_IN_SUP_COMP;

	unsigned new_comps_start_ofs = component_stack_.size();

	for (auto vt = super_comp.varsBegin(); *vt != varsSENTINEL; vt++)
		if (variables_seen_[*vt] == CA_IN_SUP_COMP) {
			recordComponentOf(*vt);
			if (component_search_stack_.size() == 1) {
				top.includeSolution(2);
				variables_seen_[*vt] = CA_IN_OTHER_COMP;
			} else {
				/////////////////////////////////////////////////
				// BEGIN store variables and clauses in component_stack_.back()
				// protocol is: variables first, then clauses
				/////////////////////////////////////////////////
				component_stack_.push_back(new Component());
				component_stack_.back()->reserveSpace(
						component_search_stack_.size(),
						super_comp.numLongClauses());

				for (auto v_it = super_comp.varsBegin(); *v_it != varsSENTINEL;
						v_it++)
					if (variables_seen_[*v_it] == CA_SEEN) { //we have to put a var into our component
						component_stack_.back()->addVar(*v_it);
						variables_seen_[*v_it] = CA_IN_OTHER_COMP;
					}
				component_stack_.back()->closeVariableData();

				for (auto it_cl = super_comp.clsBegin(); *it_cl != clsSENTINEL;
						it_cl++)
					if (clauses_seen_[*it_cl] == CA_SEEN) {
						component_stack_.back()->addCl(*it_cl);
						clauses_seen_[*it_cl] = CA_IN_OTHER_COMP;
					}
				component_stack_.back()->closeClauseData();
				/////////////////////////////////////////////////
				// END store variables in resComp
				/////////////////////////////////////////////////
				if (config_.perform_component_caching) {
					CacheEntryID id = cache_.createEntryFor(
							*component_stack_.back(),
							component_stack_.size() - 1);
					if (id != 0) {
						component_stack_.back()->set_id(id);
						// set up the father
						assert(cache_.hasEntry(id));
						assert(cache_.hasEntry(super_comp.id()));
						if (cache_.requestValueOf(*component_stack_.back(),
								tmp_model_count)) {
							top.includeSolution(tmp_model_count);
							cache_.eraseEntry(id);
							delete component_stack_.back();
							component_stack_.pop_back();
						} else {
							cache_.entry(id).set_father(super_comp.id());
							cache_.add_descendant(super_comp.id(), id);
						}
					}
				}
			}
		}

	top.set_unprocessed_components_end(component_stack_.size());

	assert(new_comps_start_ofs <= component_stack_.size());

	// sort the remaining components for processing
	for (unsigned i = new_comps_start_ofs; i < component_stack_.size(); i++)
		for (unsigned j = i + 1; j < component_stack_.size(); j++) {
			if (component_stack_[i]->num_variables()
					< component_stack_[j]->num_variables())
				swap(component_stack_[i], component_stack_[j]);
		}
	return true;
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
				for (auto itL = beginOfClause(*pcl_ofs); *itL != SENTINEL_LIT;
						itL++) {
					assert(itL->var() <= max_variable_id_);
					if (variables_seen_[itL->var()] == CA_NIL) { //i.e. the variable is not active
						if (isResolved(*itL))
							continue;
						//BEGIN accidentally entered a satisfied clause: undo the search process
						while (component_search_stack_.end() != itVEnd) {
							assert(
									component_search_stack_.back() <= max_variable_id_);
							variables_seen_[component_search_stack_.back()] =
									CA_IN_SUP_COMP;
							component_search_stack_.pop_back();
						}
						clauses_seen_[clID] = CA_NIL;
						for (auto itX = beginOfClause(*pcl_ofs); itX != itL;
								itX++) {
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

void ComponentAnalyzer::initializeComponentStack() {
	component_stack_.clear();
	component_stack_.reserve(max_variable_id_ + 2);
	component_stack_.push_back(new Component());
	component_stack_.push_back(new Component());
	assert(component_stack_.size() == 2);
	component_stack_.back()->createAsDummyComponent(max_variable_id_,
			max_clause_id_);
	CacheEntryID id = cache_.createEntryFor(*component_stack_.back(),
			component_stack_.size() - 1);
	component_stack_.back()->set_id(id);
	assert(id == 1);
}

void ComponentAnalyzer::removeAllCachePollutionsOf(StackLevel &top) {
	if (!config_.perform_component_caching)
		return;
	// all processed components are found in
	// [top.currentRemainingComponent(), component_stack_.size())
	// first, remove the list of descendants from the father
	assert(top.remaining_components_ofs() <= component_stack_.size());
	assert(top.super_component() != 0);
	assert(cache_.hasEntry(superComponentOf(top).id()));
	if (top.remaining_components_ofs() == component_stack_.size())
		return;

	for (unsigned u = top.remaining_components_ofs();
			u < component_stack_.size(); u++) {
		assert(cache_.hasEntry(component_stack_[u]->id()));
		cache_.cleanPollutionsInvolving(component_stack_[u]->id());
	}

#ifdef DEBUG
	cache_.test_descendantstree_consistency();
#endif
}
