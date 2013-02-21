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
#include "component.h"

#include <math.h>



class DifferencePackedComponent:public BasePackedComponent {
public:

  DifferencePackedComponent() {
  }

  inline DifferencePackedComponent(Component &rComp);

  unsigned num_variables() const{
    return (*data_) & variable_mask();
  }

  unsigned log2(unsigned v){
       // taken from
       // http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogLookup
       static const char LogTable256[256] =
       {
       #define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
           -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
           LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
           LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
       };

       unsigned r;     // r will be lg(v)
       register unsigned int t, tt; // temporaries

       if (tt = v >> 16)
       {
         r = (t = tt >> 8) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
       }
       else
       {
         r = (t = v >> 8) ? 8 + LogTable256[t] : LogTable256[v];
       }
       return r;
     }
};


DifferencePackedComponent::DifferencePackedComponent(Component &rComp) {

  unsigned max_var_diff = 0;
  unsigned hashkey_vars = *rComp.varsBegin();
  for (auto it = rComp.varsBegin() + 1; *it != varsSENTINEL; it++) {
    hashkey_vars = (hashkey_vars * 3) + *it;
    if ((*it - *(it - 1)) > max_var_diff)
      max_var_diff = (*it - *(it - 1)) ;
  }

  unsigned hashkey_clauses = *rComp.clsBegin();
  unsigned max_clause_diff = 0;
  if (*rComp.clsBegin()) {
    for (auto jt = rComp.clsBegin() + 1; *jt != clsSENTINEL; jt++) {
      hashkey_clauses = hashkey_clauses * 3 + *jt;
      if (*jt - *(jt - 1) > max_clause_diff)
        max_clause_diff = *jt - *(jt - 1);
    }
  }

  hashkey_ = hashkey_vars + (((unsigned) hashkey_clauses) << 16);

  //VERIFIED the definition of bits_per_var_diff and bits_per_clause_diff
  unsigned bits_per_var_diff = log2(max_var_diff) + 1;
  unsigned bits_per_clause_diff = log2(max_clause_diff) + 1;

  unsigned data_size_vars = 2*bits_per_variable() + 5
                            + (rComp.num_variables() - 1) * bits_per_var_diff ;

  unsigned data_size_clauses = 0;
  if(*rComp.clsBegin())
    data_size_clauses = bits_per_clause() + 5
       + (rComp.numLongClauses() - 1) * bits_per_clause_diff;

  unsigned data_size = 1 + (data_size_vars + data_size_clauses)/bits_per_block();
    data_size+=  ((data_size_vars + data_size_clauses) % bits_per_block())? 1 : 0;

  unsigned *p = data_ = new unsigned[data_size];

  *p = (rComp.num_variables()) | (bits_per_var_diff << bits_per_variable());
  unsigned int bitpos = bits_per_variable() + 5;

  *p |= *rComp.varsBegin() << bitpos;
  bitpos += bits_per_variable();
  if (bitpos >= bits_per_block()) {
    assert(*p);
    bitpos -= bits_per_block();
    *(++p) = (*rComp.varsBegin()) >> (bits_per_variable() - bitpos);
  }
  if(bits_per_var_diff)
  for (auto it = rComp.varsBegin() + 1; *it != varsSENTINEL; it++) {
    *p |= (*it - *(it - 1)) << bitpos;
    bitpos += bits_per_var_diff;
    if (bitpos >= bits_per_block()) {
      assert(*p);
      bitpos -= bits_per_block();
      *(++p) = ((*it - *(it - 1)) >> (bits_per_var_diff - bitpos));
    }
  }

  if (*rComp.clsBegin()) {
    *p |= bits_per_clause_diff << bitpos;
    bitpos += 5;
    if (bitpos >= bits_per_block()) {
      assert(*p);
      bitpos -= bits_per_block();
      *(++p) = (bits_per_clause_diff >> (5 - bitpos));
    }
    *p |= *rComp.clsBegin() << bitpos;
    bitpos += bits_per_clause();
    if (bitpos >= bits_per_block()) {
          assert(*p);
          bitpos -= bits_per_block();
          *(++p) = (*rComp.clsBegin() >> (bits_per_clause() - bitpos));
    }
    for (auto jt = rComp.clsBegin() + 1; *jt != clsSENTINEL; jt++) {
      *p |= ((*jt - *(jt - 1)) << (bitpos));
      bitpos += bits_per_clause_diff;
      if (bitpos >= bits_per_block()) {
        assert(*p);
        bitpos -= bits_per_block();
        *(++p) = ((*jt - *(jt - 1)) >> (bits_per_clause_diff - bitpos));
      }
    }
  }

  if(bitpos > 0)
    p++;
  *p=0;

  // this will tell us if we computed the data_size
  // correctly
//  if(p - data_ + 1 != data_size)
//       cout << " " << (int) ((p - data_) + 1 - (int) data_size) << " " << endl;
//  assert(p - data_  < data_size);
  assert(p - data_ + 1 == data_size);

}

