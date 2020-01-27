#include "Features.h"
#include "Memory.h"
#include <windows.h>

#ifndef NDEBUG
#include <iostream>
using std::cout;
using std::cin;
using std::endl;
using std::hex;
using std::dec;
#endif

namespace
{
	constexpr wchar_t processName[] = L"re2.exe";

	Pointer getB10(Pointer bb0Base)
	{
		return pointerPath(bb0Base, 0x130, 0);
	}

	//pattern must match at the beginning of a 0xE8 call instruction
	template<typename FunctionType>
	FunctionType getFunctionPointer(std::string_view pattern, std::wstring_view moduleName)
	{
		if (Pointer result = patternScan(pattern, moduleName)) {
			return reinterpret_cast<FunctionType>(getPointerFromImmediate(result + 1));
		}

		return nullptr;
	}
}

struct Inventory::TextHash
{
	std::int64_t firstPart, secondPart;
};

Inventory::Inventory()
	:mInventorySizeBase(patternScan("48 8B 15 ????????  45 33 C0  E8 ????????  0FB6 ??  48 8B 43 50  4C 39 70 18", processName)),
	getWeaponTextHash(getFunctionPointer<decltype(getWeaponTextHash)>("E8 ????????  48 8B 43 50  48 39 78 18  75 ??  8B 05 ????????  0FB7 0D", processName)),
	getItemTextHash(reinterpret_cast<decltype(getItemTextHash)>(patternScan("48 89 5C 24 10  48 89 74 24 18  48 89 7C 24 20  55  41 56  41 57  48 8B EC  48 83 EC 60  44 0FB7 15 ????????", processName))),
	getName(getFunctionPointer<decltype(getName)>("E8 ????????  33 FF  48 85 C0  74 ??  48 C7 C1 FFFFFFFF  0F1F 44 00 00", processName)),
	mGetNameFirstParameter(patternScan("48 8B 0D ????????  48 8D 54 24 30  89 7C 24 30  66 89 74 24 34  66 89 6C 24 36  4C 89 74 24 38  E8 ????????  33 FF", processName)),
	mBB0Base(patternScan("48 8B 15 ????????  48 85 C0  74 ??  44 8B 40 14  EB ??  41 B8 FFFFFFFF  48 8B CB  48 85 D2  75 ??  45 33 C0  41 8D 50 38  E8 ????????  33 C0  48 8B 5C 24 30  48 83 C4 20", processName)),
	mUnnamedArgumentPointer(patternScan("48 8B 15 ????????  48 83 78 18 00  75 ??  48 85 D2  75 19  45 33 C0  41 8D 50 38  E8 ????????   32 C0  48 8B 5C 24 30  48 83 C4 20  5F  C3  48 8B 82", processName)), //multiple matches, but they all access the same variable
	getArgument(getFunctionPointer<decltype(getArgument)>("E8 ????????  48 8B 4B 50  48 83 79 18 00  0F85 ????????  48 8B CB  48 85 C0  75 ??  45 33 C0  41 8D 50 38  48 8B 5C 24 30  48 8B 74 24 38  48 83 C4 20  5F  E9 ????????  48 8B D0  E8 ????????  48 8B 4B 50  48 8B F8  48 83 79 18 00  0F85 ????????  45 33 C0  48 8B D0", processName)),
	getArgumentForGetItemAt(getFunctionPointer<decltype(getArgumentForGetItemAt)>("E8 ????????  48 8B 4B 50  48 8B F8  48 83 79 18 00  0F85 ????????  45 33 C0  48 8B D0  48 8B CB  E8 ????????  48 8B 4B 50  0FB6 C0  48 8B 51 18  48 85 D2  74 04  32 C0  EB 05  85 C0  0F95 C0  48 85 D2  75 78  84 C0  74 74  83 BE 74050000 01  74 6B  48 8B CB  48 85 FF  74 8A  48 8B D7  E8 ????????  0FB6 C8  48 8B 43 50  48 83 78 18 00  75 4D  85 C9  74 49  41 B0 01  48 8B D6  48 8B CB  E8 ????????  48 8B 43 50  48 83 78 18 00  75 30  48 8B 05 ????????  48 85 C0  75 1D  45 33 C0  8D 50 38  48 8B CB  48 8B 5C 24 30  48 8B 74 24 38  48 83 C4 20  5F  E9 ????????  C7 40 68 06000000  48 8B 5C 24 30  48 8B 74 24 38  48 83 C4 20  5F  C3", processName)),
	getItemAtSlot(reinterpret_cast<decltype(getItemAtSlot)>(patternScan("48 89 5C 24 18  48 89 7C 24 20  41 56  48 83 EC 20  48 8B 41 50  45 0FB6 F1  41 8B F8  48 8B D9  48 83 78 18 00  74 13  33 C0  48 8B 5C 24 40  48 8B 7C 24 48  48 83 C4 20  41 5E  C3", processName))),
	mF0C0ArgumentBase(patternScan("48 8B 0D ????????  E8 ????????  48 8B D8  83 78 78 00  75 ??  48 8B C8  E8 ????????  FF 43 78  48 8B 05  ????????", processName)),
	mGetF0C0Ptr(reinterpret_cast<decltype(mGetF0C0Ptr)>(mF0C0ArgumentBase ? getPointerFromImmediate(mF0C0ArgumentBase + 0x7 + 0x1) : nullptr)),
	mWeaponInfoTableBase(patternScan("4C 8B 05 ????????  48 85 C9  74 ?? 48 8B 41 10", processName)),
	mCapacityCheckOpcode(patternScan("0F4C D8  48 85 F6", processName))
	//mFreeF0C0Ptr(reinterpret_cast<decltype(mFreeF0C0Ptr)>(patternScan("40 53  48 83 EC 20  48 8B 41 50  48 8B D9  48 83 78 08 00", processName)))
{
	if (!(mInventorySizeBase
		  && getWeaponTextHash
		  && getItemTextHash
		  && getName
		  && mGetNameFirstParameter
		  && mBB0Base
		  && mUnnamedArgumentPointer
		  && getArgument
		  && getArgumentForGetItemAt
		  && getItemAtSlot
		  && mF0C0ArgumentBase
		  && mGetF0C0Ptr
		  && mWeaponInfoTableBase
		  && mCapacityCheckOpcode))
		throw std::runtime_error("Could not find one or more addresses");
	//Pointer getWeaponTextHashPtr = patternScan("E8 ????????  48 8B 43 50  48 39 78 18  75 ??  8B 05 ????????  0FB7 0D", processName);
	//Pointer getNamePtr = patternScan("E8 ????????  33 FF  48 85 C0  74 ??  48 C7 C1 FFFFFFFF  0F1F 44 00 00", processName);
	//Pointer getArgumentPtr = patternScan("E8 ????????  48 8B 4B 50  48 83 79 18 00  0F85 ????????  48 8B CB  48 85 C0  75 ??  45 33 C0  41 8D 50 38  48 8B 5C 24 30  48 8B 74 24 38  48 83 C4 20  5F  E9 ????????  48 8B D0  E8 ????????  48 8B 4B 50  48 8B F8  48 83 79 18 00  0F85 ????????  45 33 C0  48 8B D0", processName); //multiple matches, but they all access the same variable
	//Pointer getArgumentForGetItemAtPtr = patternScan("E8 ????????  48 8B 4B 50  48 8B F8  48 83 79 18 00  0F85 ????????  45 33 C0  48 8B D0  48 8B CB  E8 ????????  48 8B 4B 50  0FB6 C0  48 8B 51 18  48 85 D2  74 04  32 C0  EB 05  85 C0  0F95 C0  48 85 D2  75 78  84 C0  74 74  83 BE 74050000 01  74 6B  48 8B CB  48 85 FF  74 8A  48 8B D7  E8 ????????  0FB6 C8  48 8B 43 50  48 83 78 18 00  75 4D  85 C9  74 49  41 B0 01  48 8B D6  48 8B CB  E8 ????????  48 8B 43 50  48 83 78 18 00  75 30  48 8B 05 ????????  48 85 C0  75 1D  45 33 C0  8D 50 38  48 8B CB  48 8B 5C 24 30  48 8B 74 24 38  48 83 C4 20  5F  E9 ????????  C7 40 68 06000000  48 8B 5C 24 30  48 8B 74 24 38  48 83 C4 20  5F  C3", processName);

	//getArgumentForGetItemAt = reinterpret_cast<decltype(getArgumentForGetItemAt)>(getPointerFromImmediate(getArgumentForGetItemAtPtr + 1));
	mInventorySizeBase = getPointerFromImmediate(mInventorySizeBase + 3);
	mBB0Base = getPointerFromImmediate(mBB0Base + 3);
	//getWeaponTextHash = (reinterpret_cast<decltype(getWeaponTextHash)>(getPointerFromImmediate(getWeaponTextHashPtr + 1)));
	//getName = (reinterpret_cast<decltype(getName)>(getPointerFromImmediate(getNamePtr + 1)));
	mGetNameFirstParameter = getPointerFromImmediate(mGetNameFirstParameter + 3);
	mUnnamedArgumentPointer = getPointerFromImmediate(mUnnamedArgumentPointer + 3);
	//getArgument = reinterpret_cast<decltype(getArgument)>(getPointerFromImmediate(getArgumentPtr + 1));
	//mGetF0C0Ptr = reinterpret_cast<decltype(mGetF0C0Ptr)>(getPointerFromImmediate(mF0C0ArgumentBase + 0x8));
	mF0C0ArgumentBase = getPointerFromImmediate(mF0C0ArgumentBase + 0x3);
	mWeaponInfoTableBase = getPointerFromImmediate(mWeaponInfoTableBase + 0x3);

	#ifndef NDEBUG
	cout << (void*)mInventorySizeBase << " -> mInventoryBase" << endl;
	cout << (void*)mGetNameFirstParameter << " -> mGetNameFirstParameter" << endl;
	cout << (void*)mBB0Base << " -> BB0 Base" << endl;
	cout << getWeaponTextHash << " -> getWeaponTextHash entry point" << endl;
	cout << getName << " -> getName entry point" << endl;
	#endif
}

