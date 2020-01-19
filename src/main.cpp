#include <Windows.h>
#include <CommCtrl.h>
#include <vector>
#include <string>
#include <Utility.h>
#include "Features.h"
#include "CommandHandler.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using std::cin;
using std::cout;
using std::wcout;
using std::endl;

DWORD WINAPI ConsoleProc(LPVOID);
DWORD WINAPI Win32Main(LPVOID);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH: {
			if (HANDLE hThread = CreateThread(nullptr, 0, ConsoleProc, hModule, 0, nullptr)) {
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

void itemName(Inventory &inv, const std::cmatch &match)
{
	wcout << inv.getItemName(static_cast<Inventory::ItemId>(std::stoi(match[1]))) << endl;
	//cout << "itemName" << ' ' << match[1] << endl;
}

void weaponName(Inventory &inv, const std::cmatch &match)
{
	wcout << inv.getWeaponName(static_cast<Inventory::WeaponId>(std::stoi(match[1]))) << endl;
}

void clear(const std::cmatch &match)
{
	system("cls");
}

void getItemAt(Inventory &inv, const std::cmatch &match)
{
	auto item = inv.getItemAt(std::stoi(match[1]));
	
	if (item)
	{
		item;
		if (item->weaponId != Inventory::WeaponId::Invalid) { //if it's a gun
			wcout << inv.getWeaponName(item->weaponId);// ' ' << item->ammo << ' ' << inv.getItemName(item->ammoType) << " rounds" << endl;
			if (item->ammoType != Inventory::ItemId::Invalid)
				wcout << ' ' << item->ammo << ' ' << inv.getItemName(item->ammoType) << " rounds";
			wcout << endl;
		}
		else if (item->itemId != Inventory::ItemId::Invalid) { //if it's an item
			wcout << inv.getItemName(item->itemId) << " quantity: " << item->ammo << endl;
		}
	}
}

void inventoryPtr(Inventory &inv, const std::cmatch &match)
{
	cout << reinterpret_cast<void*>(inv.getInventoryAddress()) << endl;
}

//slot item weapon upgrades ammoType ammo
void setItemAt(Inventory &inv, const std::cmatch &match)
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
			item->itemId = Inventory::ItemId(std::stoi(match[2]));
			item->weaponId = Inventory::WeaponId::Invalid;
		}
		/*else
			item->itemId = Inventory::ItemId::Invalid;*/

		if (match[3].matched) {
			item->weaponId = Inventory::WeaponId(std::stoi(match[3]));
			item->itemId = Inventory::ItemId::Invalid;
		}
		/*else
			item->weaponId = Inventory::WeaponId::Invalid;*/

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

DWORD WINAPI ConsoleProc(LPVOID lpParameter)
{
	AllocConsole();
	RedirectSTDIO();

	std::string command, args;
	CommandHandler handler;
	Inventory inv;
	HWND consoleWindow;

	try {
		handler.addHandler("ItemName", "[[:space:]]?([[:digit:]]+)", std::bind(itemName, inv, std::placeholders::_1));
		handler.addHandler("WeaponName", "[[:space:]]?([[:digit:]]+)", std::bind(weaponName, inv, std::placeholders::_1));
		handler.addHandler("get", "[[:space:]]?([[:digit:]]+)", std::bind(getItemAt, inv, std::placeholders::_1));
		handler.addHandler("set", "[ ]?([[:digit:]]+) (?:itemId:([[:digit:]]+)*)?[ ]?(?:weaponId:([[:digit:]]+)*)?[ ]?(?:upgrades:([[:digit:]]+)*)?[ ]?(?:ammoType:([[:digit:]]+)*)?[ ]?(?:ammo:([[:digit:]]+)*)?", std::bind(setItemAt, inv, std::placeholders::_1));
		handler.addHandler("inventoryPtr", "", std::bind(inventoryPtr, inv, std::placeholders::_1));
		handler.addHandler("clear", "", clear);
	}
	catch (const CommandHandlerException &e) {
		cout << e.what() << endl;
	}

	cout << "OK!" << endl;

	while (cin >> command)
	{
		std::getline(cin, args);

		try {
			handler.callHandler(command, args);
		}
		catch (const CommandHandlerException &e) {
			cout << e.what() << endl;
		}
	}

	consoleWindow = GetConsoleWindow();
	FreeConsole();
	PostMessage(consoleWindow, WM_CLOSE, 0, 0);
	FreeLibraryAndExitThread((HMODULE)lpParameter, 0/*(DWORD)msg.wParam*/);
}