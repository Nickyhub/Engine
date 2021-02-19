// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifdef _DEBUG
	#define ASSERT(x, ...) if(!x) {EN_ERROR(__VA_ARGS__); __debugbreak();}
#else
	#define ASSERT(x, ...)
#endif