std::wstring_view Inventory::getWeaponName(WeaponId id)
{
	TextHash hash;
	std::wstring_view result;
	auto f0c0 = getF0c0();

	getWeaponTextHash(f0c0, getB10(mBB0Base), id, hash);
	if(auto namePtr = getName(getValue<Pointer>(mGetNameFirstParameter), hash))
		result = namePtr;

	//mFreeF0C0Ptr(f0c0);
	return result;
}

std::wstring_view Inventory::getItemName(ItemId id)
{
	TextHash hash;
	std::wstring_view result;
	auto f0c0 = getF0c0();

	TextHash &hash2 = getItemTextHash(hash, f0c0, getValue<Pointer>(mBB0Base), id);
	if (auto namePtr = getName(getValue<Pointer>(mGetNameFirstParameter), hash))
		result = namePtr;

	//mFreeF0C0Ptr(f0c0);
	return result;
}

Inventory::ItemData* Inventory::getItemAt(int slot)
{
	ItemData *result = nullptr;
	auto f0c0 = getF0c0();
	auto arg = getArgument(f0c0, getValue<Pointer>(mUnnamedArgumentPointer) + 0x50);
	
	if (arg) {
		auto argForGetItemAt = getArgumentForGetItemAt(f0c0, arg);
		result = getItemAtSlot(f0c0, getValue<void*>((Pointer)argForGetItemAt + 0xA8), slot);
	}

	//mFreeF0C0Ptr(f0c0);
	return result;
}

