#include <stdlib.h>

#include "Memory.hpp"
#include "Logger.hpp"

const char* Memory::m_MemoryTagStrings[MEMORY_TAG_MAX] = {
	 "MEMORY_TAG_ARRAY",
	 "MEMORY_TAG_std::vector",
	 "MEMORY_TAG_TEXTURE",
	 "MEMORY_TAG_MESH",
	 "MEMORY_TAG_RENDERER"
	};

unsigned long Memory::m_Capacity;
AllocationStats Memory::m_Stats;

// TODO implement dynamic allocator
void* Memory::Allocate(unsigned int size, MemoryTag tag) {
	m_Stats.s_TotalAllocations++;
	m_Stats.s_MemoryAllocationStats[tag] += size;
	//EN_DEBUG("Allocate size: %d, TotalAllocations: %d.", size, m_Stats.s_TotalAllocations);
	return malloc(size);
}

void Memory::Free(void* block, unsigned int size, MemoryTag tag) {
	m_Stats.s_TotalAllocations--;
	m_Stats.s_MemoryAllocationStats[tag] -= size;
	//EN_DEBUG("Free size: %d, TotalAllocations: %d.", size, m_Stats.s_TotalAllocations);
	free(block);
}

void Memory::ZeroOut(void* block, int size) {
	memset(block, 0, size);
}

void Memory::Set(void* block, int value, int size) {
	memset(block, value, size);
}

void Memory::PrintMemoryStats() {
	for (int i = 0; i < MEMORY_TAG_MAX; i++) {
		EN_INFO("%s: %u", m_MemoryTagStrings[i], m_Stats.s_MemoryAllocationStats[i]);
	}
	EN_INFO("Total allocations: %d.", m_Stats.s_TotalAllocations);
}

void* Memory::Copy(void* dest, const void* src, unsigned int size) {
	return memcpy(dest, src, (size_t)size);
}

unsigned int Memory::GetTotalAllocations() {
	return m_Stats.s_TotalAllocations;
}