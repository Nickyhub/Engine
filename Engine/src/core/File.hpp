#pragma once
#include <stdio.h>

enum FileMode {
	FILE_MODE_READ = 0x01,
	FILE_MODE_WRITE = 0x02,
};

class File {
public:
	File() { m_Handle = 0; }
	bool Open(const char* path, FileMode mode, bool isBinary);
	void Close();

	unsigned int Size() { return m_Size; }

	bool ReadAllBytes(char* buffer);

	static bool Exists(const char* path);
private:
	bool m_isOpen = false;
	unsigned int m_Size = 0;
	const char* m_Path = "";
	FILE* m_Handle;
};