void Inventory::setInventorySize(unsigned size)
{
	if (Pointer slotCountPtr = pointerPath(mInventorySizeBase, 0x50))
		if (slotCountPtr = pointerPath(slotCountPtr, 0x90))
			setValue(slotCountPtr, size);
}

void Inventory::setWeaponMagazineSize(WeaponId id, int capacity)
{
	Pointer weaponInfoTable = pointerPath(mWeaponInfoTableBase, 0x78, 0x30, 0);
	int tableSize = getValue<int>(weaponInfoTable + 0x1C);
	weaponInfoTable += 0x20;

	for (std::int64_t i = 0; i < tableSize; ++i)
	{
		if (getValue<WeaponId>(pointerPath(weaponInfoTable + i * 8, 0x10)) == id) {
			setValue(pointerPath(weaponInfoTable + i * 8, 0x18, 0x20, 0x48), capacity);
			break;
		}
	}
}

void Inventory::toggleItemCapacityCheck(bool toggle)
{
	DWORD protect;
	if (VirtualProtect(mCapacityCheckOpcode, 3, PAGE_EXECUTE_READWRITE, &protect))
	{
		if (toggle) {
			const char *original = "\x0F\x4C\xD8";
			memcpy(mCapacityCheckOpcode, original, 3);
		}
		else
			memset(mCapacityCheckOpcode, 0x90, 3);

		VirtualProtect(mCapacityCheckOpcode, 3, protect, &protect);
	}
}

