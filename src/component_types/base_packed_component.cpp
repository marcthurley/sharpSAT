/*
 * base_packed_component.cpp
 *
 *  Created on: Feb 4, 2013
 *      Author: mthurley
 */
#include "base_packed_component.h"
#include <math.h>
#include <iostream>

unsigned BasePackedComponent::_bits_per_clause = 0;
unsigned BasePackedComponent::_bits_per_variable = 0; // bitsperentry
unsigned BasePackedComponent::_variable_mask = 0;
unsigned BasePackedComponent::_clause_mask = 0; // bitsperentry
//unsigned BasePackedComponent::_end_clause_mask = 0;
unsigned BasePackedComponent::_debug_static_val=0;
unsigned BasePackedComponent::_bits_of_data_size=0;
unsigned BasePackedComponent::_data_size_mask = 0;


void BasePackedComponent::adjustPackSize(unsigned int maxVarId,
    unsigned int maxClId) {

  _bits_per_variable = log2(maxVarId) + 1;
  _bits_per_clause   = log2(maxClId) + 1;

  _bits_of_data_size = log2(maxVarId + maxClId) + 1;

  _variable_mask = _clause_mask = _data_size_mask = 0;
  for (unsigned int i = 0; i < _bits_per_variable; i++)
    _variable_mask = (_variable_mask << 1) + 1;
  for (unsigned int i = 0; i < _bits_per_clause; i++)
    _clause_mask = (_clause_mask << 1) + 1;
  for (unsigned int i = 0; i < _bits_of_data_size; i++)
    _data_size_mask = (_data_size_mask << 1) + 1;

 // _end_clause_mask = _clause_mask << (sizeof(unsigned)*8 - _bits_per_clause);

//  cout << "_bits_per_variable,_bits_per_clause  " <<_bits_per_variable <<","<<_bits_per_clause << endl;
//  cout << "_end_clause_mask ";
//  outbit(_end_clause_mask);
//
//  _end_clause_mask |= _clause_mask << (sizeof(unsigned)*8 - 2*_bits_per_clause);
//  cout << "\n_end_clause_mask ";
//  outbit(_end_clause_mask);
//
//
//  cout << endl;
//  cout << "_clause_mask     ";
//  outbit(_clause_mask);
//  cout << endl;
//  cout << "_variable_mask   ";
//    outbit(_variable_mask);
//    cout << endl;
}




