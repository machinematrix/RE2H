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
	FunctionType getFunctionPointer(std::string_view pattern, std::wstring_view mModuleName)
	{
		if (Pointer result = patternScan(pattern, mModuleName)) {
			return reinterpret_cast<FunctionType>(getPointerFromImmediate(result + 1));
		}

		return nullptr;
	}
}

struct Game::TextHash
{
	std::int64_t firstPart, secondPart;
};

struct Game::Microseconds //microseconds = overflows * 0xFFFFFFFF + microseconds
{
	std::uint32_t microseconds;
	std::uint32_t overflows;
};

struct Game::Timer //Game time = mGeneralTimer - mUnknownFrozenTimer - mPauseTimer1
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

Game::Game()
{
	PatternScanner scanner(processName);
	std::pair<Pointer*, std::string_view> members[] = {
		{ &mInventorySizeBase, "48 8B 15 ????????  45 33 C0  E8 ????????  0FB6 ??  48 8B 43 50  4C 39 70 18"},
		{ reinterpret_cast<Pointer*>(&getWeaponTextHash), "E8 ????????  48 8B 43 50  48 39 78 18  75 ??  8B 05 ????????  0FB7 0D" },
		{ reinterpret_cast<Pointer*>(&getItemTextHash), "48 89 5C 24 10  48 89 74 24 18  48 89 7C 24 20  55  41 56  41 57  48 8B EC  48 83 EC 60  44 0FB7 15 ????????" },
		{ reinterpret_cast<Pointer*>(&getName), "E8 ????????  33 FF  48 85 C0  74 ??  48 C7 C1 FFFFFFFF  0F1F 44 00 00" },
		{ &mGetNameFirstParameter, "48 8B 0D ????????  48 8D 54 24 30  89 7C 24 30  66 89 74 24 34  66 89 6C 24 36  4C 89 74 24 38  E8 ????????  33 FF" },
		{ &mBB0Base, "48 8B 15 ????????  48 85 C0  74 ??  44 8B 40 14  EB ??  41 B8 FFFFFFFF  48 8B CB  48 85 D2  75 ??  45 33 C0  41 8D 50 38  E8 ????????  33 C0  48 8B 5C 24 30  48 83 C4 20" },
		{ &mUnnamedArgumentPointer, "48 8B 15 ????????  48 83 78 18 00  75 ??  48 85 D2  75 19  45 33 C0  41 8D 50 38  E8 ????????   32 C0  48 8B 5C 24 30  48 83 C4 20  5F  C3  48 8B 82" },
		{ reinterpret_cast<Pointer*>(&getArgument), "E8 ????????  48 8B 4B 50  48 83 79 18 00  0F85 ????????  48 8B CB  48 85 C0  75 ??  45 33 C0  41 8D 50 38  48 8B 5C 24 30  48 8B 74 24 38  48 83 C4 20  5F  E9 ????????  48 8B D0  E8 ????????  48 8B 4B 50  48 8B F8  48 83 79 18 00  0F85 ????????  45 33 C0  48 8B D0" },
		{ reinterpret_cast<Pointer*>(&getArgumentForGetItemAt), "E8 ????????  48 8B 4B 50  48 8B F8  48 83 79 18 00  0F85 ????????  45 33 C0  48 8B D0  48 8B CB  E8 ????????  48 8B 4B 50  0FB6 C0  48 8B 51 18  48 85 D2  74 04  32 C0  EB 05  85 C0  0F95 C0  48 85 D2  75 78  84 C0  74 74  83 BE 74050000 01  74 6B  48 8B CB  48 85 FF  74 8A  48 8B D7  E8 ????????  0FB6 C8  48 8B 43 50  48 83 78 18 00  75 4D  85 C9  74 49  41 B0 01  48 8B D6  48 8B CB  E8 ????????  48 8B 43 50  48 83 78 18 00  75 30  48 8B 05 ????????  48 85 C0  75 1D  45 33 C0  8D 50 38  48 8B CB  48 8B 5C 24 30  48 8B 74 24 38  48 83 C4 20  5F  E9 ????????  C7 40 68 06000000  48 8B 5C 24 30  48 8B 74 24 38  48 83 C4 20  5F  C3" },
		{ reinterpret_cast<Pointer*>(&getItemAtSlot), "48 89 5C 24 18  48 89 7C 24 20  41 56  48 83 EC 20  48 8B 41 50  45 0FB6 F1  41 8B F8  48 8B D9  48 83 78 18 00  74 13  33 C0  48 8B 5C 24 40  48 8B 7C 24 48  48 83 C4 20  41 5E  C3" },
		{ &mF0C0ArgumentBase, "48 8B 0D ????????  E8 ????????  48 8B D8  83 78 78 00  75 ??  48 8B C8  E8 ????????  FF 43 78  48 8B 05  ????????" },
		{ &mWeaponInfoTableBase, "4C 8B 05 ????????  48 85 C9  74 ?? 48 8B 41 10" },
		{ &mCapacityCheckOpcode, "0F4C D8  48 85 F6" },
		{ &mItemCapacityFunction, "41 8D 40 F1  83 F8 12  77 39  48 98" },
		{ &mTimerBase, "48 8B 05  ????????  48 85 C9  75 ??  48 85 C0  74 ??  88 48 53" },
		{ &mSaveCounterBase, "48 8B 0D ????????  48 85 C9  0F84 ????????  48 8B 81 ????????  48 85 C0 75 ??  45 33 C0  8D 50 38  48 8B CB  E8 ????????  48 8B 0D ????????  44 8B C6" },
		{ &mHealthBase, "48 8B 3D ????????  0F5B ??  0F5A ??  48 85 FF  75 ??  45 33 C0  8D 57 38  E8 ????????  E9 ????????  48 8B D7" },
		{ &mPlayerBase, "48 8B 0D ????????  48 85 C9  0F84 ????????  48 8B 49 18  48 85 C9  75 ??  8D 51 46  45 33 C0  48 8B CB  E8 ????????  48 8B CF  48 8B 43 50  48 39 78 18  0F85 ????????  48 85 C9  0F84 ????????  48 8D 54 24 20  E8 ????????  48 8B 43 50  48 39 78 18  0F85 ????????  0F28 44 24 20" },
		{ &mClippingFunction, "40 55  53  56  57  41 56  48 8D 6C 24 B0  48 81 EC ????????  0F29 B4 24" },
		{ &mCollisionCheckFunction, "74 ??  8B 41 54  D1 E8" }
	};

	for (unsigned i = 0; i < sizeof(members) / sizeof(decltype(members[i])); ++i)
		scanner.addPattern(i, members[i].second);

	scanner.scan();
	scanner.waitForScan();

	for (unsigned i = 0; i < sizeof(members) / sizeof(decltype(members[i])); ++i) {
		*members[i].first = scanner.getScanResult(i);
		if (*members[i].first == nullptr)
			throw std::runtime_error("Could not find one or more addresses");
	}

	getWeaponTextHash = reinterpret_cast<decltype(getWeaponTextHash)>(getPointerFromImmediate(reinterpret_cast<Pointer>(getWeaponTextHash) + 0x1));
	getName = reinterpret_cast<decltype(getName)>(getPointerFromImmediate(reinterpret_cast<Pointer>(getName) + 0x1));
	getArgument = reinterpret_cast<decltype(getArgument)>(getPointerFromImmediate(reinterpret_cast<Pointer>(getArgument) + 0x1));
	getArgumentForGetItemAt = reinterpret_cast<decltype(getArgumentForGetItemAt)>(getPointerFromImmediate(reinterpret_cast<Pointer>(getArgumentForGetItemAt) + 0x1));
	getF0C0Ptr = reinterpret_cast<decltype(getF0C0Ptr)>(mF0C0ArgumentBase ? getPointerFromImmediate(mF0C0ArgumentBase + 0x7 + 0x1) : nullptr);

	mInventorySizeBase = getPointerFromImmediate(mInventorySizeBase + 0x3);
	mBB0Base = getPointerFromImmediate(mBB0Base + 0x3);
	mGetNameFirstParameter = getPointerFromImmediate(mGetNameFirstParameter + 0x3);
	mUnnamedArgumentPointer = getPointerFromImmediate(mUnnamedArgumentPointer + 0x3);
	mF0C0ArgumentBase = getPointerFromImmediate(mF0C0ArgumentBase + 0x3);
	mWeaponInfoTableBase = getPointerFromImmediate(mWeaponInfoTableBase + 0x3);
	mTimerBase = getPointerFromImmediate(mTimerBase + 0x3);
	mSaveCounterBase = getPointerFromImmediate(mSaveCounterBase + 0x3);
	mHealthBase = getPointerFromImmediate(mHealthBase + 0x3);
	mPlayerBase = getPointerFromImmediate(mPlayerBase + 0x3);

	for (unsigned i = 0; i <= static_cast<unsigned>(Game::ItemId::StuffedDoll); ++i)
	{
		auto name = getItemName(static_cast<Game::ItemId>(i));
		if (!name.empty() && name.find(L"<COLOR") == std::wstring_view::npos)
			mItemIds[name] = static_cast<ItemId>(i);
	}

	for (unsigned i = 0; i <= static_cast<unsigned>(Game::WeaponId::Minigun2); ++i)
	{
		auto name = getWeaponName(static_cast<Game::WeaponId>(i));
		if (!name.empty() && name.find(L"<COLOR") == std::wstring_view::npos)
			mWeaponIds[name] = static_cast<WeaponId>(i);
	}

	#ifndef NDEBUG
	cout << (void*)mInventorySizeBase << " -> mInventoryBase" << endl;
	cout << (void*)mGetNameFirstParameter << " -> mGetNameFirstParameter" << endl;
	cout << (void*)mBB0Base << " -> BB0 Base" << endl;
	cout << getWeaponTextHash << " -> getWeaponTextHash entry point" << endl;
	cout << getName << " -> getName entry point" << endl;
	cout << (void*)mTimerBase << " timer base" << endl;
	cout << (void*)mSaveCounterBase << " save counter base" << endl;
	#endif
}

