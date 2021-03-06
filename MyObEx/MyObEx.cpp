// MyObEx.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "MyObEx.h"

wchar_t filename[MAXSIZE];
LPVOID	pExeBase;
HANDLE	hFile;
HANDLE	hMapping;
DWORD	bOnload;
WORD	wMagic;
PIMAGE_DOS_HEADER	pDosHeader;
ULONG_PTR pNtHeader;

// Relative to section
WORD wNumberberOfSections;
PIMAGE_SECTION_HEADER pSectionHeader;

VOID WcharToCharAndDoubleForwardSlash(_In_ wchar_t* wCharString, _Out_ char* CharString, _In_ DWORD dwMaxSize)
{
	DWORD i = 0, j = 0;
	while (i < dwMaxSize && j < dwMaxSize)
	{
		if (wCharString[i] == NULL)
		{
			break;
		}
		else
		{
			if (wCharString[i] == L'\\')
			{
				CharString[j] = (char)((WORD)wCharString[i] & 0xff);
				j++;
			}
			CharString[j] = (char)((WORD)wCharString[i] & 0xff);
		}
		i++;
		j++;
	}
}

extern "C" __declspec(dllexport) int Init(wchar_t* wcFileName)
{
	lstrcpynW(filename, wcFileName, MAXSIZE);

	hFile = CreateFileW(filename,
		GENERIC_ALL,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return INIT_ERROR;
	}

	hMapping = CreateFileMappingW(hFile, NULL, 0x4, 0, 0, 0);
	if (hMapping == INVALID_HANDLE_VALUE)
	{
		return INIT_ERROR;
	}

	pExeBase = MapViewOfFile(hMapping, 0xf001f, 0, 0, 0);
	if (!pExeBase)
	{
		return INIT_ERROR;
	}

	pDosHeader = (PIMAGE_DOS_HEADER)pExeBase;
	pNtHeader = ((ULONGLONG)pExeBase + (pDosHeader->e_lfanew));
	wMagic = *(WORD*)(pNtHeader + 0x18);
	if (wMagic == PE32)
	{
		pSectionHeader = IMAGE_FIRST_SECTION((PIMAGE_NT_HEADERS32)pNtHeader);
		wNumberberOfSections = ((PIMAGE_NT_HEADERS32)pNtHeader)->FileHeader.NumberOfSections;
	}
	else
	{
		pSectionHeader = IMAGE_FIRST_SECTION((PIMAGE_NT_HEADERS64)pNtHeader);
		wNumberberOfSections = ((PIMAGE_NT_HEADERS64)pNtHeader)->FileHeader.NumberOfSections;
	}
	bOnload = ACTIVE;
	return NO_ERROR;
}
extern "C" __declspec(dllexport) int Unload()
{
	if (bOnload != ACTIVE)
	{
		return NOT_ACTIVE;
	}

	CloseHandle(hMapping);
	CloseHandle(hFile);
	return NO_ERROR;
}

extern "C" __declspec(dllexport) int Encrypt()
{
	if (bOnload != ACTIVE)
	{
		return NOT_ACTIVE;
	}

	
	if (wMagic == PE32)
	{
		Encrypt32();
	}
	else
	{
		if (wMagic == PE64)
		{
			Encrypt64();
		}
		else
		{
			return MY_ERROR;
		}
	}
	return NO_ERROR;
}

