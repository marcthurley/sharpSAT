/*
 * component_types.h
 *
 *  Created on: Aug 23, 2012
 *      Author: Marc Thurley
 */

#ifndef COMPONENT_TYPES_H_
#define COMPONENT_TYPES_H_

#include <assert.h>
#include <vector>
#include <math.h>

#include "basic_types.h"
#include "structures.h"

using namespace std;

#define NIL_ENTRY 0

typedef unsigned CacheEntryID;
typedef unsigned ComponentDataType;

// State values for variables found during component
// analysis (CA)
typedef unsigned char CA_SearchState;
#define   CA_NIL  0
#define   CA_IN_SUP_COMP  1
#define   CA_SEEN 2
#define   CA_IN_OTHER_COMP  3

#define varsSENTINEL  0
#define clsSENTINEL   NOT_A_CLAUSE

//  the identifier of the components

class Component {
public:

  void reserveSpace(unsigned int num_variables, unsigned int num_clauses) {
    data_.reserve(num_variables + num_clauses + 2);
  }

  void set_id(CacheEntryID id) {
    id_ = id;
  }

  CacheEntryID id() {
    return id_;
  }

  void addVar(const VariableIndex var) {
    // the only time a varsSENTINEL is added should be in a
    // call to closeVariableData(..)
    assert(var != varsSENTINEL);
    data_.push_back(var);
  }

  void closeVariableData() {
    data_.push_back(varsSENTINEL);
    clauses_ofs_ = data_.size();
  }

  void addCl(const ClauseIndex cl) {
    // the only time a clsSENTINEL is added should be in a
    // call to closeClauseData(..)
    assert(cl != clsSENTINEL);
    data_.push_back(cl);
  }

  void closeClauseData() {
    data_.push_back(clsSENTINEL);
    assert(*(clsBegin()-1) == 0);
  }

  vector<VariableIndex>::const_iterator varsBegin() const {
    return data_.begin();
  }

  vector<ClauseIndex>::const_iterator clsBegin() const {
    return data_.begin() + clauses_ofs_;
  }

  unsigned num_variables() const {
    return clauses_ofs_ - 1;
  }

  unsigned numLongClauses() const {
    return data_.size() - clauses_ofs_ - 1;
  }

  bool empty() const {
    return data_.empty();
  }

  void createAsDummyComponent(unsigned max_var_id, unsigned max_clause_id) {
    data_.clear();
    clauses_ofs_ = 1;
    for (unsigned idvar = 1; idvar <= max_var_id; idvar++)
      addVar(idvar);
    closeVariableData();
    if (max_clause_id > 0)
      for (unsigned idcl = 1; idcl <= max_clause_id; idcl++)
        addCl(idcl);
    closeClauseData();
  }

private:
  // data_ stores the component data:
  // for better cache performance the
  // clause and variable data are stored in
  // a contiguous piece of memory
  // variables SENTINEL clauses SENTINEL
  // this order has to be taken care of on filling
  // in the data!
  vector<unsigned> data_;
  unsigned clauses_ofs_ = 0;
  // id_ will identify denote the entry in the cacheable component database,
  // where a Packed version of this component is stored
  // yet this does not imply that the model count of this component is already known
  // once the model count is known, a link to the packed component will be stored
  // in the hash table
  CacheEntryID id_ = 0;

};

class PackedComponent {
public:
  static unsigned bits_per_variable() {
    return _bits_per_variable;
  }
  static unsigned bits_per_clause() {
    return _bits_per_clause;
  }

  static void adjustPackSize(unsigned int maxVarId, unsigned int maxClId);

  PackedComponent() {
  }

  inline PackedComponent(Component &rComp);

  PackedComponent(Component &rComp, const mpz_class &model_count,
      unsigned long time) :
      PackedComponent(rComp) {
    model_count_ = model_count;
    creation_time_ = time;
  }

  ~PackedComponent() {
    if (data_)
      delete data_;
  }

  unsigned data_size() const {
    if (!data_)
      return 0;
    unsigned *p = data_;
    while (*p)
      p++;
    return (p - data_ + 1);
  }

  unsigned creation_time() {
    return creation_time_;
  }

  const mpz_class &model_count() const {
    return model_count_;
  }