Pointer Inventory::getF0c0()
{
	return static_cast<Pointer>(mGetF0C0Ptr(getValue<Pointer>(mF0C0ArgumentBase), ~0u));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct Stats::Microseconds //microseconds = overflows * 0xFFFFFFFF + microseconds
{
	std::uint32_t microseconds;
	std::uint32_t overflows;
};

struct Stats::Timer //Game time = mGeneralTimer - mUnknownFrozenTimer - mPauseTimer1
{
	char padding[0x18 /*24*/];
	Microseconds mGeneralTimer1; //runs all the time
	Microseconds mUnknownFrozenTimer1; //char padding2[0x8 /*8*/];
	Microseconds mInventoryTimer1; //runs when in inventory/map/etc
	Microseconds mPauseTimer1; //runs when paused
	char padding3[0x20 /*32*/];
	Microseconds mGeneralTimer2; //runs all the time
	Microseconds mUnknownFrozenTimer2; //char padding4[0x8 /*8*/];
	Microseconds mInventoryTimer2; //runs when in inventory/map/etc
	Microseconds mPauseTimer2; //runs when paused
};

Stats::Stats()
	:mTimerBase(patternScan("48 8B 05  ????????  48 85 C9  75 ??  48 85 C0  74 ??  88 48 53", processName)),
	mSaveCounterBase(patternScan("48 8B 0D ????????  48 85 C9  0F84 ????????  48 8B 81 ????????  48 85 C0 75 ??  45 33 C0  8D 50 38  48 8B CB  E8 ????????  48 8B 0D ????????  44 8B C6", processName))
{
	if (!(mTimerBase && mSaveCounterBase))
		throw std::runtime_error("Could not find one or more addresses");

	mTimerBase = getPointerFromImmediate(mTimerBase + 3);
	mSaveCounterBase = getPointerFromImmediate(mSaveCounterBase + 3);

	#ifndef NDEBUG
	cout << (void*)mTimerBase << " timer base" << endl;
	cout << (void*)mSaveCounterBase << " save counter base" << endl;
	#endif
}

void Stats::setGeneralTimer(unsigned microseconds, unsigned overflows)
{
	auto timer = getTimer();
	timer->mGeneralTimer1.microseconds = timer->mGeneralTimer2.microseconds = microseconds;
	timer->mGeneralTimer1.overflows = timer->mGeneralTimer2.overflows = overflows;
}

void Stats::setInventoryTimer(unsigned microseconds, unsigned overflows)
{
	auto timer = getTimer();
	timer->mInventoryTimer1.microseconds = timer->mInventoryTimer2.microseconds = microseconds;
	timer->mInventoryTimer1.overflows = timer->mInventoryTimer2.overflows = overflows;
}

void Stats::setPauseTimer(unsigned microseconds, unsigned overflows)
{
	auto timer = getTimer();
	timer->mPauseTimer1.microseconds = timer->mPauseTimer2.microseconds = microseconds;
	timer->mPauseTimer1.overflows = timer->mPauseTimer2.overflows = overflows;
}

void Stats::setPlayedTime(unsigned hours, unsigned minutes, unsigned seconds)
{
	auto timer = getTimer();
	std::uint64_t microseconds = 0;
	timer->mUnknownFrozenTimer1 = timer->mUnknownFrozenTimer2 = { 0, 0 };
	timer->mPauseTimer1 = timer->mPauseTimer2 = { 0, 0 };

	microseconds += 1000000ull * seconds;
	microseconds += 1000000ull * minutes * 60;
	microseconds += 1000000ull * hours * 60 * 60;

	timer->mGeneralTimer1.overflows = timer->mGeneralTimer2.overflows = microseconds / 0xFFFFFFFFull;
	timer->mGeneralTimer1.microseconds = timer->mGeneralTimer2.microseconds = microseconds % 0xFFFFFFFFull;
}

void Stats::setSaveCount(unsigned count)
{
	*reinterpret_cast<std::uint32_t*>(pointerPath(mSaveCounterBase, 0x198, 0x24)) = count;
}

Stats::Timer* Stats::getTimer()
{
	return reinterpret_cast<Timer*>(pointerPath(mTimerBase, 0x60, 0));
}