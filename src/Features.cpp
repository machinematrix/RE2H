#include "Features.h"
#include "Memory.h"
#undef NDEBUG

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

	Pointer getPointerFromImmediate(Pointer codeAddress)
	{
		return codeAddress + static_cast<std::int64_t>(getValue<std::int32_t>(codeAddress)) + 4;
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
	:mInventoryBase(patternScan("48 8B 15 ????????  45 33 C0  E8 ????????  0FB6 ??  48 8B 43 50  4C 39 70 18", processName)),
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

	mInventoryBase = getPointerFromImmediate(mInventoryBase + 3);
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
	cout << (void*)getF0c0(mExecutableBaseAddress) << " -> F0C0 Base" << endl;
	cout << (void*)getB10(mBB0Base) << " -> BB0 Base" << endl;
	cout << getWeaponTextHash << " -> getWeaponTextHash entry point" << endl;
	cout << getName << " -> getName entry point" << endl;
	#endif
}

std::wstring_view Inventory::getWeaponName(WeaponId id)
{
	TextHash hash;
	std::wstring_view result;

	getWeaponTextHash(getF0c0(mExecutableBaseAddress), getB10(mBB0Base), id, hash);
	result = getName(getValue<Pointer>(mGetNameFirstParameter), hash);

	return result;
}

std::wstring_view Inventory::getItemName(ItemId id)
{
	TextHash hash;
	std::wstring_view result;

	TextHash &hash2 = getItemTextHash(hash, getF0c0(mExecutableBaseAddress), getValue<Pointer>(mBB0Base), id);
	result = getName(getValue<Pointer>(mGetNameFirstParameter), hash);

	return result;
}

void* Inventory::getInventoryAddress()
{
	return getInventoryPointer();
}

Inventory::ItemData* Inventory::getItemAt(int slot)
{
	auto f0c0 = getF0c0(mExecutableBaseAddress);
	auto arg = getArgument(f0c0, getValue<Pointer>(mUnnamedArgumentPointer) + 0x50);
	auto argForGetItemAt = getArgumentForGetItemAt(f0c0, arg);

	return getItemAtSlot(f0c0, getValue<void*>((Pointer)argForGetItemAt + 0xA8), slot);
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

Inventory::GameInventory* Inventory::getInventoryPointer()
{
	/*Pointer invClaire = pointerPath(mInventoryBase, 0xC8, 0x38, 0x88);
	if (invClaire) {
		if (!(getValue<std::uint64_t>(invClaire) & 0x00FF000000000000))
			return reinterpret_cast<GameInventory*>(pointerPath(invClaire, 0x50, 0x10));
		else
			return nullptr;
	}
	else
		return reinterpret_cast<GameInventory*>(pointerPath(mInventoryBase, 0xC8, 0x38, 0xE0, 0x10));*/
	Pointer invClaire = pointerPath(mInventoryBase, 0xC8, 0x38, 0xC8);
	if (getValue<Pointer>(invClaire))
	{
		invClaire = pointerPath(invClaire, 0x10, 0x110, 0x10);
		if ((reinterpret_cast<std::int64_t>(invClaire) & 0x0000FFF000000000) != 0x00007FF000000000)
		{
			return reinterpret_cast<GameInventory*>(invClaire);
		}
		else
			return reinterpret_cast<GameInventory*>(pointerPath(mInventoryBase, 0xC8, 0x38, 0xE0, 0x10));
	}
	else
		return nullptr;
}

//0x00007FF7087B0570
//0x0000FFF000000000
//0x00FF000000000000