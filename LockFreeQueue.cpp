#include "LockFreeQueue.h"
#include "SearchData.h"

template <>
std::atomic<long long> LockFreeQueue<SearchData>::total_times{0};