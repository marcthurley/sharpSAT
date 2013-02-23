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

  unsigned num_variables() {
      return *data_;
  }

};


SimpleUnpackedComponent::SimpleUnpackedComponent(Component &rComp) {
  _end_clause_mask = 4294967295;


  unsigned data_size = rComp.num_variables() +  rComp.numLongClauses() + 2;

  unsigned *p = data_ =  new unsigned[data_size];
  *p = rComp.num_variables();
  unsigned hashkey_vars = 0;
  for (auto it = rComp.varsBegin(); *it != varsSENTINEL; it++) {
    *(++p) = *it;
    hashkey_vars = (hashkey_vars *3) + *it;
  }

  unsigned hashkey_clauses = 0;

  for (auto jt = rComp.clsBegin(); *jt != clsSENTINEL; jt++) {
      *(++p) = *jt;
      hashkey_clauses = (hashkey_clauses *3) + *jt;
  }
  *(++p) = 0;

  hashkey_ = hashkey_vars + (((unsigned) hashkey_clauses) << 16);

  assert(p - data_ + 1 == data_size);

}



#endif /* SIMPLE_UNPACKED_COMPONENT_H_ */
