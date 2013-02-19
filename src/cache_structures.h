/*
 * cache_structures.h
 *
 *  Created on: Aug 23, 2012
 *      Author: Marc Thurley
 */

#ifndef CACHE_STRUCTURES_H_
#define CACHE_STRUCTURES_H_

#include <assert.h>
#include <vector>

#include "primitive_types.h"

#include "component_types/difference_packed_component.h"
//#include "component_types/simple_packed_component.h"
//#include "component_types/simple_unpacked_component.h"


using namespace std;

#define NIL_ENTRY 0

class Component;
class ComponentArchetype;


// GenericCachedComponent Adds Structure to PackedComponent that is
// necessary to store it in the cache
// namely, the descendant tree structure that
// allows for the removal of cache pollutions

template< class T_Component>
class GenericCachedComponent: public T_Component {
public:
  GenericCachedComponent() {
  }

  GenericCachedComponent(Component &comp) :
      T_Component(comp) {
  }

  unsigned long SizeInBytes() const {
    return sizeof(GenericCachedComponent<T_Component>)
        + T_Component::data_size() * sizeof(unsigned)
        + T_Component::size_of_model_count();
  }

  // BEGIN Cache Pollution Management

  void set_father(CacheEntryID f) {
    father_ = f;
  }
  const CacheEntryID father() const {
    return father_;
  }

  void set_next_sibling(CacheEntryID sibling) {
    next_sibling_ = sibling;
  }
  CacheEntryID next_sibling() {
    return next_sibling_;
  }

  void set_first_descendant(CacheEntryID descendant) {
    first_descendant_ = descendant;
  }
  CacheEntryID first_descendant() {
    return first_descendant_;
  }



  void set_next_bucket_element(CacheEntryID entry) {
    next_bucket_element_ = entry;
  }

  CacheEntryID next_bucket_element() {
      return next_bucket_element_;
  }

private:


  //
  CacheEntryID next_bucket_element_ = 0;

  // theFather and theDescendants:
  // each CCacheEntry is a Node in a tree which represents the relationship
  // of the components stored
  CacheEntryID father_ = 0;
  CacheEntryID first_descendant_ = 0;
  CacheEntryID next_sibling_ = 0;

};


typedef GenericCachedComponent<DifferencePackedComponent> CachedComponent;
//typedef GenericCachedComponent<SimplePackedComponent> CachedComponent;
//typedef GenericCachedComponent<SimpleUnpackedComponent> CachedComponent;

#endif /* CACHE_STRUCTURES */
