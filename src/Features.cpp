#include "Features.h"
#include "Memory.h"
//#undef NDEBUG

#ifndef NDEBUG
#include <iostream>
using std::cout;
using std::cin;
using std::endl;
using std::hex;
using std::dec;
#endif

enum ExecutableOffsets : std::int64_t
{
	FocoBase = 0x070A27A8
};

namespace
{
	constexpr wchar_t processName[] = L"re2.exe";

	Pointer getF0c0(Pointer executableBase)
	{
		return pointerPath(executableBase + FocoBase, 0x78, 0);
	}

	Pointer getB10(Pointer bb0Base)
	{
		return pointerPath(bb0Base, 0x130, 0);
	}
}

struct Inventory::GameInventory
{
	void *vtable;
	std::uint64_t unknown;
	ItemData *items[20];
};

struct Inventory::TextHash
{
	std::int64_t firstPart, secondPart;
};

Inventory::Inventory()
	:mInventorySizeBase(patternScan("48 8B 15 ????????  45 33 C0  E8 ????????  0FB6 ??  48 8B 43 50  4C 39 70 18", processName)),
	mExecutableBaseAddress(static_cast<Pointer>(getModuleInfo(processName).moduleBase)),
	getWeaponTextHash(nullptr),
	getItemTextHash(reinterpret_cast<decltype(getItemTextHash)>(patternScan("48 89 5C 24 10  48 89 74 24 18  48 89 7C 24 20  55  41 56  41 57  48 8B EC  48 83 EC 60  44 0FB7 15 ????????", processName))),
	getName(nullptr),
	mGetNameFirstParameter(patternScan("48 8B 0D ????????  48 8D 54 24 30  89 7C 24 30  66 89 74 24 34  66 89 6C 24 36  4C 89 74 24 38  E8 ????????  33 FF", processName)),
	mBB0Base(patternScan("48 8B 15 ????????  48 85 C0  74 ??  44 8B 40 14  EB ??  41 B8 FFFFFFFF  48 8B CB  48 85 D2  75 ??  45 33 C0  41 8D 50 38  E8 ????????  33 C0  48 8B 5C 24 30  48 83 C4 20", processName)),
	mUnnamedArgumentPointer(patternScan("48 8B 15 ????????  48 83 78 18 00  75 ??  48 85 D2  75 19  45 33 C0  41 8D 50 38  E8 ????????   32 C0  48 8B 5C 24 30  48 83 C4 20  5F  C3  48 8B 82", processName)), //multiple matches, but they all access the same variable
	getArgument(nullptr),
	getArgumentForGetItemAt(nullptr),
	getItemAtSlot(reinterpret_cast<decltype(getItemAtSlot)>(patternScan("48 89 5C 24 18  48 89 7C 24 20  41 56  48 83 EC 20  48 8B 41 50  45 0FB6 F1  41 8B F8  48 8B D9  48 83 78 18 00  74 13  33 C0  48 8B 5C 24 40  48 8B 7C 24 48  48 83 C4 20  41 5E  C3", processName)))
{
	constexpr auto itemSize = sizeof(ItemData);
	Pointer getWeaponTextHashPtr = patternScan("E8 ????????  48 8B 43 50  48 39 78 18  75 ??  8B 05 ????????  0FB7 0D", processName);
	Pointer getNamePtr = patternScan("E8 ????????  33 FF  48 85 C0  74 ??  48 C7 C1 FFFFFFFF  0F1F 44 00 00", processName);
	Pointer getArgumentPtr = patternScan("E8 ????????  48 8B 4B 50  48 83 79 18 00  0F85 ????????  48 8B CB  48 85 C0  75 ??  45 33 C0  41 8D 50 38  48 8B 5C 24 30  48 8B 74 24 38  48 83 C4 20  5F  E9 ????????  48 8B D0  E8 ????????  48 8B 4B 50  48 8B F8  48 83 79 18 00  0F85 ????????  45 33 C0  48 8B D0", processName); //multiple matches, but they all access the same variable
	Pointer getArgumentForGetItemAtPtr = patternScan("E8 ????????  48 8B 4B 50  48 8B F8  48 83 79 18 00  0F85 ????????  45 33 C0  48 8B D0  48 8B CB  E8 ????????  48 8B 4B 50  0FB6 C0  48 8B 51 18  48 85 D2  74 04  32 C0  EB 05  85 C0  0F95 C0  48 85 D2  75 78  84 C0  74 74  83 BE 74050000 01  74 6B  48 8B CB  48 85 FF  74 8A  48 8B D7  E8 ????????  0FB6 C8  48 8B 43 50  48 83 78 18 00  75 4D  85 C9  74 49  41 B0 01  48 8B D6  48 8B CB  E8 ????????  48 8B 43 50  48 83 78 18 00  75 30  48 8B 05 ????????  48 85 C0  75 1D  45 33 C0  8D 50 38  48 8B CB  48 8B 5C 24 30  48 8B 74 24 38  48 83 C4 20  5F  E9 ????????  C7 40 68 06000000  48 8B 5C 24 30  48 8B 74 24 38  48 83 C4 20  5F  C3", processName);

	getArgumentForGetItemAt = reinterpret_cast<decltype(getArgumentForGetItemAt)>(getPointerFromImmediate(getArgumentForGetItemAtPtr + 1));

	mInventorySizeBase = getPointerFromImmediate(mInventorySizeBase + 3);
	mBB0Base = getPointerFromImmediate(mBB0Base + 3);
	getWeaponTextHash = (reinterpret_cast<decltype(getWeaponTextHash)>(getPointerFromImmediate(getWeaponTextHashPtr + 1)));
	getName = (reinterpret_cast<decltype(getName)>(getPointerFromImmediate(getNamePtr + 1)));
	mGetNameFirstParameter = getPointerFromImmediate(mGetNameFirstParameter + 3);
	mUnnamedArgumentPointer = getPointerFromImmediate(mUnnamedArgumentPointer + 3);
	getArgument = (reinterpret_cast<decltype(getArgument)>(getPointerFromImmediate(getArgumentPtr + 1)));
	
	#ifndef NDEBUG
	cout << (void*)mExecutableBaseAddress << " -> mExecutableBaseAddress" << endl;
	cout << (void*)mInventoryBase << " -> mInventoryBase" << endl;
	cout << (void*)mGetNameFirstParameter << " -> mGetNameFirstParameter" << endl;
	cout << (void*)mExecutableBaseAddress << " -> F0C0 Base" << endl;
	cout << (void*)mBB0Base << " -> BB0 Base" << endl;
	cout << getWeaponTextHash << " -> getWeaponTextHash entry point" << endl;
	cout << getName << " -> getName entry point" << endl;
	#endif
}