extern "C" __declspec(dllexport) int AddDummyData(BYTE buffer[], DWORD dwSize)
{
	PIMAGE_SECTION_HEADER	pPrevSectionHeader;
	PIMAGE_SECTION_HEADER	pCurrSectionHeader;
	IMAGE_SECTION_HEADER	DummySectionHeader;
	PIMAGE_NT_HEADERS		pNtHeader32 = 0;
	PIMAGE_NT_HEADERS64		pNtHeader64 = 0;
	LPVOID					pRawData;



	if (bOnload != ACTIVE)
	{
		return NOT_ACTIVE;
	}

	pDosHeader = (PIMAGE_DOS_HEADER)pExeBase;

	if (wMagic == PE32)
	{
		pNtHeader32 = (PIMAGE_NT_HEADERS)((ULONGLONG)pExeBase + (pDosHeader->e_lfanew));
		pCurrSectionHeader = IMAGE_FIRST_SECTION(pNtHeader32);
	}
	else
	{
		if (wMagic == PE64)
		{
			pNtHeader64 = (PIMAGE_NT_HEADERS64)((ULONGLONG)pExeBase + (pDosHeader->e_lfanew));
			pCurrSectionHeader = IMAGE_FIRST_SECTION(pNtHeader64);
		}
		else
		{
			return MY_ERROR;
		}
	}

	pPrevSectionHeader = pCurrSectionHeader;
	while (pCurrSectionHeader->VirtualAddress != 0x00)
	{
		pPrevSectionHeader = pCurrSectionHeader;
		pCurrSectionHeader = (PIMAGE_SECTION_HEADER)((ULONGLONG)pCurrSectionHeader + IMAGE_SIZEOF_SECTION_HEADER);
	}



	DummySectionHeader.Name[0] = 'L';
	DummySectionHeader.Name[1] = 'I';
	DummySectionHeader.Name[2] = 'N';
	DummySectionHeader.Name[3] = 'H';
	DummySectionHeader.Name[4] = 0;
	DummySectionHeader.Name[5] = 0;
	DummySectionHeader.Name[6] = 0;
	DummySectionHeader.Name[7] = 0;
	DummySectionHeader.Misc.VirtualSize = dwSize;
	DummySectionHeader.VirtualAddress = pPrevSectionHeader->VirtualAddress + pPrevSectionHeader->SizeOfRawData;
	DummySectionHeader.SizeOfRawData = (dwSize & 0xfffff000) + (((dwSize & 0x00000fff) != 0) * 0x1000);
	DummySectionHeader.PointerToRawData = pPrevSectionHeader->PointerToRawData + pPrevSectionHeader->SizeOfRawData;
	DummySectionHeader.PointerToRelocations = 0x00000000;
	DummySectionHeader.PointerToLinenumbers = 0x00000000;
	DummySectionHeader.NumberOfRelocations = 0x0000;
	DummySectionHeader.NumberOfLinenumbers = 0x0000;
	DummySectionHeader.Characteristics = 0x60000020;

	if (memcmp((char *)pPrevSectionHeader, (char *)&DummySectionHeader, 7) == 0)
	{
		return DUMMY_ALREALY;
	}
	if (wMagic == PE32)
	{
		pNtHeader32->FileHeader.NumberOfSections += 1;
	}
	else
		pNtHeader64->FileHeader.NumberOfSections += 1;
	memcpy((char *)pCurrSectionHeader, (char *)&DummySectionHeader, IMAGE_SIZEOF_SECTION_HEADER);
	SetFilePointer(hFile, pCurrSectionHeader->PointerToRawData, NULL, FILE_BEGIN);
	DWORD n;
	WriteFile(hFile, buffer, dwSize, &n, NULL);
	return NO_ERROR;
}



