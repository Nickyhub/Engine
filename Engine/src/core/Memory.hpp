#pragma once
#include <memory.h>

enum MemoryTag {
	MEMORY_TAG_ARRAY,
	MEMORY_TAG_DARRAY,
	MEMORY_TAG_TEXTURE,
	MEMORY_TAG_MESH,
	MEMORY_TAG_RENDERER,
	MEMORY_TAG_MAX
};

struct AllocationStats {
	unsigned int s_MemoryAllocationStats[MEMORY_TAG_MAX];
	unsigned int s_TotalAllocations = 0;
};


class Memory {
public:
	static void* Allocate(unsigned int size, MemoryTag tag);
	static void Free(void* block, unsigned int size, MemoryTag tag);
	static void* Copy(void* dest, const void* src, unsigned int size);
	static void ZeroOut(void* block, int size);
	static void Set(void* block, int value, int size);
	static unsigned int GetTotalAllocations();
	static void PrintMemoryStats();
private:
	static const char* m_MemoryTagStrings[MEMORY_TAG_MAX];
	static AllocationStats m_Stats;
	static unsigned long m_Capacity;
};