#ifndef __MEMORY__
#define __MEMORY__
#include <string_view>
#include <memory>

using Pointer = char*;

struct ModuleInfo
{
	void *moduleBase;
	std::uint32_t moduleSize;
};

ModuleInfo getModuleInfo(std::wstring_view mModuleName);
Pointer patternScanHeap(std::string_view unformattedPattern);
Pointer patternScan(std::string_view unformattedPattern);
Pointer patternScan(std::string_view unformattedPattern, std::wstring_view mModuleName);

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
	if (baseAddress) {
		baseAddress += offset;
		return baseAddress;
	}
	else
		return nullptr;
}

template<typename T, typename ...Args>
Pointer pointerPath(Pointer baseAddress, const T &offset, const Args& ...offsets)
{
	memcpy(&baseAddress, baseAddress, sizeof(Pointer));
	if (baseAddress) {
		baseAddress += offset;
		return pointerPath(baseAddress, offsets...);
	}
	else
		return nullptr;
}

Pointer getPointerFromImmediate(Pointer codeAddress);

class PatternScanner
{
	using IdType = int;
	class Impl;
	std::unique_ptr<Impl> mThis;
public:
	PatternScanner(std::wstring_view moduleName = L"");
	~PatternScanner();
	void addPattern(IdType id, std::string_view pattern);
	Pointer getScanResult(IdType id);
	void scan();
	void waitForScan();
};

#endif