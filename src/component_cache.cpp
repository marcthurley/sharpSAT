/*
 * component_cache.cpp
 *
 *  Created on: Feb 5, 2013
 *      Author: mthurley
 */

#include "component_cache.h"

#include <algorithm>
#include <sys/sysinfo.h>
#include <cstdint>

#include "stack.h"


ComponentCache::ComponentCache(DataAndStatistics &statistics) :
		statistics_(statistics) {
}

void ComponentCache::init(Component &super_comp) {

    cout << sizeof(CachedComponent) << " " << sizeof(mpz_class) << endl;
	CachedComponent &packed_super_comp = *new CachedComponent(super_comp);
	my_time_ = 1;

	entry_base_.clear();
	entry_base_.reserve(2000000);
	entry_base_.push_back(new CachedComponent()); // dummy Element
	new_table_.clear();
	new_table_.resize(2000003, 0);
	free_entry_base_slots_.clear();
	free_entry_base_slots_.reserve(10000);

	struct sysinfo info;
	sysinfo(&info);

	uint64_t free_ram =	info.freeram *(uint64_t) info.mem_unit;
	uint64_t max_cache_bound = 95 * (free_ram / 100);

	if (statistics_.maximum_cache_size_bytes_ == 0) {
	  statistics_.maximum_cache_size_bytes_ = max_cache_bound;
	}

	if (statistics_.maximum_cache_size_bytes_ > free_ram) {
		cout << endl <<" WARNING: Maximum cache size larger than free RAM available" << endl;
		cout << " Free RAM " << free_ram / 1000000 << "MB" << endl;
	}

	cout << "Maximum cache size:\t"
			<< statistics_.maximum_cache_size_bytes_ / 1000000 << " MB" << endl
			<< endl;

	recompute_bytes_memory_usage();

	assert(!statistics_.cache_full());

	if (entry_base_.capacity() == entry_base_.size())
		entry_base_.reserve(2 * entry_base_.size());

	entry_base_.push_back(&packed_super_comp);

	statistics_.incorporate_cache_store(packed_super_comp);

	super_comp.set_id(1);
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

			bool found = false;

			while (act_sib) {
				CacheEntryID next_sib = entry(act_sib).next_sibling();
				if (act_sib == id)
					found = true;
				act_sib = next_sib;
			}
			assert(found);
		}
}

uint64_t ComponentCache::recompute_bytes_memory_usage() {
  statistics_.cache_bytes_memory_usage_ = sizeof(ComponentCache)
              + sizeof(CacheEntryID)* new_table_.capacity();
      for (auto pentry : entry_base_)
          if (pentry != nullptr) {
              statistics_.cache_bytes_memory_usage_ += pentry->SizeInBytes();
          }
      return statistics_.cache_bytes_memory_usage_;
}



bool ComponentCache::deleteEntries() {
  assert(statistics_.cache_full());

	vector<double> scores;
	for (auto it = entry_base_.begin() + 1; it != entry_base_.end(); it++)
		if (*it != nullptr && (*it)->isDeletable()) {
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
		if (entry_base_[id] != nullptr &&
		    entry_base_[id]->isDeletable() &&
		      (double) entry_base_[id]->creation_time() <= cutoff) {
				removeFromDescendantsTree(id);
				eraseEntry(id);

        }
	// then go through the Hash Table and erase all Links to empty entries


#ifdef DEBUG
	test_descendantstree_consistency();
#endif

	reHashTable(new_table_.size());
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
