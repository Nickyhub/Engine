#pragma once
#include <iostream>

#include <vector>
#include "core/Memory.hpp"
#include "core/Logger.hpp"

// When calling vkEnumerate... with DArrays the m_Size does not update
// TODO fix this

template <typename T>
class DArray {
public:
	DArray() {
	//	/*m_Data = (T*)Memory::Allocate(2 * sizeof(T), MEMORY_TAG_DARRAY);
	//	Memory::ZeroOut(m_Data, 2 * sizeof(T));
	//	m_Capacity = 2;
	//	m_Size = 0;*/
	}

	~DArray() {
		//EN_DEBUG("Freed typename: %s", typeid(this).name());
		//Memory::Free(m_Data, m_Capacity * sizeof(T), MEMORY_TAG_DARRAY);
	}

	void Resize(unsigned int size) {
		/*if (size > m_Capacity) {
			Realloc(size);
		}
		return;*/
		internalVector.resize(size);
	}

	// TODO implement get minimum for number types

	void Fill(T value) {
		/*for (unsigned int i = 0; i < m_Size; i++) {
			m_Data[i] = value;
		}*/
		for (unsigned int i = 0; i < internalVector.size(); i++) {
			internalVector[i] = value;
		}
	}

	void PushBack(const T& elem) {
	/*	if (m_Size >= m_Capacity) {
			Realloc(m_Capacity * 2);
		}

		m_Data[m_Size] = elem;
		m_Size++;*/
		internalVector.push_back(elem);
	}

	void PopBack() {
		/*if (m_Size > 0) {
			m_Size--;
			m_Data[m_Size].~T();
		}*/
		internalVector.pop_back();
	}

	void Clear() {
	/*	for (int i = 0; i < m_Size; i++) {
			m_Data[m_Data].~T();
		}*/
		internalVector.clear();
	}

	bool Contains(T elem) {
	/*	for (unsigned int i = 0; i < m_Size; i++) {
			if (elem == m_Data[i]) {
				return true;
			}
		}
		return false;*/
		for (unsigned int i = 0; i < internalVector.size(); i++) {
			if (elem == internalVector[i]) {
				return true;
			}
		}
		return false;
	}

	const T& operator[](unsigned int index) const {
		// TODO handle invalid index
		/*if (index > m_Size) {
			EN_ERROR("Tried to access DArray with invalid index.");
			return 0;
		}*/
		return internalVector[index];
	}

	T& operator[](unsigned int index) {
		// TODO handle invalid index
		/*if (index > m_Size) {
			EN_ERROR("Tried to access DArray with invalid index.");
		}*/
		return internalVector[index];
	}

	unsigned int Size() const { return (unsigned int) internalVector.size(); }
	//unsigned int Capacity() const { return m_Capacity; }
	T* GetData() { return internalVector.data(); }

private:
	//void Realloc(unsigned int newCapacity) {
	//	T* newBlock = (T*)Memory::Allocate(newCapacity * sizeof(T), MEMORY_TAG_DARRAY);
	//	Memory::Copy(newBlock, m_Data, m_Size * sizeof(T));


	//	Memory::Free(m_Data, m_Capacity * sizeof(T), MEMORY_TAG_DARRAY);
	//	m_Data = newBlock;
	//	m_Capacity = newCapacity;
	//}

	std::vector<T> internalVector;

	//unsigned int m_Size;
	//unsigned int m_Capacity;
	//T* m_Data;
};