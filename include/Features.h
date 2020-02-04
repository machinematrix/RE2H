#ifndef __FEATURES__
#define __FEATURES__
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <stdexcept>

class Game
{
	using Pointer = char*;
public:
	enum class WeaponId : std::int32_t;
	enum class ItemId : std::int32_t;
	struct ItemData;
	struct Coordinates;

	Game();
	~Game();
	std::wstring_view getWeaponName(WeaponId id);
	std::wstring_view getItemName(ItemId id);
	WeaponId getWeaponId(std::wstring_view name);
	ItemId getItemId(std::wstring_view name);
	ItemData* getItemAt(int slot);
	void setInventorySize(unsigned size);
	void setWeaponMagazineSize(WeaponId id, int capacity);
	void toggleInfiniteMagazine(WeaponId, bool);
	//std::vector<Pointer> getWeaponInfoTableEntries();
	void toggleItemCapacityCheck(bool toggle);
	void setUniversalItemCapacity(int);
	void setGeneralTimer(unsigned microseconds, unsigned overflows);
	void setInventoryTimer(unsigned microseconds, unsigned overflows);
	void setPauseTimer(unsigned microseconds, unsigned overflows);
	void setPlayedTime(unsigned hours, unsigned minutes, unsigned seconds);
	void setSaveCount(unsigned count);
	void setHealth(int offset);
	int getHealth();
	Coordinates* getCoords();
	void toggleClipping(bool);

private:
	struct Microseconds;
	struct TextHash;
	struct Timer;
	Timer* getTimer();
	Pointer getF0c0();

	std::unordered_map<std::wstring_view, WeaponId> mWeaponIds;
	std::unordered_map<std::wstring_view, ItemId> mItemIds;
	Pointer mInventorySizeBase = nullptr; //re2.exe+70A17E0
	Pointer mBB0Base = nullptr; //re2.exe+70A5EA8
	Pointer mGetNameFirstParameter = nullptr; //re2.exe+7095BF0
	Pointer mWeaponInfoTableBase = nullptr; //re2.exe+709C2F0
	Pointer mCapacityCheckOpcode = nullptr;
	Pointer mF0C0ArgumentBase = nullptr; //re2.exe+7095E08 function that returns it and argument to function can be found from here
	Pointer mUnnamedArgumentPointer = nullptr; //re2.exe+709B5E0
	Pointer mItemCapacityFunction = nullptr;
	Pointer mTimerBase = nullptr; //re2.exe+709E120
	Pointer mSaveCounterBase = nullptr; //re2.exe+700A8860
	Pointer mHealthBase = nullptr; //re2.exe+70A7CD0 -> 0x50 -> 0x230
	Pointer mPlayerBase = nullptr; //re2.exe+70973C8
	Pointer mClippingFunction = nullptr; //re2.exe+1E7AC10
	Pointer mCollisionCheckFunction = nullptr; //re2.exe+22BA674

	//Game functions
	std::int64_t	(*getWeaponTextHash)		(void* /*f0c0*/, void* /*bb0*/, WeaponId, TextHash&) = nullptr; //returns 0 if it can't find the name
	TextHash&		(*getItemTextHash)			(TextHash&, void* /*f0c0*/, void* /*bb0 + 0*/, ItemId) = nullptr; //returns a pointer to the first argument
	const wchar_t*	(*getName)					(void*, TextHash&) = nullptr;
	void*			(*getArgument)				(void* /*F0C0*/, void* /*unnamedArgument + 0x50*/) = nullptr;
	void*			(*getArgumentForGetItemAt)	(void* /*F0C0*/, void* /*return value from getArgument*/) = nullptr;
	ItemData*		(*getItemAtSlot)			(void* /*F0C0*/, void* /*result from function above + 0xA8*/, std::int64_t /*slotIndex*/) = nullptr;
	void*			(*getF0C0Ptr)				(void* /* *mF0C0Base */, std::uint32_t /*~0u*/) = nullptr;
	//int (*damagePlayer)(void* /*F0C0*/, void*, int);
};

struct Game::ItemData //size 0xF0 (240) bytes
{
private:
	void *unknownPtr1;
	std::uint64_t unknownInt1;
	void *derivedPtr; //points to this+0x60
	void *unknownPtr2; //points to this+0x90
	void *unknownPtr3; //changes in SLS60 when changing ammo type. sometimes points to this+0xC0
public:
	std::uint64_t slotIndex; //zero-based left-to-right index of this item in inventory
private:
	void *unknownPtr4;
	std::uint64_t unknownInt2;
	std::uint64_t unknownInt3;
	std::uint64_t unknownInt4;
	std::uint64_t unknownInt5;
	std::uint64_t unknownInt6;
	//DERIVED
	void *vtable;
	std::uint32_t unknown;
	std::uint32_t unknown2;
public:
	ItemId itemId;
	WeaponId weaponId; //has to be -1 for itemId to be taken into account
	std::uint32_t upgrades; //bitset of upgrades
	ItemId ammoType;
	std::uint32_t ammo;
private:
	std::uint32_t unknownInt7;
	std::uint64_t unknownInt8;
	void *unknownPtr5;
	std::uint64_t unknownInt9;
	std::uint64_t unknownInt10;
	std::uint64_t unknownInt11;
	std::uint64_t unknownInt12;
	std::uint64_t unknownInt13;
	void *unknownPtr6;
	std::uint64_t unknownInt15;
	std::uint64_t unknownInt16;
	std::uint64_t unknownInt17;
	std::uint64_t unknownInt18;
	std::uint64_t unknownInt19;
};