std::wstring_view Inventory::getWeaponName(WeaponId id)
{
	TextHash hash;
	std::wstring_view result;

	getWeaponTextHash(getF0c0(mExecutableBaseAddress), getB10(mBB0Base), id, hash);
	if(auto namePtr = getName(getValue<Pointer>(mGetNameFirstParameter), hash))
		result = namePtr;

	return result;
}

std::wstring_view Inventory::getItemName(ItemId id)
{
	TextHash hash;
	std::wstring_view result;

	TextHash &hash2 = getItemTextHash(hash, getF0c0(mExecutableBaseAddress), getValue<Pointer>(mBB0Base), id);
	if (auto namePtr = getName(getValue<Pointer>(mGetNameFirstParameter), hash))
		result = namePtr;

	return result;
}

Inventory::ItemData* Inventory::getItemAt(int slot)
{
	ItemData *result = nullptr;
	auto f0c0 = getF0c0(mExecutableBaseAddress);
	auto arg = getArgument(f0c0, getValue<Pointer>(mUnnamedArgumentPointer) + 0x50);
	if (arg) {
		auto argForGetItemAt = getArgumentForGetItemAt(f0c0, arg);
		result = getItemAtSlot(f0c0, getValue<void*>((Pointer)argForGetItemAt + 0xA8), slot);
	}

	return result;
	/*ItemData *result = nullptr;
	
	if (auto inv = getInventoryPointer())
	{
		for (auto item : inv->items)
		{
			if (item->slotIndex == slot) {
				result = item;
				break;
			}
		}
	}

	return result;*/
}

void Inventory::setInventorySize(unsigned size)
{
	if (Pointer slotCountPtr = pointerPath(mInventorySizeBase, 0x50))
		if (slotCountPtr = pointerPath(slotCountPtr, 0x90))
			setValue(slotCountPtr, size);
}

//Inventory::GameInventory* Inventory::getInventoryPointer()
//{
//	/*Pointer invClaire = pointerPath(mInventoryBase, 0xC8, 0x38, 0x88);
//	if (invClaire) {
//		if (!(getValue<std::uint64_t>(invClaire) & 0x00FF000000000000))
//			return reinterpret_cast<GameInventory*>(pointerPath(invClaire, 0x50, 0x10));
//		else
//			return nullptr;
//	}
//	else
//		return reinterpret_cast<GameInventory*>(pointerPath(mInventoryBase, 0xC8, 0x38, 0xE0, 0x10));*/
//	Pointer invClaire = pointerPath(mInventoryBase, 0xC8, 0x38, 0xC8);
//	if (getValue<Pointer>(invClaire))
//	{
//		invClaire = pointerPath(invClaire, 0x10, 0x110, 0x10);
//		if ((reinterpret_cast<std::int64_t>(invClaire) & 0x0000FFF000000000) != 0x00007FF000000000)
//		{
//			return reinterpret_cast<GameInventory*>(invClaire);
//		}
//		else
//			return reinterpret_cast<GameInventory*>(pointerPath(mInventoryBase, 0xC8, 0x38, 0xE0, 0x10));
//	}
//	else
//		return nullptr;
//}

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