//DifferencePackedComponent::DifferencePackedComponent(Component &rComp) {
//
//  unsigned max_var_diff = 0;
//  unsigned hashkey_vars = *rComp.varsBegin();
//  for (auto it = rComp.varsBegin() + 1; *it != varsSENTINEL; it++) {
//    hashkey_vars = (hashkey_vars * 3) + (*it - *(it - 1));
//    if ((*it - *(it - 1)) - 1 > max_var_diff)
//      max_var_diff = (*it - *(it - 1)) - 1;
//  }
//
//  unsigned hashkey_clauses = *rComp.clsBegin();
//  unsigned max_clause_diff = 0;
//  if (*rComp.clsBegin()) {
//    for (auto jt = rComp.clsBegin() + 1; *jt != clsSENTINEL;
//        jt++) {
//
//      hashkey_clauses = hashkey_clauses * 3 + (*jt - *(jt - 1));
//      if (*jt - *(jt - 1) - 1 > max_clause_diff)
//        max_clause_diff = *jt - *(jt - 1) -1;
//    }
//  }
//
//  hashkey_ = hashkey_vars + (((unsigned) hashkey_clauses) << 16);
//
//  //VERIFIED the definition of bits_per_var_diff and bits_per_clause_diff
//  unsigned bits_per_var_diff = log2(max_var_diff) + 1;
//  unsigned bits_per_clause_diff = log2(max_clause_diff) + 1;
//
//  if(bits_per_var_diff == 0) bits_per_var_diff= 1;
//  if(bits_per_clause_diff == 0) bits_per_clause_diff= 1;
//  unsigned data_size_vars = bits_per_variable() + 5
//                            + (rComp.num_variables() - 1) * bits_per_var_diff ;
//
//
//  unsigned data_size_clauses = 0;
//  if(*rComp.clsBegin())
//    data_size_clauses = bits_per_clause() + 5
//       + (rComp.numLongClauses() - 1) * bits_per_clause_diff;
//
//  unsigned data_size = 1 + data_size_vars/ bits_per_block() + data_size_clauses/  bits_per_block();
//    data_size+=  (data_size_vars % bits_per_block())? 1 : 0;
//    data_size+=  (data_size_clauses % bits_per_block())? 1 : 0;
//
//  unsigned *p = data_ = new unsigned[data_size];
//
//  *p = (*rComp.varsBegin() << 5) | bits_per_var_diff;
//  unsigned int bitpos = bits_per_variable() + 5;
//
//  for (auto it = rComp.varsBegin() + 1; *it != varsSENTINEL; it++) {
//    *p |= (*it - *(it - 1) - 1) << bitpos;
//    bitpos += bits_per_var_diff;
//    if (bitpos >= bits_per_block()) {
//      bitpos -= bits_per_block();
//      *(++p) = ((*it - *(it - 1) - 1) >> (bits_per_var_diff - bitpos));
//    }
//  }
//  if (bitpos > 0)
//       p++;
//
//  clauses_ofs_ = p - data_;
//  bitpos = 0;
//  if (*rComp.clsBegin()) {
//    *p =  (*rComp.clsBegin() << 5) | bits_per_clause_diff;
//    bitpos = bits_per_clause() + 5;
//    for (auto jt = rComp.clsBegin() + 1; *jt != clsSENTINEL;
//        jt++) {
//      *p |= ((*jt - *(jt - 1) - 1) << (bitpos));
//      bitpos += bits_per_clause_diff;
//      if (bitpos >= bits_per_block()) {
//        bitpos -= bits_per_block();
//        *(++p) = ((*jt - *(jt - 1) - 1) >> (bits_per_clause_diff - bitpos));
//      }
//    }
//  }
//  if(bitpos > 0)
//    p++;
//  *p=0;
////  if (bits_per_block() - bitpos < bits_per_clause_diff){
////        p++;
////  *p = 0;
////  }
//  // this will tell us if we computed the data_size
//  // correctly
//  //assert(p - data_ + 1 == data_size);
////  if(p - data_ + 1 != data_size)
////     cout << " " << (int) ((p - data_) + 1 - (int) data_size) << " ";
//}

