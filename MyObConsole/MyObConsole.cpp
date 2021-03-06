// MyObConsole.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include "stdio.h"
#include ".\..\MyObEx\MyObEx.h"
#pragma comment(lib,"MyObEx.lib")

// run to compile copy_lib_to_consoleApp.bat

//int main()
//{
//	HANDLE hFile;
//	WIN32_FIND_DATA ffd;
//	wchar_t wszPathQuery[] =	L"C:\\Users\\LINH\\Documents\\obfuscation\\sample\\expection\\*";
//	wchar_t wszPath[] =			L"C:\\Users\\LINH\\Documents\\obfuscation\\sample\\expection\\";
//	wchar_t wszFileName[MAXSIZE];
//	wchar_t wszNewFileName[MAXSIZE];
//	
//	hFile = FindFirstFile(wszPathQuery, &ffd);
//	if (hFile == INVALID_HANDLE_VALUE)
//	{
//		printf("FindFirstFile error!\n");
//		return 1;
//	}
//	wprintf(L"%s -- skip\n", ffd.cFileName);
//	FindNextFile(hFile, &ffd);
//	wprintf(L"%s -- skip\n", ffd.cFileName);
//	FindNextFile(hFile, &ffd);
//	do
//	{
//		wprintf(L"%s\n", ffd.cFileName);
//		memset(wszFileName, 0, MAXSIZE * 2);
//		wcscat_s(wszFileName, MAXSIZE, wszPath);
//		wcscat_s(wszFileName,MAXSIZE,ffd.cFileName);
//
//		wcscpy_s(wszNewFileName, MAXSIZE, wszFileName);
//		wcscat_s(wszNewFileName, MAXSIZE, L"_obf");
//		CopyFile(wszFileName, wszNewFileName, true);
//
//		if (Init(wszNewFileName) == INIT_ERROR)
//		{
//			printf("[-]Init fail!\n");
//			return 1;
//		}
//		else
//		{
//			printf("[+]Init success!\n");
//		}
//		AddDummyDataToEmptySpace();
//		Unload();
//
//	} while (FindNextFile(hFile, &ffd) != NULL);
//    printf("hello, world!\n"); 
//	return 1;
//}

//int main(int argc, char* argv[])
//{
//	HANDLE hFile;
//	WIN32_FIND_DATA ffd;
//	wchar_t wszPathQuery[] =	L"C:\\Users\\linhv\\PROJECTS\\Obfuscation\\Code\\MyObEx\\Debug\\test\\*";
//	
//	// Directory of the target 
//	wchar_t wszPath[] =			L"C:\\Users\\linhv\\PROJECTS\\Obfuscation\\Code\\MyObEx\\Debug\\test\\";
//	wchar_t wszFileName[MAXSIZE];
//	hFile = FindFirstFile(wszPathQuery, &ffd);
//	if (hFile == INVALID_HANDLE_VALUE)
//	{
//		printf("FindFirstFile error!\n");
//		return 1;
//	}
//	wprintf(L"%s -- skip\n", ffd.cFileName);
//	FindNextFile(hFile, &ffd);
//	wprintf(L"%s -- skip\n", ffd.cFileName);
//	FindNextFile(hFile, &ffd);
//	do
//	{
//		wprintf(L"%s\n", ffd.cFileName);
//		memset(wszFileName, 0, MAXSIZE * 2);
//		wcscat_s(wszFileName, MAXSIZE, wszPath);
//		wcscat_s(wszFileName,MAXSIZE,ffd.cFileName);
//
//		if (Init(wszFileName) == INIT_ERROR)
//		{
//			printf("[-]Init fail!\n");
//			return 1;
//		}
//		else
//		{
//			printf("[+]Init success!\n");
//			
//			// 
//			Encrypt();
//			//AddDummyDataToEmptySpace();
//		}
//		Unload();
//
//	} while (FindNextFile(hFile, &ffd) != NULL);
//    printf("SUCESSFUL\n"); 
//	return 1;
//}


bool bChoseFile(LPTCH *lpFileName)
{
    OPENFILENAME ofn;       // common dialog box structure
    
    //TCHAR szFile[260];       // buffer for file name
    HWND hwnd = NULL;              // owner window
    HANDLE hf;              // file handle

    LPTCH buffer = (LPTCH)malloc(sizeof(TCHAR) * MAX_PATH);

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = buffer;
    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
    // use the contents of szFile to initialize itself.
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(TCHAR) * MAX_PATH;
    ofn.lpstrFilter = _TEXT("All\0*.*\0Text\0*.TXT\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // Display the Open dialog box. 

    if (GetOpenFileName(&ofn) == TRUE)
        *lpFileName = buffer;
    else
        return false;
    return true;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    LPTCH lpFileName = NULL;
    if (!bChoseFile(&lpFileName))
        return 1;
    MessageBox(NULL, lpFileName, _TEXT("Simple Ob"), MB_OK);
    return 0;
}

