#ifndef __FEATURES__
#define __FEATURES__
#include <cstdint>

class Inventory
{
	using Pointer = char*;
public:
	struct ItemData;

	Inventory();
	void* getInventoryAddress();

private:
	Pointer mInventory;
};

struct Inventory::ItemData
{
	void *vtable;
	std::uint32_t unknown;
	std::uint32_t unknown;
	std::uint32_t itemId;
	std::uint32_t weaponId; //has to be -1 for itemId to be taken into account
	std::uint32_t upgrades; //bitset of upgrades
	std::uint32_t ammoType;
	std::uint32_t ammo;
};

#endif