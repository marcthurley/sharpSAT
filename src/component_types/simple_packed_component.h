/*
 * simple_packed_component.h
 *
 *  Created on: Feb 4, 2013
 *      Author: mthurley
 */

#ifndef SIMPLE_PACKED_COMPONENT_H_
#define SIMPLE_PACKED_COMPONENT_H_

#include "base_packed_component.h"

typedef unsigned ComponentDataType;

class SimplePackedComponent : public BasePackedComponent {
public:

  SimplePackedComponent() {
  }

  inline SimplePackedComponent(Component &rComp);

  SimplePackedComponent(Component &rComp, const mpz_class &model_count,
      unsigned long time) :
      SimplePackedComponent(rComp) {
    model_count_ = model_count;
    creation_time_ = time;
  }

//  ~SimplePackedComponent() {
//    if (data_)
//      delete data_;
//  }



  unsigned num_variables() {
      return (clauses_ofs_ * sizeof(unsigned) * 8) /bits_per_variable();
  }

};



SimplePackedComponent::SimplePackedComponent(Component &rComp) {
  unsigned data_size = sizeof(ComponentDataType)*((rComp.num_variables() * bits_per_variable()
      + rComp.numLongClauses() * bits_per_clause())/bits_per_block() + 3);

  ComponentDataType *p = data_ = (ComponentDataType *) malloc(data_size);

  *p = *rComp.varsBegin();
  unsigned hashkey_vars = *p;
  unsigned int bitpos = bits_per_variable();
  for (auto it = rComp.varsBegin()+1; *it != varsSENTINEL; it++) {
    *p |= ((*it) << bitpos);
    bitpos += bits_per_variable();
    hashkey_vars = (hashkey_vars *3) + (*it - *(it-1));
    if (bitpos >= bits_per_block()) {
      bitpos -= bits_per_block();
      *(++p) = ((*it) >> (bits_per_variable() - bitpos));
    }
  }
  if (bitpos > 0)
    p++;

  clauses_ofs_ = p - data_;

  unsigned hashkey_clauses = *p = *rComp.clsBegin();

  if (*rComp.clsBegin()) {
    bitpos = bits_per_clause();
    for (auto jt = rComp.clsBegin()+1; *jt != clsSENTINEL; jt++) {
      *p |= ((*jt) << (bitpos));
      bitpos += bits_per_clause();
      hashkey_clauses = (hashkey_clauses *3) + (*jt - *(jt-1));
      if (bitpos >= bits_per_block()) {
        bitpos -= bits_per_block();
        *(++p) = ((*jt) >> (bits_per_clause() - bitpos));
      }
    }
    if (bitpos > 0)
      p++;
  }
  *p = 0;
  hashkey_ = hashkey_vars + (((unsigned) hashkey_clauses) << 16);
}


#endif /* SIMPLE_PACKED_COMPONENT_H_ */
