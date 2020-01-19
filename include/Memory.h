#ifndef __MEMORY__
#define __MEMORY__
#include <string_view>
#include <vector>

using Pointer = char*;

struct ModuleInfo
{
	void *moduleBase;
	std::uint32_t moduleSize;
};

ModuleInfo getModuleInfo(std::wstring_view moduleName);
Pointer patternScanHeap(std::string_view unformattedPattern);
Pointer patternScan(std::string_view unformattedPattern);
Pointer patternScan(std::string_view unformattedPattern, std::wstring_view moduleName);

template<typename T>
T getValue(Pointer address)
{
	return *(T*)(address);
}

template<typename T>
void setValue(Pointer address, T value)
{
	*(T*)(address) = value;
}

template<typename T, size_t sz>
void setValue(Pointer address, const T(&value)[sz])
{
	for (size_t i = 0; i < sz; ++i) {
		((T*)address)[i] = value[i];
	}
}

template<typename T>
Pointer pointerPath(Pointer baseAddress, const T &offset)
{
	memcpy(&baseAddress, baseAddress, sizeof(Pointer));
	baseAddress += offset;
	return baseAddress;
}

template<typename T, typename ...Args>
Pointer pointerPath(Pointer baseAddress, const T &offset, const Args& ...offsets)
{
	memcpy(&baseAddress, baseAddress, sizeof(Pointer));
	baseAddress += offset;
	return pointerPath(baseAddress, offsets...);
}

Pointer pointerPath(Pointer baseAddress, const std::vector<std::uint64_t> &offsets);
Pointer getPointerFromImmediate(Pointer codeAddress);

#endif