extern "C" __declspec(dllexport) int ListAllEmptySpace(wchar_t *name_or_id)
{
	PIMAGE_DOS_HEADER		pDosHeader;
	PIMAGE_SECTION_HEADER	pCurrSectionHeader;
	IMAGE_SECTION_HEADER	DummySectionHeader;
	PIMAGE_NT_HEADERS		pNtHeader32 = 0;
	PIMAGE_NT_HEADERS64		pNtHeader64 = 0;
	LPVOID					pRawData;
	FILE *					pfOut;
	DWORD					dwTotalEmptySize = 0;
	if (bOnload != ACTIVE)
	{
		return NOT_ACTIVE;
	}
	pDosHeader = (PIMAGE_DOS_HEADER)pExeBase;

	if (wMagic == PE32)
	{
		pNtHeader32 = (PIMAGE_NT_HEADERS)((ULONGLONG)pExeBase + (pDosHeader->e_lfanew));
		pCurrSectionHeader = IMAGE_FIRST_SECTION(pNtHeader32);
	}
	else
	{
		if (wMagic == PE64)
		{
			pNtHeader64 = (PIMAGE_NT_HEADERS64)((ULONGLONG)pExeBase + (pDosHeader->e_lfanew));
			pCurrSectionHeader = IMAGE_FIRST_SECTION(pNtHeader64);
		}
		else
		{
			return MY_ERROR;
		}
	}

	fopen_s(&pfOut, "out_size.json", "a");
	char szfilename[MAXSIZE] = { 0 };
	char szSectionName[9] = {0};
	WcharToCharAndDoubleForwardSlash(name_or_id, szfilename, MAXSIZE);
	
	// "filename":
	//fprintf(pfOut, "\"%s\":{", szfilename);
	
	

	int index = 0;
	while (1)
	{
		memcpy_s(szSectionName, 8,(char*)pCurrSectionHeader->Name,8);
		
		// {"section name":
		// fprintf(pfOut, "\"%s\":{", szSectionName);
		{
			// {"total":"0xabcd",
			// fprintf(pfOut, "\"total\":\"0x%x\",", pCurrSectionHeader->SizeOfRawData);
			
			// "used":"0xabcd",
			// fprintf(pfOut, "\"used\":\"0x%x\",", pCurrSectionHeader->Misc.VirtualSize);
			
			DWORD dwAvailable = pCurrSectionHeader->SizeOfRawData - pCurrSectionHeader->Misc.VirtualSize;


			if (pCurrSectionHeader->SizeOfRawData < pCurrSectionHeader->Misc.VirtualSize)
				dwAvailable = 0;
			dwTotalEmptySize += dwAvailable;

			// "available":"0xabcd"}
			// fprintf(pfOut, "\"available\":\"0x%x\"", dwAvailable);
		}
		pCurrSectionHeader = (PIMAGE_SECTION_HEADER)((ULONGLONG)pCurrSectionHeader + IMAGE_SIZEOF_SECTION_HEADER);
		index++;
		if (index < wNumberberOfSections)
			//fprintf(pfOut, "},");
			continue;
		else
		{
			//fprintf(pfOut, "}");
			break;
		}
	}

	// get empty size of each file
	fprintf(pfOut, "\"%s\":%d,", szfilename, dwTotalEmptySize);

	// },
	//fprintf(pfOut, "},", szfilename);
	fclose(pfOut);
}

WORD GetSectionOrder(DWORD dwRelativeVirtualAddress)
{
	PIMAGE_SECTION_HEADER pCurrentSectionHeader;
	ULONG_PTR pCurrentAddress;
	for (WORD i = 0; i < wNumberberOfSections; i++)
	{
		if (dwRelativeVirtualAddress >= pSectionHeader[i].VirtualAddress
			&& dwRelativeVirtualAddress < (pSectionHeader[i].VirtualAddress + (DWORD)(pSectionHeader[i].Misc.VirtualSize)))
		{
			return i;
			break;
		}
	}
	return 0xffff;
}


extern "C" __declspec(dllexport) int AddDummyDataToEmptySpace()
{
	PIMAGE_DOS_HEADER		pDosHeader;
	PIMAGE_SECTION_HEADER	pCurrSectionHeader;

	PIMAGE_NT_HEADERS32		pNtHeader32 = 0;
	PIMAGE_NT_HEADERS64		pNtHeader64 = 0;
	LPVOID					pRawData;
	FILE *					pfOut;

	if (bOnload != ACTIVE)
	{
		return NOT_ACTIVE;
	}
	pDosHeader = (PIMAGE_DOS_HEADER)pExeBase;

	if (wMagic == PE32)
	{
		pNtHeader32 = (PIMAGE_NT_HEADERS32)((ULONGLONG)pExeBase + (pDosHeader->e_lfanew));
		pCurrSectionHeader = IMAGE_FIRST_SECTION(pNtHeader32);
	}
	else
	{
		if (wMagic == PE64)
		{
			pNtHeader64 = (PIMAGE_NT_HEADERS64)((ULONGLONG)pExeBase + (pDosHeader->e_lfanew));
			wNumberberOfSections = pNtHeader64->FileHeader.NumberOfSections;
			pCurrSectionHeader = IMAGE_FIRST_SECTION(pNtHeader64);
		}
		else
		{
			return MY_ERROR;
		}
	}

	char szfilename[MAXSIZE] = { 0 };
	srand(0);
	WORD wOrderOfSections = 0;
	while (wOrderOfSections < wNumberberOfSections)
	{
		wOrderOfSections++;
		DWORD dwAvailable = 0;
		dwAvailable = pCurrSectionHeader->SizeOfRawData - pCurrSectionHeader->Misc.VirtualSize;
		if (pCurrSectionHeader->SizeOfRawData <= pCurrSectionHeader->Misc.VirtualSize || dwAvailable <= 0x13)
		{
			dwAvailable = 0;
			pCurrSectionHeader = (PIMAGE_SECTION_HEADER)((ULONGLONG)pCurrSectionHeader + IMAGE_SIZEOF_SECTION_HEADER);
			continue;
		}

		DWORD *tmp = (DWORD*)((ULONGLONG)pExeBase + pCurrSectionHeader->PointerToRawData + pCurrSectionHeader->Misc.VirtualSize + 0xf); //skip a 0xf byte
		DWORD times = 0;

		// 
		times = (dwAvailable-0xf) / 4;
		if (times == 0)
			continue;
		for (int j = 0; j < times; j++)
		{
			DWORD data = (0x12345678 * rand()) & 0xffffffff ;
			tmp[j] = data;
		}
		pCurrSectionHeader->Misc.VirtualSize = pCurrSectionHeader->Misc.VirtualSize + dwAvailable;
		pCurrSectionHeader = (PIMAGE_SECTION_HEADER)((ULONGLONG)pCurrSectionHeader + IMAGE_SIZEOF_SECTION_HEADER);
	}
}

