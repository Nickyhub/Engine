#include "File.hpp"
#include "Logger.hpp"
#include <sys/stat.h>

bool File::Open(const char* path, FileMode mode, bool isBinary)
{
	const char* modeString;
	if (this->Exists(path)) {
		m_Path = path;
		if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) != 0) {
			modeString = isBinary ? "w+b" : "w+";
		}
		else if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) == 0) {
			modeString = isBinary ? "rb" : "r";
		}
		else if ((mode & FILE_MODE_READ) == 0 && (mode & FILE_MODE_WRITE) != 0) {
			modeString = isBinary ? "wb" : "w";
		}
		else {
			EN_ERROR("Invalid mode passed while trying to open file: '%s'", path);
			return false;
		}

		// Try to open file
		m_Handle = fopen(m_Path, modeString);
		if (!m_Handle) {
			EN_ERROR("Error opening file: '%s'.", path);
			return false;
		}

		// If file could be opened, determine size of the file.
		rewind(m_Handle);
		fseek(m_Handle, 0, SEEK_END);
		m_Size = ftell(m_Handle);
		rewind(m_Handle);

		m_isOpen = true;
		return true;
	}
	else {
		EN_ERROR("Tried to open non existent file: %s", path);
		m_isOpen = false;
		return false;
	}
}

bool File::ReadAllBytes(char* buffer) {
	if (m_isOpen) {
		fread(buffer, 1, m_Size, m_Handle);

		return true;
	}
	else {
		EN_ERROR("Tried to read from file that has not been opened. Open file first: %s.", m_Path);
		return false;
	}
}

void File::Close()
{
	if (m_Handle) {
		fclose(m_Handle);
		m_Handle = 0;
	}
}

bool File::Exists(const char* path)
{
#ifdef _MSC_VER
	struct _stat buffer;
	return _stat(path, &buffer) == 0;
#else
	struct stat buffer;
	return stat(path, &buffer) == 0;
#endif
}

