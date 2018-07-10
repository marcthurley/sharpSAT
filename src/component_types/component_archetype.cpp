/*
 * component_archetype.cpp
 *
 *  Created on: Feb 9, 2013
 *      Author: mthurley
 */

#include <sharpSAT/component_types/component_archetype.h>

CA_SearchState *ComponentArchetype::seen_ = nullptr;
unsigned ComponentArchetype::seen_byte_size_ = 0;
