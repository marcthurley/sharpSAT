/*
 * difference_packed_component.h
 *
 *  Created on: Feb 5, 2013
 *      Author: mthurley
 */

#ifndef DIFFERENCE_PACKED_COMPONENT_H_
#define DIFFERENCE_PACKED_COMPONENT_H_

#include "base_packed_component.h"

#include <math.h>

class DifferencePackedComponent:public BasePackedComponent {
public:

  DifferencePackedComponent() {
  }

  inline DifferencePackedComponent(Component &rComp, unsigned creation_time);

//  DifferencePackedComponent(Component &rComp, const mpz_class &model_count,
//      unsigned long time) :
//      DifferencePackedComponent(rComp) {
//    model_count_ = model_count;
//    creation_time_ = time;
//  }

//  ~DifferencePackedComponent() {
//    if (data_)
//      delete data_;
//  }

  // NOTE that the following is only an upper bound on
  // the number of varaibles
  // it might overcount by a few variables
  // this is due to the way things are packed
  // and to reduce time needed to compute this value
  unsigned num_variables() const{
    unsigned bits_per_var_diff = (*data_) & 31;
    return 1 + (clauses_ofs_ * sizeof(unsigned) * 8 - bits_per_variable() - 5) / bits_per_var_diff;
  }

};


DifferencePackedComponent::DifferencePackedComponent(Component &rComp, unsigned creation_time)
  : BasePackedComponent(creation_time) {
  unsigned max_diff = 0;

  for (auto it = rComp.varsBegin() + 1; *it != varsSENTINEL; it++) {
    if (*it - *(it - 1) > max_diff)
      max_diff = *it - *(it - 1);
  }

  unsigned bits_per_var_diff = (unsigned int) ceil(
      log((double) max_diff + 1) / log(2.0));
  if(bits_per_var_diff == 0)
	   bits_per_var_diff = 1;
  assert(bits_per_var_diff != 0);
  assert((bits_per_var_diff&31)!= 0);

  max_diff = 0;
  for (auto jt = rComp.clsBegin() + 1; *jt != clsSENTINEL; jt++) {
    if (*jt - *(jt - 1) > max_diff)
      max_diff = *jt - *(jt - 1);
  }

  unsigned bits_per_clause_diff = (unsigned int) ceil(
      log((double) max_diff + 1) / log(2.0));


  unsigned data_size = (bits_per_variable() + 5 + bits_per_clause() + 5
      + (rComp.num_variables() - 1) * bits_per_var_diff
      + (rComp.numLongClauses() - 1) * bits_per_clause_diff) / bits_per_block()
      + 3;

  unsigned * p = data_ = (unsigned*) malloc(sizeof(unsigned) * data_size);

  *p = bits_per_var_diff;
  unsigned int bitpos = 5;

  *p |= *rComp.varsBegin() << bitpos;
  bitpos += bits_per_variable();
  unsigned hashkey_vars = *rComp.varsBegin();

  for (auto it = rComp.varsBegin() + 1; *it != varsSENTINEL; it++) {
    *p |= ((*it) - *(it - 1)) << bitpos;
    bitpos += bits_per_var_diff;
    hashkey_vars = hashkey_vars * 3 + ((*it) - *(it - 1));
    if (bitpos >= bits_per_block()) {
      bitpos -= bits_per_block();
      *(++p) = (((*it) - *(it - 1)) >> (bits_per_var_diff - bitpos));
    }
  }
  if (bitpos > 0)
    p++;
  clauses_ofs_ = p - data_;

  unsigned hashkey_clauses = *rComp.clsBegin();
  if (*rComp.clsBegin()) {
    *p = bits_per_clause_diff;
    bitpos = 5;
    *p |= *rComp.clsBegin() << bitpos;
    bitpos += bits_per_clause();
    for (auto jt = rComp.clsBegin() + 1; *jt != clsSENTINEL; jt++) {
      *p |= ((*jt - *(jt - 1)) << (bitpos));
      bitpos += bits_per_clause_diff;
      hashkey_clauses = hashkey_clauses * 3 + (*jt - *(jt - 1));
      if (bitpos >= bits_per_block()) {
        bitpos -= bits_per_block();
        *(++p) = ((*jt - *(jt - 1)) >> (bits_per_clause_diff - bitpos));
      }
    }
    if (bitpos > 0)
      p++;
  }
  *p = 0;
  hashkey_ = hashkey_vars + (((unsigned long) hashkey_clauses) << 16);
}


#endif /* DIFFERENCE_PACKED_COMPONENT_H_ */