Game::~Game()
{
	toggleItemCapacityCheck(true);
	setUniversalItemCapacity(0);
	toggleClipping(true);
}

std::wstring_view Game::getWeaponName(WeaponId id)
{
	TextHash hash;
	std::wstring_view result;
	auto f0c0 = getF0c0();

	getWeaponTextHash(f0c0, getB10(mBB0Base), id, hash);
	if(auto namePtr = getName(getValue<Pointer>(mGetNameFirstParameter), hash))
		result = namePtr;

	return result;
}

std::wstring_view Game::getItemName(ItemId id)
{
	TextHash hash;
	std::wstring_view result;
	auto f0c0 = getF0c0();

	TextHash &hash2 = getItemTextHash(hash, f0c0, getValue<Pointer>(mBB0Base), id);
	if (auto namePtr = getName(getValue<Pointer>(mGetNameFirstParameter), hash))
		result = namePtr;

	return result;
}

Game::WeaponId Game::getWeaponId(std::wstring_view name)
{
	WeaponId result = WeaponId::Invalid;
	auto it = mWeaponIds.find(name);

	if (it != mWeaponIds.end())
		result = it->second;

	return result;
}

Game::ItemId Game::getItemId(std::wstring_view name)
{
	ItemId result = ItemId::Invalid;
	auto it = mItemIds.find(name);
	
	if (it != mItemIds.end())
		result = it->second;

	return result;
}