extern "C" __declspec(dllexport) int ListAllResource()
{
	PIMAGE_DOS_HEADER		pDosHeader;

	PIMAGE_NT_HEADERS32		pNtHeader32 = 0;
	PIMAGE_NT_HEADERS64		pNtHeader64 = 0;
	PIMAGE_DATA_DIRECTORY	pDataDirectory;
	LPVOID					pRawData;
	FILE *					pfOut;

	if (bOnload != ACTIVE)
	{
		return NOT_ACTIVE;
	}

	if (wMagic == PE32)
	{
		pDataDirectory = ((PIMAGE_NT_HEADERS32)pNtHeader)->OptionalHeader.DataDirectory;
	}
	else
	{
		pDataDirectory = ((PIMAGE_NT_HEADERS64)pNtHeader)->OptionalHeader.DataDirectory;
	}
	if (pDataDirectory[2].VirtualAddress == 0)
		return NO_ERROR;

	WORD order = GetSectionOrder(pDataDirectory[2].VirtualAddress);
	if (order == 0xffff)
	{
		printf("invalid\n");
		return 1;
	}
	LPVOID ResourceBase = (LPVOID)((ULONGLONG)pExeBase + pSectionHeader[order].PointerToRawData);
	ListResRescusive(ResourceBase, ResourceBase, 0);
}

VOID ListResRescusive(LPVOID pOrinialResourceBase, LPVOID pCurrentResourceBase, WORD wLevel)
{
	PIMAGE_RESOURCE_DIRECTORY pRes = (PIMAGE_RESOURCE_DIRECTORY)pCurrentResourceBase;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pResEntry = NULL;
	PIMAGE_RESOURCE_DATA_ENTRY pResDataEntry;
	WORD wNumberOfEntries = pRes->NumberOfIdEntries;
	if (wNumberOfEntries == 0)
		return;
	pResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((ULONG_PTR)pRes + IMAGE_SIZEOF_RESOURCE_DIRECTORY);
	for (int i = 0; i < wNumberOfEntries; i++)
	{
		int j = 0;
		while (j < wLevel)
		{
			printf("\t");
			j++;
		}
		printf("Resource Directory Entry ID: %x\n", pResEntry[i].Name);
		if (pResEntry[i].OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY)
		{
			ListResRescusive(pOrinialResourceBase, (LPVOID)((ULONGLONG)pOrinialResourceBase + (ULONGLONG)(pResEntry[i].OffsetToData - IMAGE_RESOURCE_DATA_IS_DIRECTORY)), wLevel + 1);
		}
		else
		{
			pResDataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)((ULONGLONG)pOrinialResourceBase + (ULONGLONG)pResEntry[i].OffsetToData);
			int j = 0;
			while (j < wLevel)
			{
				printf("\t");
				j++;
			}
			printf("OffsetToData: %x\tSize:%x\n", pResDataEntry->OffsetToData, pResDataEntry->Size);
		}
	}
}


