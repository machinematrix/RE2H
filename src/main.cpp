#include <Windows.h>
#include <CommCtrl.h>
#include <vector>
#include <string>
#include <regex>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "Utility.h"
#include "Features.h"
#include "CommandHandler.h"
//#include "Memory.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using std::cin;
using std::cout;
using std::wcout;
using std::endl;

DWORD WINAPI ConsoleMain(LPVOID);
DWORD WINAPI Win32Main(LPVOID);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

namespace
{
	std::regex singleDigitRegex("[ ]?([[:digit:]]+)");
	std::regex stringRegex("[ ]?(.+)");
	std::regex booleanRegex("[ ]?(true|false|0|1)");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH: {
			if (HANDLE hThread = CreateThread(nullptr, 0, ConsoleMain, hModule, 0, nullptr)) {
				CloseHandle(hThread);
			}
			else ErrorBox(nullptr, TEXT("Could not create thread"));
			break;
		}
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}

	return TRUE;
}

void itemName(Game &inv, std::string_view args)
{
	static std::regex rgx("[ ]?([[:digit:]]+|\\*)");
	std::cmatch match;

	if (std::regex_match(args.data(), match, rgx))
	{
		if (match[1] != '*')
			wcout << inv.getItemName(static_cast<Game::ItemId>(std::stoi(match[1]))) << endl;
		else
		{
			for (unsigned i = 0; i <= static_cast<unsigned>(Game::ItemId::StuffedDoll); ++i) {
				auto name = inv.getItemName(static_cast<Game::ItemId>(i));
				if (!name.empty() && name.find(L"<COLOR") == std::wstring_view::npos)
					wcout << std::setfill(L'0') << std::setw(3) << i << std::setw(0) << ": " << name << endl;
			}
		}
	}
	else
		cout << "Usage: itemName [id]" << endl;
}

void weaponName(Game &inv, std::string_view args)
{
	static std::regex rgx("[ ]?([[:digit:]]+|\\*)");
	std::cmatch match;

	if (std::regex_match(args.data(), match, rgx))
	{
		if (match[1] != '*')
			wcout << inv.getWeaponName(static_cast<Game::WeaponId>(std::stoi(match[1]))) << endl;
		else
		{
			for (unsigned i = 0; i <= static_cast<unsigned>(Game::WeaponId::Minigun2); ++i) {
				auto name = inv.getWeaponName(static_cast<Game::WeaponId>(i));
				if(!name.empty() && name.find(L"<COLOR") == std::wstring_view::npos)
					wcout << std::setfill(L'0') << std::setw(3) << i << std::setw(0) << ": " << name << endl;
			}
		}
	}
	else
		cout << "Usage: weaponName ([id] | *)" << endl;
}

void weaponId(Game &game, std::string_view args)
{
	std::cmatch match;

	if (std::regex_match(args.data(), match, stringRegex))
	{
		std::string asciiName = match[1];
		std::wstring unicodeName(asciiName.begin(), asciiName.end());
		auto id = game.getWeaponId(unicodeName);

		if (id != Game::WeaponId::Invalid) {
			cout << static_cast<int>(id) << endl;
		}
		else cout << "Invalid name" << endl;
	}
	else
		cout << "Usage: weaponId [name]" << endl;
}

void itemId(Game &game, std::string_view args)
{
	std::cmatch match;

	if (std::regex_match(args.data(), match, stringRegex))
	{
		std::string asciiName = match[1];
		std::wstring unicodeName(asciiName.begin(), asciiName.end());
		auto id = game.getItemId(unicodeName);

		if (id != Game::ItemId::Invalid) {
			cout << static_cast<int>(id) << endl;
		}
		else cout << "Invalid name" << endl;
	}
	else
		cout << "Usage: itemId [name]" << endl;
}

void clear(std::string_view args)
{
	system("cls");
}

void getItemAt(Game &inv, std::string_view args)
{
	std::cmatch match;

	if (std::regex_match(args.data(), match, singleDigitRegex))
	{
		if (auto item = inv.getItemAt(std::stoi(match[1])))
		{
			if (item->weaponId != Game::WeaponId::Invalid) { //if it's a gun
				wcout << item << ": " << inv.getWeaponName(item->weaponId);
				if (item->ammoType != Game::ItemId::Invalid)
					wcout << ", " << item->ammo << ' ' << inv.getItemName(item->ammoType) << " rounds";
				wcout << endl;
			}
			else if (item->itemId != Game::ItemId::Invalid) { //if it's an item
				wcout << item << ": " << inv.getItemName(item->itemId) << " quantity: " << item->ammo << endl;
			}
		}
	}
	else
		cout << "Usage: get [id]" << endl;
}