struct Game::Coordinates
{
	float mX, mY, mZ;
};

enum class Game::WeaponId
{
	Invalid = -1,
	Matilda = 0x01,
	M19,
	JMBHp3,
	QuickdrawArmy,
	MUP = 0x07,
	BroomHc,
	SLS60,
	W870 = 0x0B,
	MQ11 = 0x15,
	LE5 = 0x17,
	LightningHawk = 0x1F,
	EMFVisualizer = 0x29,
	GM79,
	ChemicalFlamethrower,
	SparkShot,
	ATM4,
	CombatKnife,
	CombatKnifeInfinite,
	AntiTankRocket = 0x31,
	Minigun,
	HandGrenade = 0x41,
	FlashGrenade,
	SamuraiEdge_Original = 0x52,
	SamuraiEdge_Albert = 0x55,
	ATM4Infinite = 0xDE,
	AntiTankRocketInfinite = 0xF2,
	Minigun2 = 0xFC
};

enum class Game::ItemId
{
	Invalid = 0,
	FirstAidSpray = 0x01,
	GreenHerb,
	RedHerb,
	BlueHerb,
	MixedHerbsGG,
	MixedHerbsGR,
	MixedHerbsGB,
	MixedHerbsGGB,
	MixedHerbsGGG,
	MixedHerbsGRB,
	MixedHerbsRB,
	HandgunAmmo = 0x0F,
	ShotgunShells,
	SubmachineGunAmmo,
	MAGAmmo,
	AcidRounds = 0x16,
	FlameRounds,
	NeedleCartridges,
	Fuel,
	LargeCaliberHandgunAmmo,
	HighPoweredRounds,
	Detonator = 0x1F,
	InkRibbon,
	WoodenBoards,
	DetonatorWithoutBattery,
	Battery,
	Gunpowder,
	GunpowderLarge,
	HighGradeGunpowderYellow,
	HighGradeGunpowderWhite,
	MatildaMagazine = 0x30,
	MatildaMuzzleBrake,
	MatildaStock,
	SLS60SpeedLoader,
	JMBHp3Laser,
	SLS60ReinforcedFrame,
	JMBHp3Magazine,
	W870Stock,
	W870Barrel,
	MQ11Magazine = 0x3A,
	MQ11Suppressor = 0x3C,
	LightningHawkSight = 0x3D,
	LightningHawkBarrel,
	GM79Stock = 0x40,
	FlamethrowerRegulator,
	SparkShotHighVoltageCondenser,
	RollFilm1 = 0x48,
	RollFilm2,
	RollFilm3,
	RollFilm4,
	RollFilm5,
	StorageRoomKey,
	MechanicJackHandle = 0x4F,
	SquareCrank,
	UnicornMedallion,
	SpadeKey,
	ParkingGarageKeyCard,
	WeaponsLockerKeyCard,
	RoundHandle = 0x56,
	STARSBadge,
	Scepter,
	RedJewel = 0x5A,
	BejeweledBox,
	BishopPlug = 0x5D,
	RookPlug,
	KingPlug,
	PictureBlock = 0x62,
	USBDongleKey = 0x66,
	SpareKey = 0x70,
	RedBook = 0x72,
	StatueLeftHand,
	LeftArmWithBook,
	LionMedallion = 0x76,
	DiamondKey,
	CarKey,
	MaidenMedallion = 0x7C,
	PowerPanelPart1 = 0x7E,
	PowerPanelPart2,
	LoverRelief,
	SmallGear,
	LargeGear,
	CourtyardKey,
	KnightPlug,
	PawnPlug,
	QueenPlug,
	BoxedElectronicPart1,
	BoxedElectronicPart2,
	OrphanageKey = 0x9F,
	ClubKey,
	HeartKey = 0xA9,
	USSDigitalVideoCassette,
	TBarValveHandle = 0xB0,
	DispersalCartridgeEmpty = 0xB3,
	DispersalCartridgeSolution,
	DispersalCartridgeHerbicide,
	JointPlug = 0xB7,
	UpgradeChipAdmin = 0xBA,
	IDWristbandAdmin,
	SignalModulator = 0xBD,
	Trophy1,
	Trophy2,
	SewersKey = 0xC2,
	IDWristbandVisitor,
	IDWristbandGeneralStaff,
	IDWristbandSeniorStaff,
	UpgradeChipGeneralStaff,
	UpgradeChipSeniorStaff,
	LabDigitalVideoCassette,
	Briefcase = 0xE6,
	FuseMainHall = 0xF0,
	FuseBreakRoomHallway,
	Scissors = 0xF3,
	CuttingTool,
	StuffedDoll,
	HipPouch = 0x106
};
#endif