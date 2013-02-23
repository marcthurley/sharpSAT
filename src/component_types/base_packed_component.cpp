/*
 * base_packed_component.cpp
 *
 *  Created on: Feb 4, 2013
 *      Author: mthurley
 */
#include "base_packed_component.h"
#include <math.h>


unsigned BasePackedComponent::_bits_per_clause = 0;
unsigned BasePackedComponent::_bits_per_variable = 0; // bitsperentry
unsigned BasePackedComponent::_variable_mask = 0;
unsigned BasePackedComponent::_clause_mask = 0; // bitsperentry
unsigned BasePackedComponent::_end_clause_mask = 0;
unsigned BasePackedComponent::_debug_static_val=0;


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

       if ((tt = (v >> 16)))
       {
         r = (t = (tt >> 8)) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
       }
       else
       {
         r = (t = (v >> 8)) ? 8 + LogTable256[t] : LogTable256[v];
       }
       return r;
     }


void BasePackedComponent::adjustPackSize(unsigned int maxVarId,
    unsigned int maxClId) {

  _bits_per_variable = log2(maxVarId) + 1;
  _bits_per_clause   = log2(maxClId) + 1;

  _variable_mask = _clause_mask = 0;
  for (unsigned int i = 0; i < _bits_per_variable; i++)
    _variable_mask = (_variable_mask << 1) + 1;
  for (unsigned int i = 0; i < _bits_per_clause; i++)
    _clause_mask = (_clause_mask << 1) + 1;

  _end_clause_mask = _clause_mask << (sizeof(unsigned)*8 - _bits_per_clause);
}




