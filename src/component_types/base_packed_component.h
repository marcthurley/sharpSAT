/*
 * base_packed_component.h
 *
 *  Created on: Feb 5, 2013
 *      Author: mthurley
 */

#ifndef BASE_PACKED_COMPONENT_H_
#define BASE_PACKED_COMPONENT_H_

#include <assert.h>
#include <gmpxx.h>
#include <iostream>

using namespace std;


class BasePackedComponent {
public:
  static unsigned bits_per_variable() {
    return _bits_per_variable;
  }
  static unsigned bits_per_clause() {
    return _bits_per_clause;
  }

  static unsigned bits_per_block(){
	  return _bits_per_block;
  }

  static void adjustPackSize(unsigned int maxVarId, unsigned int maxClId);

  BasePackedComponent() {}
  BasePackedComponent(unsigned creation_time): creation_time_(creation_time) {}

  ~BasePackedComponent() {
    if (data_)
      delete data_;
  }

  unsigned data_size() const {
      if (!data_)
        return 0;
      unsigned *p = data_ + clauses_ofs_;
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

  unsigned size_of_model_count() const{
	  return sizeof(mpz_class)
		     + model_count_.get_mpz_t()->_mp_size * sizeof(mp_limb_t);
  }

  void set_creation_time(unsigned time) {
    creation_time_ = time;
  }

  void set_model_count(const mpz_class &rn) {
    model_count_ = rn;
  }

  unsigned hashkey() const  {
    return hashkey_;
  }

  inline bool equals(const BasePackedComponent &comp) const;

  inline bool data_only_equals(const BasePackedComponent &comp) const;

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
  unsigned creation_time_ = 1;

private:
  static unsigned _bits_per_clause, _bits_per_variable; // bitsperentry
  static unsigned _variable_mask, _clause_mask;
  static const unsigned _bits_per_block= (sizeof(unsigned) << 3);

};

//bool BasePackedComponent::equals(const BasePackedComponent &comp) const {
//  if(hashkey_ != comp.hashkey())
//    return false;
//  if (clauses_ofs_ != comp.clauses_ofs_)
//    return false;
//  unsigned* p = data_;
//  unsigned* r = comp.data_;
//  while (*p && *p == *r) {
//    p++;
//    r++;
//  }
//  return *p == *r;
//}


bool BasePackedComponent::equals(const BasePackedComponent &comp) const {
  if(hashkey_ != comp.hashkey())
    return false;
  if (clauses_ofs_ != comp.clauses_ofs_)
    return false;
  unsigned* p = data_;
  unsigned* r = comp.data_;
  while (((p - data_ < clauses_ofs_) || *p) && *p == *r) {
    p++;
    r++;
  }
  return *p == *r;
}


#endif /* BASE_PACKED_COMPONENT_H_ */
