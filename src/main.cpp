#include <Windows.h>
#include <CommCtrl.h>
#include <vector>
#include <string>
#include <regex>
#include "Utility.h"
#include "Features.h"
#include "CommandHandler.h"

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
				if (!name.empty())
					wcout << i << ": " << name << endl;
			}
		}
	}
	else
		cout << "Usage: ItemName [id]" << endl;
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
				if(!name.empty())
					wcout << i << ": " << name << endl;
			}
		}
	}
	else
		cout << "Usage: WeaponName ([id] | *)" << endl;
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
				wcout << inv.getWeaponName(item->weaponId);
				if (item->ammoType != Game::ItemId::Invalid)
					wcout << ", " << item->ammo << ' ' << inv.getItemName(item->ammoType) << " rounds";
				wcout << endl;
			}
			else if (item->itemId != Game::ItemId::Invalid) { //if it's an item
				wcout << inv.getItemName(item->itemId) << " quantity: " << item->ammo << endl;
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

void toggleCapacityCheck(Game &inv, std::string_view args)
{
	static std::regex rgx("[ ]?(true|false|0|1)");
	std::cmatch match;

	if (std::regex_match(args.data(), match, rgx))
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

void setHealth(Game &inv, std::string_view args)
{
	std::cmatch match;

	if (std::regex_match(args.data(), match, singleDigitRegex))
		inv.setHealth(std::stoi(match[1]));
	else
		cout << inv.getHealth() << endl;
}

DWORD WINAPI ConsoleMain(LPVOID lpParameter)
{
	AllocConsole();
	RedirectSTDIO();
	HWND consoleWindow = GetConsoleWindow();

	try {
		std::string command, args;
		CommandHandler handler;
		Game inv;

		handler.addHandler("ItemName", std::bind(itemName, std::ref(inv), std::placeholders::_1));
		handler.addHandler("WeaponName", std::bind(weaponName, std::ref(inv), std::placeholders::_1));
		handler.addHandler("get", std::bind(getItemAt, std::ref(inv), std::placeholders::_1));
		handler.addHandler("set", std::bind(setItemAt, std::ref(inv), std::placeholders::_1));
		handler.addHandler("clear", clear);
		handler.addHandler("setTime", std::bind(setTime, std::ref(inv), std::placeholders::_1));
		handler.addHandler("setSaveCounter", std::bind(setSaveCounter, std::ref(inv), std::placeholders::_1));
		handler.addHandler("inventorySize", std::bind(setInventorySize, std::ref(inv), std::placeholders::_1));
		handler.addHandler("weaponCapacity", std::bind(setWeaponCapacity, std::ref(inv), std::placeholders::_1));
		handler.addHandler("capacityCheck", std::bind(toggleCapacityCheck, std::ref(inv), std::placeholders::_1));
		handler.addHandler("itemCapacity", std::bind(setItemCapacity, std::ref(inv), std::placeholders::_1));
		handler.addHandler("health", std::bind(setHealth, std::ref(inv), std::placeholders::_1));

		cout << "READY" << endl;
		cout << ">> ";

		while (cin >> command && command != "exit")
		{
			std::getline(cin, args);

			try {
				handler.callHandler(command, args);
			}
			catch (const CommandHandlerException &e) {
				cout << e.what() << endl;
			}
			cout << ">> ";
		}
	}
	catch (const std::runtime_error &e) {
		ErrorBox(consoleWindow, e.what());
	}

	FreeConsole();
	PostMessage(consoleWindow, WM_CLOSE, 0, 0);
	FreeLibraryAndExitThread((HMODULE)lpParameter, 0/*(DWORD)msg.wParam*/);
}