/*
 * component_types.cpp
 *
 *  Created on: Aug 23, 2012
 *      Author: Marc Thurley
 */

#include "component_types.h"

unsigned PackedComponent::_bits_per_clause = 0;
unsigned PackedComponent::_bits_per_variable = 0; // bitsperentry
unsigned PackedComponent::_variable_mask = 0;
unsigned PackedComponent::_clause_mask = 0; // bitsperentry

void PackedComponent::adjustPackSize(unsigned int maxVarId,
    unsigned int maxClId) {
  _bits_per_variable = (unsigned int) ceil(
      log((double) maxVarId + 1) / log(2.0));
  _bits_per_clause = (unsigned int) ceil(log((double) maxClId + 1) / log(2.0));

  _variable_mask = _clause_mask = 0;
  for (unsigned int i = 0; i < _bits_per_variable; i++)
    _variable_mask = (_variable_mask << 1) + 1;
  for (unsigned int i = 0; i < _bits_per_clause; i++)
    _clause_mask = (_clause_mask << 1) + 1;
}
