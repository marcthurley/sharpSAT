/*
 * simple_unpacked_component.h
 *
 *  Created on: Feb 16, 2013
 *      Author: mthurley
 */

#ifndef SIMPLE_UNPACKED_COMPONENT_H_
#define SIMPLE_UNPACKED_COMPONENT_H_

#include "base_packed_component.h"
#include "component_archetype.h"
#include "component.h"


#include "../primitive_types.h"

class SimpleUnpackedComponent : public BasePackedComponent {
public:

  SimpleUnpackedComponent() {
  }

  inline SimpleUnpackedComponent(Component &rComp);

//  SimpleUnpackedComponent(Component &rComp, const mpz_class &model_count,
//      unsigned long time) :
//        SimpleUnpackedComponent(rComp) {
//    model_count_ = model_count;
//    creation_time_ = time;
//  }

  unsigned num_variables() {
      return (clauses_ofs_ * sizeof(unsigned) * 8) /bits_per_variable();
  }

};


SimpleUnpackedComponent::SimpleUnpackedComponent(Component &rComp) {
  unsigned data_size = rComp.num_variables() +  rComp.numLongClauses() + 1;

  unsigned *p = data_ =  new unsigned[data_size];

  *p = *rComp.varsBegin();
  unsigned hashkey_vars = *p;
  for (auto it = rComp.varsBegin()+1; *it != varsSENTINEL; it++) {
    *(++p) = *it;
    hashkey_vars = (hashkey_vars *3) + (*it - *(it-1));
  }

  clauses_ofs_ = (++p) - data_;

 // assert(clauses_ofs_ == rComp.num_variables());
  unsigned hashkey_clauses = *p = *rComp.clsBegin();

  if (*rComp.clsBegin()) {
    for (auto jt = rComp.clsBegin()+1; *jt != clsSENTINEL; jt++) {
      *(++p) = *jt;
      hashkey_clauses = (hashkey_clauses *3) + (*jt - *(jt-1));
    }
    *(++p) = 0;
  }
  hashkey_ = hashkey_vars + (((unsigned) hashkey_clauses) << 16);

 // assert(p - data_ + 1 == data_size);
}



#endif /* SIMPLE_UNPACKED_COMPONENT_H_ */