Game::ItemData* Game::getItemAt(int slot)
{
	ItemData *result = nullptr;
	auto f0c0 = getF0c0();
	auto arg = getArgument(f0c0, getValue<Pointer>(mUnnamedArgumentPointer) + 0x50);
	
	if (arg) {
		auto argForGetItemAt = getArgumentForGetItemAt(f0c0, arg);
		result = getItemAtSlot(f0c0, getValue<void*>((Pointer)argForGetItemAt + 0xA8), slot);
	}

	return result;
}

void Game::setInventorySize(unsigned size)
{
	if (Pointer slotCountPtr = pointerPath(mInventorySizeBase, 0x50, 0x90))
			setValue(slotCountPtr, size);
}

void Game::setWeaponMagazineSize(WeaponId id, int capacity)
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

void Game::toggleInfiniteMagazine(WeaponId id, bool toggle)
{
	Pointer weaponInfoTable = pointerPath(mWeaponInfoTableBase, 0x78, 0x30, 0);
	int tableSize = getValue<int>(weaponInfoTable + 0x1C);
	weaponInfoTable += 0x20;

	for (std::int64_t i = 0; i < tableSize; ++i)
	{
		if (getValue<WeaponId>(pointerPath(weaponInfoTable + i * 8, 0x10)) == id) {
			setValue(pointerPath(weaponInfoTable + i * 8, 0x18, 0x20, 0x18), toggle ? 257 : 1);
			break;
		}
	}
}