//DifferencePackedComponent::DifferencePackedComponent(Component &rComp) {
//  unsigned max_var_diff = 0;
//
//  unsigned hashkey_vars = *rComp.varsBegin();
//  for (auto it = rComp.varsBegin() + 1; *it != varsSENTINEL; it++) {
//    hashkey_vars = (hashkey_vars * 3) + (*it - *(it - 1));
//    if ((*it - *(it - 1)) - 1 > max_var_diff)
//      max_var_diff = (*it - *(it - 1)) - 1;
//  }
//
//
//
//  unsigned hashkey_clauses = *rComp.pck_clause_data_.begin();
//  unsigned max_clause_diff = 0;
//  if (*rComp.pck_clause_data_.begin()) {
//    for (auto jt = rComp.pck_clause_data_.begin() + 1; *jt != clsSENTINEL;
//        jt++) {
//
//      hashkey_clauses = hashkey_clauses * 3 + (*jt - *(jt - 1));
//      if (*jt - *(jt - 1) - 1 > max_clause_diff)
//        max_clause_diff = *jt - *(jt - 1) -1;
//    }
//  }
//
//  hashkey_ = hashkey_vars + (((unsigned) hashkey_clauses) << 16);
//
//
//  // TODO note the +2 is due to some error when runnnign
//  //      logictics.c. with -noIBCP
//  //      this needs further investigation
//  unsigned bits_per_var_diff = log2(max_var_diff + 1) + 1;
//  unsigned bits_per_clause_diff = log2(max_clause_diff + 1) + 1;
//
//  assert(bits_per_var_diff < 32);
//  assert(bits_per_clause_diff < 32);
//  //TODO this is a hack, but change this
//  if(bits_per_var_diff >  bits_per_variable())
//    bits_per_var_diff =  bits_per_variable();
//
//  // checkrep
//  assert(bits_per_var_diff <= bits_per_variable());
//  assert(bits_per_clause_diff <= bits_per_clause());
//
//  // end checkrep
//
//  unsigned data_size = (bits_per_variable()
//      + (rComp.num_variables() - 1) * bits_per_var_diff + 5);
//
//  if(*rComp.pck_clause_data_.begin())
//   data_size += (bits_per_clause() + 5
//       + (rComp.pck_clause_data_.size() - 1) * bits_per_clause_diff);
//
//  data_size /= bits_per_block();
//  data_size += 3;
//
//  unsigned *p = data_ = new unsigned[data_size];
//  memset(data_, 0, sizeof(unsigned)*data_size);
//
//  *p = (*rComp.varsBegin() << 5) | bits_per_var_diff;
//  unsigned int bitpos = bits_per_variable() + 5;
//
//  for (auto it = rComp.varsBegin() + 1; *it != varsSENTINEL; it++) {
//    *p |= (*it - *(it - 1) - 1) << bitpos;
//    bitpos += bits_per_var_diff;
//    if (bitpos >= bits_per_block()) {
//      bitpos -= bits_per_block();
//      *(++p) = ((*it - *(it - 1) - 1) >> (bits_per_var_diff - bitpos));
//    }
//  }
//  p++;
////  if(*p != 0)
////    p++;
////     cout << "@";
////  if (bitpos > 0)
////       p++;
//  /// end b
//
//  clauses_ofs_ = p - data_;
//
//  if (*rComp.pck_clause_data_.begin()) {
//    *p = bits_per_clause_diff;
//    bitpos = 5;
//    *p |= *rComp.pck_clause_data_.begin() << bitpos;
//    bitpos += bits_per_clause();
//    for (auto jt = rComp.pck_clause_data_.begin() + 1; *jt != clsSENTINEL;
//        jt++) {
//      *p |= ((*jt - *(jt - 1) - 1) << (bitpos));
//      bitpos += bits_per_clause_diff;
//      if (bitpos >= bits_per_block()) {
//        bitpos -= bits_per_block();
//        *(++p) = ((*jt - *(jt - 1) - 1) >> (bits_per_clause_diff - bitpos));
//      }
//    }
//    if (bitpos > 0)
//      p++;
//  }
//  *p = 0;
//
//  assert(p - data_ < data_size);
//  if(p - data_ < data_size - 1)
//    cout << data_size - (p-data_) << " ";
//}