//slot item weapon upgrades ammoType ammo
void setItemAt(Game &inv, std::string_view args)
{
	static std::regex rgx("[ ]?([[:digit:]]+) (?:itemId:([[:digit:]]+)|weaponId:([[:digit:]]+))?[ ]?(?:upgrades:([[:digit:]]+)*)?[ ]?(?:ammoType:([[:digit:]]+)*)?[ ]?(?:ammo:([[:digit:]]+)*)?");
	std::cmatch match;

	if (std::regex_match(args.data(), match, rgx))
	{
		/*cout << "Slot: " << match[1] << endl;
		cout << "itemId: " << match[2] << endl;
		cout << "weaponId: " << match[3] << endl;
		cout << "upgrades: " << match[4] << endl;
		cout << "ammoType: " << match[5] << endl;
		cout << "ammo: " << match[6] << endl;*/
		auto item = inv.getItemAt(std::stoi(match[1]));

		if (item)
		{
			if (match[2].matched) {
				item->itemId = Game::ItemId(std::stoi(match[2]));
				item->weaponId = Game::WeaponId::Invalid;
			}
			else if (match[3].matched) {
				item->weaponId = Game::WeaponId(std::stoi(match[3]));
				item->itemId = Game::ItemId::Invalid;
			}

			if (match[4].matched)
				item->upgrades = std::stoi(match[4]);

			if (match[5].matched)
				item->ammoType = Game::ItemId(std::stoi(match[5]));

			if (match[6].matched)
				item->ammo = std::stoi(match[6]);
		}
		else {
			cout << "Could not find item at that slot" << endl;
		}
	}
	else
		cout << "Usage: set [slot] (itemId:[id] | weaponId:[id]) upgrades:[val] ammoType:[id] ammo:[value]\nAll arguments except slot are optional, but they must appear in the order shown above" << endl;
}

void setTime(Game &stats, std::string_view args)
{
	static std::regex rgx("[ ]?([[:digit:]]{2}):([[:digit:]]{2}):([[:digit:]]{2})");
	std::cmatch match;

	if (std::regex_match(args.data(), match, rgx))
		stats.setPlayedTime(std::stoul(match[1]), std::stoul(match[2]), std::stoul(match[3]));
	else
		cout << "Usage: setTime [hh:mm:ss]" << endl;
}

void setSaveCounter(Game &stats, std::string_view args)
{
	std::cmatch match;

	if (std::regex_match(args.data(), match, singleDigitRegex))
		stats.setSaveCount(std::stoul(match[1]));
	else
		cout << "Usage: setSaveCounter [count]" << endl;
}

void setInventorySize(Game &inv, std::string_view args)
{
	std::cmatch match;

	if (std::regex_match(args.data(), match, singleDigitRegex))
		inv.setInventorySize(std::stoul(match[1]));
	else
		cout << "Usage: inventorySize [count]" << endl;
}

void setWeaponCapacity(Game &inv, std::string_view args)
{
	static std::regex rgx("[ ]?([[:digit:]]+) ([[:digit:]]+)");
	std::cmatch match;

	if (std::regex_match(args.data(), match, rgx))
		inv.setWeaponMagazineSize(static_cast<Game::WeaponId>(std::stoi(match[1])), std::stoi(match[2]));
	else
		cout << "Usage: weaponCapacity [id] [value]" << endl;
}

void setUnlimitedMagazine(Game &game, std::string_view args)
{
	static std::regex rgx("[ ]?([[:digit:]]+) (true|false|0|1)");
	std::cmatch match;

	if (std::regex_match(args.data(), match, rgx)) {
		auto toggle = match[2];
		game.toggleUnlimitedMagazine(static_cast<Game::WeaponId>(std::stoi(match[1])), toggle == "true" || toggle == "1" ? true : false);
	}
	else
		cout << "Usage: unlimitedMag [Weapon ID] [true|false]" << endl;
}

//void unlimitedAmmo(Game &game, std::string_view args)
//{
//	static std::regex rgx("[ ]?([[:digit:]]+) (true|false|0|1)");
//	std::cmatch match;
//
//	if (std::regex_match(args.data(), match, rgx)) {
//		auto toggle = match[2];
//		game.toggleUnlimitedAmmo(static_cast<Game::WeaponId>(std::stoi(match[1])), toggle == "true" || toggle == "1" ? true : false);
//	}
//	else
//		cout << "Usage: unlimitedAmmo [Weapon ID] [true|false|0|1]" << endl;
//}

