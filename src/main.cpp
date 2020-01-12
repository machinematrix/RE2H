#include <Windows.h>
#include <CommCtrl.h>
#include <vector>
#include <string>
#include <tuple>
#include <Utility.h>
#include "Features.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

DWORD WINAPI ThreadProc(LPVOID);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH: {
			if (HANDLE hThread = CreateThread(nullptr, 0, ThreadProc, hModule, 0, nullptr)) {
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

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	using std::cin;
	using std::cout;
	using std::endl;
	AllocConsole();
	RedirectSTDIO();
	std::string command;
	Inventory inv;

	while (cin >> command)
	{
		if (command == "inventoryPtr") {
			cout << inv.getInventoryAddress() << endl;
		}
	}

	cout << "Bye" << endl;

	FreeConsole();
	FreeLibraryAndExitThread((HMODULE)lpParameter, 0/*(DWORD)msg.wParam*/);
}