///
/// This is the supoosed stable version of the constructor
////
//DifferencePackedComponent::DifferencePackedComponent(Component &rComp) {
//  unsigned max_var_diff = 0;
//
//  unsigned hashkey_vars = *rComp.varsBegin();
//  for (auto it = rComp.varsBegin() + 1; *it != varsSENTINEL; it++) {
//    hashkey_vars = (hashkey_vars * 3) + (*it - *(it - 1));
//    if ((*it - *(it - 1)) > max_var_diff)
//      max_var_diff = (*it - *(it - 1));
//  }
//
//  unsigned bits_per_var_diff = log2(max_var_diff + 1) + 2;
//
//  unsigned hashkey_clauses = *rComp.pck_clause_data_.begin();
//  unsigned max_clause_diff = 0;
//  if (*rComp.pck_clause_data_.begin()) {
//    for (auto jt = rComp.pck_clause_data_.begin() + 1; *jt != clsSENTINEL;
//        jt++) {
//
//      hashkey_clauses = hashkey_clauses * 3 + (*jt - *(jt - 1));
//      if (*jt - *(jt - 1) > max_clause_diff)
//        max_clause_diff = *jt - *(jt - 1);
//    }
//  }
//
//  hashkey_ = hashkey_vars + (((unsigned) hashkey_clauses) << 16);
//
//  unsigned bits_per_clause_diff = log2(max_clause_diff + 1) + 1;
//
//  // checkrep
//  assert(bits_per_var_diff <= bits_per_variable());
//  assert(bits_per_var_diff < 32);
//  assert(bits_per_clause_diff <= bits_per_clause());
//  assert(bits_per_clause_diff < 32);
//  // end checkrep
//
//  unsigned data_size = (bits_per_variable()
//      + (rComp.num_variables() - 1) * bits_per_var_diff + 5);
//
//  if(*rComp.pck_clause_data_.begin())
//   data_size += (bits_per_clause() + 5
//       + (rComp.pck_clause_data_.size() - 1) * bits_per_clause_diff);
//
//  data_size /= bits_per_block();
//  data_size += 3;
//
//  unsigned *p = data_ = new unsigned[data_size];
//
//  *p = (*rComp.varsBegin() << 5) | bits_per_var_diff;
//  unsigned int bitpos = bits_per_variable() + 5;
//
//  for (auto it = rComp.varsBegin() + 1; *it != varsSENTINEL; it++) {
//    *p |= (*it - *(it - 1)) << bitpos;
//    bitpos += bits_per_var_diff;
//    if (bitpos >= bits_per_block()) {
//      bitpos -= bits_per_block();
//      *(++p) = ((*it - *(it - 1)) >> (bits_per_var_diff - bitpos));
//    }
//  }
//  /// end b
//
//  clauses_ofs_ = p - data_;
//
//  if (*rComp.pck_clause_data_.begin()) {
//    *p = bits_per_clause_diff;
//    bitpos = 5;
//    *p |= *rComp.pck_clause_data_.begin() << bitpos;
//    bitpos += bits_per_clause();
//    for (auto jt = rComp.pck_clause_data_.begin() + 1; *jt != clsSENTINEL;
//        jt++) {
//      *p |= ((*jt - *(jt - 1)) << (bitpos));
//      bitpos += bits_per_clause_diff;
//      if (bitpos >= bits_per_block()) {
//        bitpos -= bits_per_block();
//        *(++p) = ((*jt - *(jt - 1)) >> (bits_per_clause_diff - bitpos));
//      }
//    }
//    if (bitpos > 0)
//      p++;
//  }
//  *p = 0;
//}



