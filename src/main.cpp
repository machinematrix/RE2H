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

void itemName(Inventory &inv, std::string_view args)
{
	static std::regex rgx("[ ]?([[:digit:]]+)");
	std::cmatch match;

	if (std::regex_match(args.data(), match, rgx)) {
		wcout << inv.getItemName(static_cast<Inventory::ItemId>(std::stoi(match[1]))) << endl;
	}
	else
		cout << "Usage: itemName [id]" << endl;
}

void weaponName(Inventory &inv, std::string_view args)
{
	static std::regex rgx("[ ]?([[:digit:]]+)");
	std::cmatch match;

	if (std::regex_match(args.data(), match, rgx)) {
		wcout << inv.getWeaponName(static_cast<Inventory::WeaponId>(std::stoi(match[1]))) << endl;
	}
	else
		cout << "Usage: weaponName [id]" << endl;
}

void clear(std::string_view args)
{
	system("cls");
}

void getItemAt(Inventory &inv, std::string_view args)
{
	static std::regex rgx("[ ]?([[:digit:]]+)");
	std::cmatch match;

	if (std::regex_match(args.data(), match, rgx))
	{
		auto item = inv.getItemAt(std::stoi(match[1]));

		if (item)
		{
			item;
			if (item->weaponId != Inventory::WeaponId::Invalid) { //if it's a gun
				wcout << inv.getWeaponName(item->weaponId);
				if (item->ammoType != Inventory::ItemId::Invalid)
					wcout << ", " << item->ammo << ' ' << inv.getItemName(item->ammoType) << " rounds";
				wcout << endl;
			}
			else if (item->itemId != Inventory::ItemId::Invalid) { //if it's an item
				wcout << inv.getItemName(item->itemId) << " quantity: " << item->ammo << endl;
			}
		}
	}
	else
		cout << "Usage: get [id]" << endl;
}

//slot item weapon upgrades ammoType ammo
void setItemAt(Inventory &inv, std::string_view args)
{
	static std::regex rgx("[ ]?([[:digit:]]+) (?:itemId:([[:digit:]]+)|weaponId:([[:digit:]]+))?[ ]?(?:upgrades:([[:digit:]]+)*)?[ ]?(?:ammoType:([[:digit:]]+)*)?[ ]?(?:ammo:([[:digit:]]+)*)?");
	std::cmatch match;

	if (std::regex_match(args.data(), match, rgx))
	{
		cout << "Slot: " << match[1] << endl;
		cout << "itemId: " << match[2] << endl;
		cout << "weaponId: " << match[3] << endl;
		cout << "upgrades: " << match[4] << endl;
		cout << "ammoType: " << match[5] << endl;
		cout << "ammo: " << match[6] << endl;
		auto item = inv.getItemAt(std::stoi(match[1]));

		if (item)
		{
			if (match[2].matched) {
				item->itemId = Inventory::ItemId(std::stoi(match[2]));
				item->weaponId = Inventory::WeaponId::Invalid;
			}
			else if (match[3].matched) {
				item->weaponId = Inventory::WeaponId(std::stoi(match[3]));
				item->itemId = Inventory::ItemId::Invalid;
			}

			if (match[4].matched)
				item->upgrades = std::stoi(match[4]);

			if (match[5].matched)
				item->ammoType = Inventory::ItemId(std::stoi(match[5]));

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

void setTime(Stats &stats, std::string_view args)
{
	static std::regex rgx("[ ]?([[:digit:]]{2}):([[:digit:]]{2}):([[:digit:]]{2})");
	std::cmatch match;

	if (std::regex_match(args.data(), match, rgx))
		stats.setPlayedTime(std::stoul(match[1]), std::stoul(match[2]), std::stoul(match[3]));
	else
		cout << "Usage: setTime [hh:mm:ss]" << endl;
}

void setSaveCounter(Stats &stats, std::string_view args)
{
	static std::regex rgx("[ ]?([[:digit:]]+)");
	std::cmatch match;

	if (std::regex_match(args.data(), match, rgx))
		stats.setSaveCount(std::stoul(match[1]));
	else
		cout << "Usage: setSaveCounter [count]" << endl;
}

DWORD WINAPI ConsoleMain(LPVOID lpParameter)
{
	AllocConsole();
	RedirectSTDIO();

	std::string command, args;
	CommandHandler handler;
	Inventory inv;
	Stats stats;
	HWND consoleWindow;

	handler.addHandler("ItemName", /*"[[:space:]]?([[:digit:]]+)",*/ std::bind(itemName, std::ref(inv), std::placeholders::_1));
	handler.addHandler("WeaponName", /*"[[:space:]]?([[:digit:]]+)",*/ std::bind(weaponName, std::ref(inv), std::placeholders::_1));
	handler.addHandler("get", /*"[[:space:]]?([[:digit:]]+)",*/ std::bind(getItemAt, std::ref(inv), std::placeholders::_1));
	handler.addHandler("set", /*"[ ]?([[:digit:]]+) (?:itemId:([[:digit:]]+)*)?[ ]?(?:weaponId:([[:digit:]]+)*)?[ ]?(?:upgrades:([[:digit:]]+)*)?[ ]?(?:ammoType:([[:digit:]]+)*)?[ ]?(?:ammo:([[:digit:]]+)*)?",*/ std::bind(setItemAt, inv, std::placeholders::_1));
	handler.addHandler("clear", /*"",*/ clear);
	handler.addHandler("setTime", std::bind(setTime, std::ref(stats), std::placeholders::_1));
	handler.addHandler("setSaveCounter", std::bind(setSaveCounter, std::ref(stats), std::placeholders::_1));

	cout << "READY" << endl;

	while (cin >> command)
	{
		std::getline(cin, args);

		try {
			handler.callHandler(command, args);
		}
		catch (const CommandHandlerException & e) {
			cout << e.what() << endl;
		}
	}

	consoleWindow = GetConsoleWindow();
	FreeConsole();
	PostMessage(consoleWindow, WM_CLOSE, 0, 0);
	FreeLibraryAndExitThread((HMODULE)lpParameter, 0/*(DWORD)msg.wParam*/);
}