void toggleCapacityCheck(Game &inv, std::string_view args)
{
	std::cmatch match;

	if (std::regex_match(args.data(), match, booleanRegex))
	{
		if (match[1] == "true" || match[1] == "1")
			inv.toggleItemCapacityCheck(true);
		else
			inv.toggleItemCapacityCheck(false);
	}
	else
		cout << "Usage: capacityCheck [true|false|0|1]" << endl;
}

void setItemCapacity(Game &inv, std::string_view args)
{
	std::cmatch match;

	if (std::regex_match(args.data(), match, singleDigitRegex))
	{
		inv.setUniversalItemCapacity(std::stoi(match[1]));
	}
	else
		cout << "Usage: itemCapacity [value]\nif value is less than or equal to zero, default capacities are used" << endl;
}

void health(Game &inv, std::string_view args)
{
	std::cmatch match;

	if (std::regex_match(args.data(), match, singleDigitRegex))
		inv.setHealth(std::stoi(match[1]));
	else
		cout << inv.getHealth() << endl;
}

void maxHealth(Game &game, std::string_view args)
{
	std::cmatch match;

	if (std::regex_match(args.data(), match, singleDigitRegex))
		game.setMaxHealth(std::stoul(match[1]));
	else
		cout << game.getMaxHealth() << endl;
}

void whereAmI(Game &game, std::string_view args)
{
	if (auto coords = game.getCoords())
		cout << "X: " << coords->mX << " Y: " << coords->mY << " Z: " << coords->mZ << endl;
	else
		cout << "Nowhere" << endl;
}

void goTo(Game &game, std::string_view args)
{
	static std::regex coordinateRegex("[ ]?([+-]?[0-9]*[.]?[0-9]+) ([+-]?[0-9]*[.]?[0-9]+) ([+-]?[0-9]*[.]?[0-9]+)");
	std::cmatch match;
	
	if (std::regex_match(args.data(), match, coordinateRegex))
	{
		if (auto *coords = game.getCoords()) {
			coords->mX = std::stof(match[1]);
			coords->mY = std::stof(match[2]);
			coords->mZ = std::stof(match[3]);
		}
		else
			cout << "Can't go there" << endl;
	}
	else
		cout << "Usage: goto [x] [y] [z]" << endl;
}

void clipping(Game &game, std::string_view args)
{
	std::cmatch match;

	if (std::regex_match(args.data(), match, booleanRegex))
	{
		if (match[1] == "true" || match[1] == "1")
			game.toggleClipping(true);
		else
			game.toggleClipping(false);
	}
	else
		cout << "Usage: clipping [true|false|0|1]" << endl;
}

//void tableEntries(Game &game, std::string_view args)
//{
//	for (auto entry : game.getWeaponInfoTableEntries())
//	{
//		wcout << (void*)entry << L": " << game.getWeaponName(getValue<Game::WeaponId>(entry + 0x10)) << endl;
//	}
//}

void getDamageInfo(Game &game, std::string_view args)
{
	static std::regex rgx("[ ]?([[:digit:]]+) ([[:digit:]]+)");
	std::cmatch match;

	if (std::regex_match(args.data(), match, rgx))
	{
		if (auto info = game.getDamageInfo(static_cast<Game::DamageType>(std::stoi(match[1])), std::stoi(match[2])))
		{
			cout << "Base Damage: " << info->mBaseDamage << "\nLimb Damage: " << info->mLimbDamage << "\nForce: " << info->mForce << endl;
		}
	}
	else
		cout << "Usage: getDamageInfo [damage type] [sub damage type]" << endl;
}

void setDamageInfo(Game &game, std::string_view args)
{
	static std::regex rgx("[ ]?([[:digit:]]+) ([[:digit:]]+)(?:[ ]baseDamage:([[:digit:]]+)*)?(?:[ ]limbDamage:([[:digit:]]+)*)?(?:[ ]force:([[:digit:]]+)*)?");
	std::cmatch match;

	if (std::regex_match(args.data(), match, rgx))
	{
		if (auto info = game.getDamageInfo(static_cast<Game::DamageType>(std::stoi(match[1])), std::stoi(match[2])))
		{
			if (match[3].matched)
				info->mBaseDamage = std::stoi(match[3]);

			if (match[4].matched)
				info->mLimbDamage = std::stoi(match[4]);

			if (match[5].matched)
				info->mForce = std::stof(match[5]);
		}
	}
	else
		cout << "Usage: setDamageInfo [damage type] [sub damage type] baseDamage:[value] limbDamage:[value] force:[value]" << endl;
}