//std::vector<Pointer> Game::getWeaponInfoTableEntries()
//{
//	std::vector<Pointer> result;
//	Pointer weaponInfoTable = pointerPath(mWeaponInfoTableBase, 0x78, 0x30, 0);
//	int tableSize = getValue<int>(weaponInfoTable + 0x1C);
//	weaponInfoTable += 0x20;
//
//	for (std::int64_t i = 0; i < tableSize; ++i)
//		result.push_back(getValue<Pointer>(weaponInfoTable + i * 8));
//
//	return result;
//}

void Game::toggleItemCapacityCheck(bool toggle)
{
	DWORD protect;
	if (VirtualProtect(mCapacityCheckOpcode, 3, PAGE_EXECUTE_READWRITE, &protect))
	{
		if (toggle) {
			memcpy(mCapacityCheckOpcode, "\x0F\x4C\xD8", 3);
		}
		else
			memset(mCapacityCheckOpcode, 0x90, 3);

		VirtualProtect(mCapacityCheckOpcode, 3, protect, &protect);
	}
}

void Game::setUniversalItemCapacity(int value)
{
	DWORD protect;
	if (VirtualProtect(mItemCapacityFunction, 7, PAGE_EXECUTE_READWRITE, &protect))
	{
		if (value > 0) {
			*mItemCapacityFunction = '\xB8';
			memcpy(mItemCapacityFunction + 1, &value, 4);
			*(mItemCapacityFunction + 5) = '\xC3';
			*(mItemCapacityFunction + 6) = '\x90';
		}
		else
			memcpy(mItemCapacityFunction, "\x41\x8D\x40\xF1\x83\xF8\x12", 7);

		VirtualProtect(mItemCapacityFunction, 7, protect, &protect);
	}
}

void Game::setGeneralTimer(unsigned microseconds, unsigned overflows)
{
	auto timer = getTimer();
	timer->mGeneralTimer1.microseconds = timer->mGeneralTimer2.microseconds = microseconds;
	timer->mGeneralTimer1.overflows = timer->mGeneralTimer2.overflows = overflows;
}

void Game::setInventoryTimer(unsigned microseconds, unsigned overflows)
{
	auto timer = getTimer();
	timer->mInventoryTimer1.microseconds = timer->mInventoryTimer2.microseconds = microseconds;
	timer->mInventoryTimer1.overflows = timer->mInventoryTimer2.overflows = overflows;
}

void Game::setPauseTimer(unsigned microseconds, unsigned overflows)
{
	auto timer = getTimer();
	timer->mPauseTimer1.microseconds = timer->mPauseTimer2.microseconds = microseconds;
	timer->mPauseTimer1.overflows = timer->mPauseTimer2.overflows = overflows;
}

void Game::setPlayedTime(unsigned hours, unsigned minutes, unsigned seconds)
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

void Game::setSaveCount(unsigned count)
{
	*reinterpret_cast<std::uint32_t*>(pointerPath(mSaveCounterBase, 0x198, 0x24)) = count;
}

void Game::setHealth(int offset)
{
	//int result = -1;
	auto f0c0 = getF0c0();

	if (auto base = pointerPath(mHealthBase, 0x50, 0x230, 0)) {
		setValue(base + 0x58, offset);
		//result = setHealth(f0c0, base, offset);
	}

	//return result;
}

int Game::getHealth()
{
	int result = -1;
	auto f0c0 = getF0c0();

	if (auto base = pointerPath(mHealthBase, 0x50, 0x230, 0))
		result = getValue<int>(base + 0x58);

	return result;
}

Game::Coordinates* Game::getCoords()
{
	return reinterpret_cast<Coordinates*>(pointerPath(mPlayerBase, 0x18, 0x30));
}

void Game::toggleClipping(bool toggle)
{
	DWORD protect, protect2;
	if (VirtualProtect(mClippingFunction, 1, PAGE_EXECUTE_READWRITE, &protect))
	{
		if (VirtualProtect(mCollisionCheckFunction, 1, PAGE_EXECUTE_READWRITE, &protect2)) {
			*mCollisionCheckFunction = toggle ? 0x74 : 0xEB;
			*mClippingFunction = toggle ? 0x40 : 0xC3;
			VirtualProtect(mCollisionCheckFunction, 1, protect2, &protect2);
		}
		VirtualProtect(mClippingFunction, 1, protect, &protect);
	}
}

Game::Timer* Game::getTimer()
{
	return reinterpret_cast<Timer*>(pointerPath(mTimerBase, 0x60, 0));
}

Pointer Game::getF0c0()
{
	return static_cast<Pointer>(getF0C0Ptr(getValue<Pointer>(mF0C0ArgumentBase), ~0u));
}