// error
int Encrypt64()
{
	char pShell[] = {
	0x90, 0x90, 0x90, 0x90, 0x48, 0x31, 0xC0, 0x48, 0x31, 0xDB, 
	0xE8, 0x00, 0x00, 0x00, 0x00, 0x58, 0x48, 0x25, 0x00, 0x00,
	0xF0, 0xFF, 0x48, 0xC7, 0xC3, 0xD0, 0x14, 0x00, 0x00, 0x48,
	0x09, 0xC3, 0xBE, 0x00, 0x00, 0x00, 0x00, 0xEB, 0x06, 0x89,
	0xF0, 0xFF, 0xC0, 0x89, 0xC6, 0x83, 0xFE, 0x0A, 0x7D, 0x26,
	0x48, 0x63, 0xC6, 0x48, 0x89, 0xD9, 0x48, 0x8B, 0x04, 0xC1, 
	0x48, 0xB9, 0xFE, 0xCA, 0xAF, 0xDE, 0xFE, 0xCA, 0xAF, 0xDE, 
	0x48, 0x31, 0xC8, 0x48, 0x89, 0xC2, 0x48, 0x63, 0xC6, 0x48, 
	0x89, 0xD9, 0x48, 0x89, 0x14, 0xC1, 0xEB, 0xCF, 0x53, 0xC3
	};

	PIMAGE_DOS_HEADER pDosHeader;
	PIMAGE_NT_HEADERS64 pNtHeader;
	PIMAGE_OPTIONAL_HEADER64 pNtOptionHeader;
	PIMAGE_FILE_HEADER pNtFileHeader;
	WORD wSizeOfOptionalHeader;
	DWORD dwAddressOfEntryPoint;
	DWORD dwAddressOfNewEntryPoint;
	PIMAGE_SECTION_HEADER pFirstSectionHeader;
	PIMAGE_SECTION_HEADER pCurrentSectionHeader;
	LPVOID pRawData;

	if (bOnload != ACTIVE)
	{
		return NOT_ACTIVE;
	}

	pDosHeader = (PIMAGE_DOS_HEADER)pExeBase;
	pNtHeader = (PIMAGE_NT_HEADERS64)((ULONGLONG)pExeBase + (pDosHeader->e_lfanew));
	pNtFileHeader = (PIMAGE_FILE_HEADER)(&(pNtHeader->FileHeader));
	pNtOptionHeader = (PIMAGE_OPTIONAL_HEADER64)(&(pNtHeader->OptionalHeader));
	dwAddressOfEntryPoint = pNtOptionHeader->AddressOfEntryPoint;
	wSizeOfOptionalHeader = pNtFileHeader->SizeOfOptionalHeader;

	pFirstSectionHeader = IMAGE_FIRST_SECTION(pNtHeader);
	pCurrentSectionHeader = pFirstSectionHeader;
	
	WORD wOrderOfSections = 0;
	while (wOrderOfSections < wNumberberOfSections)
	{
		wOrderOfSections++;
		if (dwAddressOfEntryPoint >= pCurrentSectionHeader->VirtualAddress
			&& dwAddressOfEntryPoint < (pCurrentSectionHeader->VirtualAddress + (DWORD)(pCurrentSectionHeader->Misc.VirtualSize)))
		{
			break;
		}
		pCurrentSectionHeader = (PIMAGE_SECTION_HEADER)((ULONGLONG)pCurrentSectionHeader + IMAGE_SIZEOF_SECTION_HEADER);
	}
	if (pCurrentSectionHeader->VirtualAddress == 0x00)
	{
		return false;
	}

	// Alter the flag of .code section
	pCurrentSectionHeader->Characteristics = pCurrentSectionHeader->Characteristics | 0x80000000;

	pRawData = (LPVOID)((ULONGLONG)pExeBase + dwAddressOfEntryPoint + pCurrentSectionHeader->PointerToRawData - pCurrentSectionHeader->VirtualAddress);
	for (int i = 0; i < 0x0a; i++)
	{
		ULONGLONG tmp;
		tmp = ((ULONGLONG*)pRawData)[i];
		tmp = tmp ^ 0xdeafcafedeafcafe;
		((ULONGLONG*)pRawData)[i] = tmp;
	}

	// Inject decrypt code
	dwAddressOfNewEntryPoint = pCurrentSectionHeader->VirtualAddress + pCurrentSectionHeader->Misc.VirtualSize;
	pNtOptionHeader->AddressOfEntryPoint = dwAddressOfNewEntryPoint;

	pRawData = (LPVOID)((ULONGLONG)pExeBase + dwAddressOfNewEntryPoint + pCurrentSectionHeader->PointerToRawData - pCurrentSectionHeader->VirtualAddress);
	*(DWORD*)&(pShell[25]) = dwAddressOfEntryPoint;
	memcpy((char *)pRawData, pShell, 90);
	return true;

}

