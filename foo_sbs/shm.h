/*
 * (c) 2011 Victor Su
 *
 * This program is open source. For license terms, see the LICENSE file.
 *
 */
#ifndef _SHM_H_
#define _SHM_H_

#define FOO_SBS_SHM_SEGMENT  "FooSbsSharedMemory"
#define FOO_SBS_SHM_MAP      "FooSbsSharedMap"
#define FOO_SBS_MUTEX        "FooSbsMutex"

#include <string>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

using namespace boost::interprocess;

// Note that map<Key, MappedType>'s value_type is std::pair<const Key, MappedType>,
// so the allocator must allocate that pair.
typedef std::string KeyType;
typedef std::string MappedType;
typedef std::pair<std::string, std::string> ValueType;

// Alias an STL compatible allocator of for the map.
// This allocator will allow to place containers
// in managed shared memory segments
typedef allocator<ValueType, managed_shared_memory::segment_manager> ShMemAllocator;

// Alias a map of ints that uses the previous STL-like allocator.
// Note that the third parameter argument is the ordering function
// of the map, just like with std::map, used to compare the keys.
typedef map<KeyType, MappedType, std::less<KeyType>, ShMemAllocator> ShMemMap;

#endif