DWORD WINAPI ConsoleMain(LPVOID lpParameter)
{
	AllocConsole();
	RedirectSTDIO();
	HWND consoleWindow = GetConsoleWindow();

	try {
		std::string command, args;
		std::ifstream commandFile("commands.txt");
		CommandHandler handler;
		int firstChar;
		Game inv;

		handler.addHandler("itemName", std::bind(itemName, std::ref(inv), std::placeholders::_1));
		handler.addHandler("weaponName", std::bind(weaponName, std::ref(inv), std::placeholders::_1));
		handler.addHandler("get", std::bind(getItemAt, std::ref(inv), std::placeholders::_1));
		handler.addHandler("set", std::bind(setItemAt, std::ref(inv), std::placeholders::_1));
		handler.addHandler("clear", clear);
		handler.addHandler("setTime", std::bind(setTime, std::ref(inv), std::placeholders::_1));
		handler.addHandler("setSaveCounter", std::bind(setSaveCounter, std::ref(inv), std::placeholders::_1));
		handler.addHandler("inventorySize", std::bind(setInventorySize, std::ref(inv), std::placeholders::_1));
		handler.addHandler("weaponCapacity", std::bind(setWeaponCapacity, std::ref(inv), std::placeholders::_1));
		handler.addHandler("unlimitedMagazine", std::bind(setUnlimitedMagazine, std::ref(inv), std::placeholders::_1));
		//handler.addHandler("unlimitedAmmo", std::bind(unlimitedAmmo, std::ref(inv), std::placeholders::_1));
		handler.addHandler("capacityCheck", std::bind(toggleCapacityCheck, std::ref(inv), std::placeholders::_1));
		handler.addHandler("itemCapacity", std::bind(setItemCapacity, std::ref(inv), std::placeholders::_1));
		handler.addHandler("health", std::bind(health, std::ref(inv), std::placeholders::_1));
		handler.addHandler("maxHealth", std::bind(maxHealth, std::ref(inv), std::placeholders::_1));
		handler.addHandler("weaponId", std::bind(weaponId, std::ref(inv), std::placeholders::_1));
		handler.addHandler("itemId", std::bind(itemId, std::ref(inv), std::placeholders::_1));
		handler.addHandler("whereami", std::bind(whereAmI, std::ref(inv), std::placeholders::_1));
		handler.addHandler("goto", std::bind(goTo, std::ref(inv), std::placeholders::_1));
		handler.addHandler("clipping", std::bind(clipping, std::ref(inv), std::placeholders::_1));
		handler.addHandler("getDamageInfo", std::bind(getDamageInfo, std::ref(inv), std::placeholders::_1));
		handler.addHandler("setDamageInfo", std::bind(setDamageInfo, std::ref(inv), std::placeholders::_1));
		//handler.addHandler("entries", std::bind(tableEntries, std::ref(inv), std::placeholders::_1));

		if (commandFile.is_open())
		{
			while (commandFile >> command)
			{
				std::getline(commandFile, args);

				try {
					handler.callHandler(command, args);
				}
				catch (const CommandHandlerException &e) {
					cout << e.what() << endl;
				}
			}
			commandFile.close();
		}


		cout << ">> ";

		while ((firstChar = cin.get())
			   && (firstChar != '\n' ? (cin.unget(), cin >> command) : (command = firstChar, cin))
			   && command != "exit")
		{
			if (command.front() != '\n')
			{
				std::getline(cin, args);

				try {
					handler.callHandler(command, args);
				}
				catch (const CommandHandlerException &e) {
					cout << e.what() << endl;
				}
			}
			cout << ">> ";
		}
	}
	catch (const std::regex_error &e) {
		std::string error("Regex error: ");
		ErrorBox(consoleWindow, error + e.what());
	}
	catch (const std::runtime_error &e) {
		ErrorBox(consoleWindow, e.what());
	}

	FreeConsole();
	PostMessage(consoleWindow, WM_CLOSE, 0, 0);
	FreeLibraryAndExitThread((HMODULE)lpParameter, 0/*(DWORD)msg.wParam*/);
}