  void set_creation_time(unsigned time) {
    creation_time_ = time;
  }

  void set_model_count(const mpz_class &rn) {
    model_count_ = rn;
  }

  unsigned hashkey() {
    return hashkey_;
  }

  // NOTE that the following is only an upper bound on
  // the number of varaibles
  // it might overcount by a few variables
  // this is due to the way things are packed
  // and to reduce time needed to compute this value
  unsigned num_variables() {
    unsigned bits_per_var_diff = (*data_) & 31;
    return 1 + (clauses_ofs_ * sizeof(unsigned) * 8 - _bits_per_variable - 5) / bits_per_var_diff;
  }
//  unsigned num_variables() {
//      return (clauses_ofs_ * sizeof(unsigned) * 8) /_bits_per_variable;
//  }

  inline bool equals(const PackedComponent &comp) const;

protected:
  // data_ contains in packed form the variable indices
  // and clause indices of the component ordered
  // structure is
  // var var ... clause clause ...
  // clauses begin at clauses_ofs_
  unsigned* data_ = nullptr;
  unsigned clauses_ofs_ = 0;
  unsigned hashkey_ = 0;

  mpz_class model_count_;
  unsigned creation_time_ = 0;

private:
  static unsigned _bits_per_clause, _bits_per_variable; // bitsperentry
  static unsigned _variable_mask, _clause_mask;
  static const unsigned _bitsPerBlock = (sizeof(unsigned) << 3);

};

// CachedComponent Adds Structure to PackedComponent that is
// necessary to store it in the cache
// namely, the descendant tree structure that
// allows for the removal of cache pollutions
class CachedComponent: public PackedComponent {

  // the position where this
  // component is stored in the component stack
  // if this is non-zero, we may not simply delete this
  // component
  unsigned component_stack_id_ = 0;

  // theFather and theDescendants:
  // each CCacheEntry is a Node in a tree which represents the relationship
  // of the components stored
  CacheEntryID father_ = 0;
  CacheEntryID first_descendant_ = 0;
  CacheEntryID next_sibling_ = 0;

public:
  // a cache entry is deletable
  // only if it is not connected to an active
  // component in the component stack
  bool deletable() {
    return component_stack_id_ == 0;
  }
  void eraseComponentStackID() {
    component_stack_id_ = 0;
  }
  void setComponentStackID(unsigned id) {
    component_stack_id_ = id;
  }
  unsigned component_stack_id() {
    return component_stack_id_;
  }

  void clear() {
    // before deleting the contents of this component,
    // we should make sure that this component is not present in the component stack anymore!
    assert(component_stack_id_ == 0);
    if (data_)
      delete data_;
    data_ = nullptr;
  }

  CachedComponent() {
  }

  CachedComponent(Component &comp, const mpz_class &model_count,
      unsigned long time) :
      PackedComponent(comp, model_count, time) {
  }

  CachedComponent(Component &comp) :
      PackedComponent(comp) {
  }
  unsigned long SizeInBytes() const {
    return sizeof(CachedComponent)
        + PackedComponent::data_size() * sizeof(unsigned)
        // and add the memory usage of model_count_
        // which is:
        + sizeof(mpz_class)
        + model_count().get_mpz_t()->_mp_size * sizeof(mp_limb_t);
  }

  // BEGIN Cache Pollution Management

  void set_father(CacheEntryID f) {
    father_ = f;
  }
  const CacheEntryID father() const {
    return father_;
  }

  void set_next_sibling(CacheEntryID sibling) {
    next_sibling_ = sibling;
  }
  CacheEntryID next_sibling() {
    return next_sibling_;
  }

  void set_first_descendant(CacheEntryID descendant) {
    first_descendant_ = descendant;
  }
  CacheEntryID first_descendant() {
    return first_descendant_;
  }
};

class CacheBucket: protected vector<CacheEntryID> {
  friend class ComponentCache;

public:

  using vector<CacheEntryID>::size;

  unsigned long getBytesMemoryUsage() {
    return sizeof(CacheBucket) + size() * sizeof(CacheEntryID);
  }
};

#include "component_types-inl.h"

#endif /* COMPONENT_TYPES_H_ */
