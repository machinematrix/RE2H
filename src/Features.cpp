#include "Features.h"
#include "Memory.h"
#include <iostream>

#ifndef NDEBUG
using std::cout;
using std::cin;
using std::endl;
#endif

namespace
{
	constexpr wchar_t processName[] = L"re2.exe";

	Pointer getInventoryPointer(Pointer inventoryBase)
	{
		return pointerPath(inventoryBase, 0, 0xC8, 0x38, 0x88, 0x50) + 0x10;
	}
}

Inventory::Inventory()
	:mInventory(patternScan("48 8B 15 ????????  45 33 C0  E8 ????????  0FB6 ??  48 8B 43 50  4C 39 70 18", processName))
{
	mInventory += 3;
	mInventory += static_cast<std::int64_t>(getValue<std::int32_t>(mInventory)) + 4;

	//mInventory = pointerPath(mInventory, 0, 0xC8, 0x38, 0x88, 0x50);

	#ifndef NDEBUG
	cout << (void*)mInventory << endl;
	#endif
}

void* Inventory::getInventoryAddress()
{
	return getInventoryPointer(mInventory);
}