//DifferencePackedComponent::DifferencePackedComponent(
//    ComponentArchetype &archetype, unsigned creation_time) :
//    BasePackedComponent(creation_time) {
//
//  unsigned max_diff = 0;
//  unsigned num_variables = 1;
//  unsigned num_long_clauses = 0;
//
//  auto vfirst_it = archetype.super_comp().varsBegin();
//  while (!archetype.var_seen(*vfirst_it))
//    ++vfirst_it;
//  auto vprev_it = vfirst_it;
//  //assert(*vfirst_it != varsSENTINEL);
//
//  for (auto v_it = vfirst_it + 1; *v_it != varsSENTINEL; v_it++)
//    if (archetype.var_seen(*v_it)) { //we have to put a var into our component
//      num_variables++;
//      if (*v_it - *vprev_it > max_diff)
//        max_diff = *v_it - *vprev_it;
//      vprev_it = v_it;
//    }
//
//  unsigned bits_per_var_diff = (unsigned int) ceil(
//      log((double) max_diff + 1) / log(2.0));
//  if (bits_per_var_diff == 0)
//    bits_per_var_diff = 1;
//  assert(bits_per_var_diff != 0);
//  assert((bits_per_var_diff&31)!= 0);
//
//  max_diff = 0;
//
//  auto cfirst_it = archetype.super_comp().clsBegin();
//  while (!archetype.clause_seen(*cfirst_it) && (*cfirst_it != clsSENTINEL))
//    ++cfirst_it;
//  auto cprev_it = cfirst_it;
//
//  if (*cfirst_it != clsSENTINEL) {
//    num_long_clauses = 1;
//    for (auto it_cl = cfirst_it + 1; *it_cl != clsSENTINEL; it_cl++)
//      if (archetype.clause_seen(*it_cl)) {
//        num_long_clauses++;
//        if (*it_cl - *cprev_it > max_diff)
//          max_diff = *it_cl - *cprev_it;
//        cprev_it = it_cl;
//      }
//  }
//
//  unsigned bits_per_clause_diff = (unsigned int) ceil(
//      log((double) max_diff + 1) / log(2.0));
//
//  unsigned data_size = (bits_per_variable() + 5 + bits_per_clause() + 5
//      + (num_variables - 1) * bits_per_var_diff
//      + (num_long_clauses - 1) * bits_per_clause_diff) / bits_per_block() + 3;
//
///////////////////////////////
//
// // unsigned * p = data_ = (unsigned*) malloc(sizeof(unsigned) * data_size);
//  unsigned * p = data_ = new unsigned[data_size];
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
#endif /* DIFFERENCE_PACKED_COMPONENT_H_ */
