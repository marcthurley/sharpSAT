/*
 * component_types-inl.h
 *
 *  Created on: Aug 23, 2012
 *      Author: Marc Thurley
 */

#ifndef COMPONENT_TYPES_INL_H_
#define COMPONENT_TYPES_INL_H_

bool PackedComponent::equals(const PackedComponent &comp) const {
  if (clauses_ofs_ != comp.clauses_ofs_)
    return false;
  unsigned* p = data_;
  unsigned* r = comp.data_;
  while (*p && *p == *r) {
    p++;
    r++;
  }
  return *p == *r;
}

//PackedComponent::PackedComponent(Component &rComp) {
//  unsigned data_size = sizeof(ComponentDataType)*((rComp.num_variables() * _bits_per_variable
//      + rComp.numLongClauses() * _bits_per_clause)/_bitsPerBlock + 3);
//
//  ComponentDataType *p = data_ = (ComponentDataType *) malloc(data_size);
//
//  *p = *rComp.varsBegin();
//  unsigned hashkey_vars = *p;
//  unsigned int bitpos = _bits_per_variable;
//  for (auto it = rComp.varsBegin()+1; *it != varsSENTINEL; it++) {
//    *p |= ((*it) << bitpos);
//    bitpos += _bits_per_variable;
//    hashkey_vars = (hashkey_vars *3) + (*it - *(it-1));
//    if (bitpos >= _bitsPerBlock) {
//      bitpos -= _bitsPerBlock;
//      *(++p) = ((*it) >> (_bits_per_variable - bitpos));
//    }
//  }
//  if (bitpos > 0)
//    p++;
//
//  clauses_ofs_ = p - data_;
//
//  unsigned hashkey_clauses = *p = *rComp.clsBegin();
//
//  if (*rComp.clsBegin()) {
//    bitpos = _bits_per_clause;
//    for (auto jt = rComp.clsBegin()+1; *jt != clsSENTINEL; jt++) {
//      *p |= ((*jt) << (bitpos));
//      bitpos += _bits_per_clause;
//      hashkey_clauses = (hashkey_clauses *3) + (*jt - *(jt-1));
//      if (bitpos >= _bitsPerBlock) {
//        bitpos -= _bitsPerBlock;
//        *(++p) = ((*jt) >> (_bits_per_clause - bitpos));
//      }
//    }
//    if (bitpos > 0)
//      p++;
//  }
//  *p = 0;
//  hashkey_ = hashkey_vars + (((unsigned) hashkey_clauses) << 16);
//}

PackedComponent::PackedComponent(Component &rComp) {
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


  unsigned data_size = (_bits_per_variable + 5 + _bits_per_clause + 5
      + (rComp.num_variables() - 1) * bits_per_var_diff
      + (rComp.numLongClauses() - 1) * bits_per_clause_diff) / _bitsPerBlock
      + 3;

  unsigned * p = data_ = (unsigned*) malloc(sizeof(unsigned) * data_size);

  *p = bits_per_var_diff;
  unsigned int bitpos = 5;

  *p |= *rComp.varsBegin() << bitpos;
  bitpos += _bits_per_variable;
  unsigned hashkey_vars = *rComp.varsBegin();

  for (auto it = rComp.varsBegin() + 1; *it != varsSENTINEL; it++) {
    *p |= ((*it) - *(it - 1)) << bitpos;
    bitpos += bits_per_var_diff;
    hashkey_vars = hashkey_vars * 3 + ((*it) - *(it - 1));
    if (bitpos >= _bitsPerBlock) {
      bitpos -= _bitsPerBlock;
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
    bitpos += _bits_per_clause;
    for (auto jt = rComp.clsBegin() + 1; *jt != clsSENTINEL; jt++) {
      *p |= ((*jt - *(jt - 1)) << (bitpos));
      bitpos += bits_per_clause_diff;
      hashkey_clauses = hashkey_clauses * 3 + (*jt - *(jt - 1));
      if (bitpos >= _bitsPerBlock) {
        bitpos -= _bitsPerBlock;
        *(++p) = ((*jt - *(jt - 1)) >> (bits_per_clause_diff - bitpos));
      }
    }
    if (bitpos > 0)
      p++;
  }
  *p = 0;
  hashkey_ = hashkey_vars + (((unsigned long) hashkey_clauses) << 16);
}

#endif /* COMPONENT_TYPES_INL_H_ */
