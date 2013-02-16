/*
 * component_archetype.cpp
 *
 *  Created on: Feb 9, 2013
 *      Author: mthurley
 */

#include "component_archetype.h"

//
//CA_SearchState *ComponentArchetype::variables_seen_ = nullptr;
//unsigned ComponentArchetype::variables_seen_byte_size_;
//CA_SearchState *ComponentArchetype::clauses_seen_;
//unsigned ComponentArchetype::clauses_seen_byte_size_;

CA_SearchState *ComponentArchetype::seen_ = nullptr;
unsigned ComponentArchetype::seen_byte_size_ = 0;
