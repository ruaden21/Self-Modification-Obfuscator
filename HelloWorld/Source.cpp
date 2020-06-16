#include "windows.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	MessageBoxA(NULL, "hello world!", "MyApplication", MB_OK);
	return 0;
}