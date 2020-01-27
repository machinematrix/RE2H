#ifndef __UTILITY__
#define __UTILITY__
#include <windows.h>
#include <string>
#include <string_view>
#include <iostream>
#include <sstream>

typedef POINT Dimension;
typedef std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR>> String;

void RedirectSTDIO();
void ErrorBox(HWND hWnd, std::wstring_view text);
void ErrorBox(HWND hWnd, std::string_view text);
String GetControlText(HWND hWnd);
Dimension GetWindowDimensions(HWND hWnd);
RECT GetWindowRectInParent(HWND hWnd);
bool CALLBACK SetFont(HWND child, LPARAM font);

#endif