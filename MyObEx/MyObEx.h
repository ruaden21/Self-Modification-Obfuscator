#ifndef STDAFX.H
#include "stdafx.h"
#endif // !STDAFX.H


#define MAXSIZE 256

#define PE32		0x010B
#define	PE64		0x020B

#define IMAGE_SIZEOF_RESOURCE_DIRECTORY 0x10


#define NO_ERROR	100
#define ACTIVE		101
#define	DUMMY_ALREALY	102

#define INIT_ERROR	201
#define NOT_ACTIVE	202
#define ENCRPYT_ERROR	203
#define CHECK_VER_ERROR	204
#define MY_ERROR	205

extern "C" __declspec(dllexport) int Encrypt();
extern "C" __declspec(dllexport) int Init(wchar_t* wcFileName);
extern "C" __declspec(dllexport) int Unload();
extern "C" __declspec(dllexport) int AddDummyData(BYTE buffer[], DWORD dwSize);
extern "C" __declspec(dllexport) int ListAllEmptySpace(wchar_t *name_or_id);
extern "C" __declspec(dllexport) int AddDummyDataToEmptySpace();
extern "C" __declspec(dllexport) int ListAllResource();

int Encrypt32();
int Encrypt64();
VOID ListResRescusive(_In_ LPVOID pOrinialResourceBase,_In_ LPVOID pCurrentResourceBase, WORD wLevel);
WORD GetSectionOrder(_In_ DWORD dwRelativeVirtualAddress);
VOID WcharToCharAndDoubleForwardSlash(_In_ wchar_t* wCharString,_Out_ char* CharString, _In_ DWORD dwSize);

