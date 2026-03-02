#ifndef NUMA_H
#define NUMA_H

#include <numa.h>
#include <numaif.h>
#include <cstring> // for memcpy
#include <iostream>
#include <mutex>

void *migrate_to_node(void *src, size_t size, int dst_node);

void bind_to_core(int core_id);

#endif