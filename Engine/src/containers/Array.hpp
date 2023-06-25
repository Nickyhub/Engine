#pragma once

template<typename T, size_t size>
class Array {
public:
	Array() { }
	~Array() {}

	Array(T value) {
		for (int i = 0; i < this->Size(); i++) {
			m_Data[i] = value;
		}
	}

	void fill(T value) {
		for (int i = 0; i < this->Size(); i++) {
			m_Data[i] = value;
		}
	}

	bool Contains(T elem) {
		for (unsigned int i = 0; i < size; i++) {
			if (elem == m_Data) {
				return true;
			}
		}
	}

	constexpr size_t Size() const { return size; }

	T& operator[](size_t index) {
		if (index >= 0 && index < size) {
			return m_Data[index];
		}
		return m_Data[0];
	}

	T* Data() { return m_Data; };

private:
	T m_Data[size] {};
};