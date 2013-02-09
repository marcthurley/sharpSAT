/*
 * difference_packed_component.h
 *
 *  Created on: Feb 5, 2013
 *      Author: mthurley
 */

#ifndef DIFFERENCE_PACKED_COMPONENT_H_
#define DIFFERENCE_PACKED_COMPONENT_H_

#include "base_packed_component.h"
#include "component_archetype.h"

#include <math.h>

class DifferencePackedComponent:public BasePackedComponent {
public:

  DifferencePackedComponent() {
  }

  inline DifferencePackedComponent(Component &rComp, unsigned creation_time);
  inline DifferencePackedComponent(ComponentArchetype &archetype, unsigned creation_time);
  // inline DifferencePackedComponent(Component &rComp, ComponentArchetype &archetype, unsigned creation_time);




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



//DifferencePackedComponent::DifferencePackedComponent(Component &rComp, ComponentArchetype &archetype, unsigned creation_time)
//  : BasePackedComponent(creation_time) {
//
//  unsigned max_diff = 0;
//  unsigned num_variables =1;
//  unsigned num_long_clauses =0;
//
//  auto vfirst_it = archetype.super_comp().varsBegin();
//  while(!archetype.var_seen(*vfirst_it) && *vfirst_it != varsSENTINEL) ++vfirst_it;
//  auto vprev_it = vfirst_it;
//  assert(*vfirst_it != varsSENTINEL);
//
//  for (auto v_it = vfirst_it+1; *v_it != varsSENTINEL; v_it++)
//    if (archetype.var_seen(*v_it)) { //we have to put a var into our component
//      num_variables++;
//      if (*v_it - *vprev_it > max_diff)
//            max_diff = *v_it - *vprev_it;
//      vprev_it = v_it;
//    }
//
//  unsigned bits_per_var_diff = (unsigned int) ceil(
//        log((double) max_diff + 1) / log(2.0));
//    if(bits_per_var_diff == 0)
//         bits_per_var_diff = 1;
//    assert(bits_per_var_diff != 0);
//    assert((bits_per_var_diff&31)!= 0);
//
//
//  max_diff = 0;
//
//  auto cfirst_it = archetype.super_comp().clsBegin();
//  while(!archetype.clause_seen(*cfirst_it) && (*cfirst_it != clsSENTINEL)) ++cfirst_it;
//  auto cprev_it = cfirst_it;
//
//  if(*cfirst_it != clsSENTINEL){
//    num_long_clauses=1;
//    for (auto it_cl = cfirst_it+1; *it_cl != clsSENTINEL; it_cl++)
//      if (archetype.clause_seen(*it_cl)) {
//        num_long_clauses++;
//        if (*it_cl - *cprev_it > max_diff)
//           max_diff = *it_cl - *cprev_it;
//        cprev_it = it_cl;
//      }
//  }
//
//  unsigned bits_per_clause_diff = (unsigned int) ceil(
//        log((double) max_diff + 1) / log(2.0));
//
//
//  unsigned data_size = (bits_per_variable() + 5 + bits_per_clause() + 5
//        + (num_variables - 1) * bits_per_var_diff
//        + (num_long_clauses - 1) * bits_per_clause_diff) / bits_per_block()
//        + 3;
//
//  /////////////////////////////
//
//  unsigned * p = data_ = (unsigned*) malloc(sizeof(unsigned) * data_size);
//
//  *p = bits_per_var_diff;
//  unsigned int bitpos = 5;
//
//  *p |= *vfirst_it << bitpos;
//  bitpos += bits_per_variable();
//  unsigned hashkey_vars = *vfirst_it;
//
//  vprev_it = vfirst_it;
//  for (auto it = vfirst_it + 1; *it != varsSENTINEL; it++)
//    if (archetype.var_seen(*it)) {
//      *p |= (*it - *vprev_it) << bitpos;
//      bitpos += bits_per_var_diff;
//      hashkey_vars = hashkey_vars * 3 + (*it - *vprev_it);
//      if (bitpos >= bits_per_block()) {
//        bitpos -= bits_per_block();
//        *(++p) = ((*it - *vprev_it) >> (bits_per_var_diff - bitpos));
//      }
//      vprev_it = it;
//    }
//  if (bitpos > 0)
//    p++;
//  clauses_ofs_ = p - data_;
//
//  unsigned hashkey_clauses = *cfirst_it;
//  cprev_it = cfirst_it;
//  if (*cfirst_it) {
//    *p = bits_per_clause_diff;
//    bitpos = 5;
//    *p |= *cfirst_it << bitpos;
//    bitpos += bits_per_clause();
//    for (auto jt = cfirst_it + 1; *jt != clsSENTINEL; jt++)
//      if (archetype.clause_seen(*jt)) {
//        *p |= ((*jt - *cprev_it) << (bitpos));
//        bitpos += bits_per_clause_diff;
//        hashkey_clauses = hashkey_clauses * 3 + (*jt - *cprev_it);
//        if (bitpos >= bits_per_block()) {
//          bitpos -= bits_per_block();
//          *(++p) = ((*jt - *cprev_it) >> (bits_per_clause_diff - bitpos));
//        }
//        cprev_it = jt;
//      }
//    if (bitpos > 0)
//      p++;
//  }
//  *p = 0;
//  hashkey_ = hashkey_vars + (((unsigned long) hashkey_clauses) << 16);
//}
//



//  : BasePackedComponent(creation_time) {
//  unsigned old_max_diff = 0;
//
//  for (auto it = rComp.varsBegin() + 1; *it != varsSENTINEL; it++) {
//    if (*it - *(it - 1) > old_max_diff)
//      old_max_diff = *it - *(it - 1);
//  }
//
//  /////////////////////////////
//    unsigned max_diff = 0;
//    unsigned num_variables =1;
//    unsigned num_long_clauses =0;
//
//    auto vfirst_it = archetype.super_comp().varsBegin();
//    while(!archetype.var_seen(*vfirst_it) && *vfirst_it != varsSENTINEL) ++vfirst_it;
//    auto vprev_it = vfirst_it;
//    assert(*vfirst_it != varsSENTINEL);
//
//    for (auto v_it = vfirst_it+1; *v_it != varsSENTINEL; v_it++)
//      if (archetype.var_seen(*v_it)) { //we have to put a var into our component
//        num_variables++;
//        if (*v_it - *vprev_it > max_diff)
//              max_diff = *v_it - *vprev_it;
//        vprev_it = v_it;
//      }
//
//   assert(old_max_diff == max_diff);
//   /////////////////////////////////////////////////
//
//
//
//  unsigned bits_per_var_diff = (unsigned int) ceil(
//      log((double) max_diff + 1) / log(2.0));
//  if(bits_per_var_diff == 0)
//       bits_per_var_diff = 1;
//  assert(bits_per_var_diff != 0);
//  assert((bits_per_var_diff&31)!= 0);
//
//  old_max_diff = 0;
//  for (auto jt = rComp.clsBegin() + 1; *jt != clsSENTINEL; jt++) {
//    if (*jt - *(jt - 1) > old_max_diff)
//      old_max_diff = *jt - *(jt - 1);
//  }
//
//  ////////
//    max_diff = 0;
//
//    auto cfirst_it = archetype.super_comp().clsBegin();
//    while(!archetype.clause_seen(*cfirst_it) && (*cfirst_it != clsSENTINEL)) ++cfirst_it;
//    auto cprev_it = cfirst_it;
//
//    if(*cfirst_it != clsSENTINEL){
//
//      num_long_clauses=1;
//      for (auto it_cl = cfirst_it+1; *it_cl != clsSENTINEL; it_cl++)
//        if (archetype.clause_seen(*it_cl)) {
//          num_long_clauses++;
//          if (*it_cl - *cprev_it > max_diff)
//             max_diff = *it_cl - *cprev_it;
//          cprev_it = it_cl;
//        }
//    }
//
//    if(old_max_diff != max_diff)
//      cout << "(" << old_max_diff << "," << max_diff << ")" << endl;
//    assert(1 & (old_max_diff == max_diff));
//
//  //////////
//
//  unsigned bits_per_clause_diff = (unsigned int) ceil(
//      log((double) max_diff + 1) / log(2.0));
//
//  assert(rComp.num_variables() == num_variables);
//  assert(rComp.numLongClauses() == num_long_clauses);
//
//  unsigned data_size = (bits_per_variable() + 5 + bits_per_clause() + 5
//      + (rComp.num_variables() - 1) * bits_per_var_diff
//      + (rComp.numLongClauses() - 1) * bits_per_clause_diff) / bits_per_block()
//      + 3;
//
//  unsigned * p = data_ = (unsigned*) malloc(sizeof(unsigned) * data_size);
//
//  *p = bits_per_var_diff;
//  unsigned int bitpos = 5;
//
//  *p |= *rComp.varsBegin() << bitpos;
//  bitpos += bits_per_variable();
//  unsigned hashkey_vars = *rComp.varsBegin();
//
//  for (auto it = rComp.varsBegin() + 1; *it != varsSENTINEL; it++) {
//    *p |= ((*it) - *(it - 1)) << bitpos;
//    bitpos += bits_per_var_diff;
//    hashkey_vars = hashkey_vars * 3 + ((*it) - *(it - 1));
//    if (bitpos >= bits_per_block()) {
//      bitpos -= bits_per_block();
//      *(++p) = (((*it) - *(it - 1)) >> (bits_per_var_diff - bitpos));
//    }
//  }
//  if (bitpos > 0)
//    p++;
//  clauses_ofs_ = p - data_;
//
//  unsigned hashkey_clauses = *rComp.clsBegin();
//  if (*rComp.clsBegin()) {
//    *p = bits_per_clause_diff;
//    bitpos = 5;
//    *p |= *rComp.clsBegin() << bitpos;
//    bitpos += bits_per_clause();
//    for (auto jt = rComp.clsBegin() + 1; *jt != clsSENTINEL; jt++) {
//      *p |= ((*jt - *(jt - 1)) << (bitpos));
//      bitpos += bits_per_clause_diff;
//      hashkey_clauses = hashkey_clauses * 3 + (*jt - *(jt - 1));
//      if (bitpos >= bits_per_block()) {
//        bitpos -= bits_per_block();
//        *(++p) = ((*jt - *(jt - 1)) >> (bits_per_clause_diff - bitpos));
//      }
//    }
//    if (bitpos > 0)
//      p++;
//  }
//  *p = 0;
//  hashkey_ = hashkey_vars + (((unsigned long) hashkey_clauses) << 16);
//}

DifferencePackedComponent::DifferencePackedComponent(
    ComponentArchetype &archetype, unsigned creation_time) :
    BasePackedComponent(creation_time) {

  unsigned max_diff = 0;
  unsigned num_variables = 1;
  unsigned num_long_clauses = 0;

  auto vfirst_it = archetype.super_comp().varsBegin();
  while (!archetype.var_seen(*vfirst_it) && *vfirst_it != varsSENTINEL)
    ++vfirst_it;
  auto vprev_it = vfirst_it;
  assert(*vfirst_it != varsSENTINEL);

  for (auto v_it = vfirst_it + 1; *v_it != varsSENTINEL; v_it++)
    if (archetype.var_seen(*v_it)) { //we have to put a var into our component
      num_variables++;
      if (*v_it - *vprev_it > max_diff)
        max_diff = *v_it - *vprev_it;
      vprev_it = v_it;
    }

  unsigned bits_per_var_diff = (unsigned int) ceil(
      log((double) max_diff + 1) / log(2.0));
  if (bits_per_var_diff == 0)
    bits_per_var_diff = 1;
  assert(bits_per_var_diff != 0);
  assert((bits_per_var_diff&31)!= 0);

  max_diff = 0;

  auto cfirst_it = archetype.super_comp().clsBegin();
  while (!archetype.clause_seen(*cfirst_it) && (*cfirst_it != clsSENTINEL))
    ++cfirst_it;
  auto cprev_it = cfirst_it;

  if (*cfirst_it != clsSENTINEL) {
    num_long_clauses = 1;
    for (auto it_cl = cfirst_it + 1; *it_cl != clsSENTINEL; it_cl++)
      if (archetype.clause_seen(*it_cl)) {
        num_long_clauses++;
        if (*it_cl - *cprev_it > max_diff)
          max_diff = *it_cl - *cprev_it;
        cprev_it = it_cl;
      }
  }

  unsigned bits_per_clause_diff = (unsigned int) ceil(
      log((double) max_diff + 1) / log(2.0));

  unsigned data_size = (bits_per_variable() + 5 + bits_per_clause() + 5
      + (num_variables - 1) * bits_per_var_diff
      + (num_long_clauses - 1) * bits_per_clause_diff) / bits_per_block() + 3;

/////////////////////////////

  unsigned * p = data_ = (unsigned*) malloc(sizeof(unsigned) * data_size);

  *p = bits_per_var_diff;
  unsigned int bitpos = 5;

  *p |= *vfirst_it << bitpos;
  bitpos += bits_per_variable();
  unsigned hashkey_vars = *vfirst_it;

  vprev_it = vfirst_it;
  for (auto it = vfirst_it + 1; *it != varsSENTINEL; it++)
    if (archetype.var_seen(*it)) {
      *p |= (*it - *vprev_it) << bitpos;
      bitpos += bits_per_var_diff;
      hashkey_vars = hashkey_vars * 3 + (*it - *vprev_it);
      if (bitpos >= bits_per_block()) {
        bitpos -= bits_per_block();
        *(++p) = ((*it - *vprev_it) >> (bits_per_var_diff - bitpos));
      }
      vprev_it = it;
    }
  if (bitpos > 0)
    p++;
  clauses_ofs_ = p - data_;

  unsigned hashkey_clauses = *cfirst_it;
  cprev_it = cfirst_it;
  if (*cfirst_it) {
    *p = bits_per_clause_diff;
    bitpos = 5;
    *p |= *cfirst_it << bitpos;
    bitpos += bits_per_clause();
    for (auto jt = cfirst_it + 1; *jt != clsSENTINEL; jt++)
      if (archetype.clause_seen(*jt)) {
        *p |= ((*jt - *cprev_it) << (bitpos));
        bitpos += bits_per_clause_diff;
        hashkey_clauses = hashkey_clauses * 3 + (*jt - *cprev_it);
        if (bitpos >= bits_per_block()) {
          bitpos -= bits_per_block();
          *(++p) = ((*jt - *cprev_it) >> (bits_per_clause_diff - bitpos));
        }
        cprev_it = jt;
      }
    if (bitpos > 0)
      p++;
  }
  *p = 0;
  hashkey_ = hashkey_vars + (((unsigned long) hashkey_clauses) << 16);
}
#endif /* DIFFERENCE_PACKED_COMPONENT_H_ */