int Encrypt32()
{
	char pShell[] = {
	0x33,0xc0,0x33,0xdb,0xe8,0x00,0x00,0x00,0x00,0x58,
	0x25,0x00,0x00,0xf0,0xff,0xbb,0x00,0x10,0x00,0x00,
	0x0b,0xd8,0x33,0xc9,0x8b,0xfb,0xeb,0x0c,0x8b,0x04,
	0x8f,0x35,0xfe,0xca,0xaf,0xde,0x89,0x04,0x8f,0x41,
	0x83,0xf9,0x0a,0x72,0xef,0x53,0xc3,0x00,0x00,0x00,
	};

	PIMAGE_DOS_HEADER pDosHeader;
	PIMAGE_NT_HEADERS pNtHeader;
	PIMAGE_OPTIONAL_HEADER pNtOptionHeader;
	PIMAGE_FILE_HEADER pNtFileHeader;
	WORD wSizeOfOptionalHeader;
	DWORD dwAddressOfEntryPoint;
	DWORD dwAddressOfNewEntryPoint;
	PIMAGE_SECTION_HEADER pFirstSectionHeader;
	PIMAGE_SECTION_HEADER pCurrentSectionHeader;
	LPVOID pRawData;

	if (bOnload != ACTIVE)
	{
		return NOT_ACTIVE;
	}

	pDosHeader = (PIMAGE_DOS_HEADER)pExeBase;
	pNtHeader = (PIMAGE_NT_HEADERS)((ULONGLONG)pExeBase + (pDosHeader->e_lfanew));
	pNtFileHeader = (PIMAGE_FILE_HEADER)(&(pNtHeader->FileHeader));
	pNtOptionHeader = (PIMAGE_OPTIONAL_HEADER)(&(pNtHeader->OptionalHeader));

	dwAddressOfEntryPoint = pNtOptionHeader->AddressOfEntryPoint;
	wSizeOfOptionalHeader = pNtFileHeader->SizeOfOptionalHeader;

	pFirstSectionHeader = IMAGE_FIRST_SECTION(pNtHeader);
	pCurrentSectionHeader = pFirstSectionHeader;
	WORD wOrderOfSections = 0;
	while (wOrderOfSections < wNumberberOfSections)
	{
		wOrderOfSections++;
		if (dwAddressOfEntryPoint >= pCurrentSectionHeader->VirtualAddress
			&& dwAddressOfEntryPoint < (pCurrentSectionHeader->VirtualAddress + (DWORD)(pCurrentSectionHeader->Misc.VirtualSize)))
		{
			break;
		}
		pCurrentSectionHeader = (PIMAGE_SECTION_HEADER)((ULONGLONG)pCurrentSectionHeader + IMAGE_SIZEOF_SECTION_HEADER);
	}
	if (pCurrentSectionHeader->VirtualAddress == 0x00)
	{
		return false;
	}

	// Alter the flag of .code section
	pCurrentSectionHeader->Characteristics = pCurrentSectionHeader->Characteristics | 0x80000000;

	// Encypt program
	pRawData = (LPVOID)((ULONGLONG)pExeBase + dwAddressOfEntryPoint + pCurrentSectionHeader->PointerToRawData - pCurrentSectionHeader->VirtualAddress);
	for (int i = 0; i < 0x0a; i++)
	{
		DWORD tmp;
		tmp = ((DWORD*)pRawData)[i];
		tmp = tmp ^ 0xdeafcafe;
		((DWORD*)pRawData)[i] = tmp;
	}

	// Inject decrypt code
	dwAddressOfNewEntryPoint = pCurrentSectionHeader->VirtualAddress + pCurrentSectionHeader->Misc.VirtualSize;
	pNtOptionHeader->AddressOfEntryPoint = dwAddressOfNewEntryPoint;

	pRawData = (LPVOID)((ULONGLONG)pExeBase + dwAddressOfNewEntryPoint + pCurrentSectionHeader->PointerToRawData - pCurrentSectionHeader->VirtualAddress);
	*(DWORD*)&(pShell[0x10]) = dwAddressOfEntryPoint;
	memcpy((char *)pRawData, pShell, 50);
	return true;
}