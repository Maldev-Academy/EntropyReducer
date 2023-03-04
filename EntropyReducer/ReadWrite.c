#include <Windows.h>
#include <stdio.h>

#include "Common.h"

#define PREFIX ".ER"


// print error message to conosle
BOOL ReportError(const char* ApiName) {
	printf("[!] \"%s\" [ FAILED ] \t%d \n", ApiName, GetLastError());
	return FALSE;
}



// read file from disk 
BOOL ReadPayloadFile(IN PCSTR cFileInput, OUT PBYTE* pPayloadData, OUT PSIZE_T sPayloadSize)
{
	HANDLE	hFile				= INVALID_HANDLE_VALUE;
	DWORD	dwFileSize			= NULL;
	DWORD	dwNumberOfBytesRead		= NULL;
	PBYTE	pBuffer				= NULL;


	hFile = CreateFileA(cFileInput, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) 
		return ReportError("CreateFileA");

	if ((dwFileSize = GetFileSize(hFile, NULL)) == INVALID_FILE_SIZE)
		return ReportError("GetFileSize");

	pBuffer = (PBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwFileSize);
	if (!pBuffer)
		return ReportError("HeapAlloc");

	if (!ReadFile(hFile, pBuffer, dwFileSize, &dwNumberOfBytesRead, NULL)) {
		printf("[i] Read %ld from %ld Bytes \n", dwNumberOfBytesRead, dwFileSize);
		return ReportError("ReadFile");
	}

	*pPayloadData = pBuffer;
	*sPayloadSize = dwNumberOfBytesRead;

	CloseHandle(hFile);

	if (*pPayloadData == NULL || *sPayloadSize == NULL)
		return FALSE;

	return TRUE;
}




// write file to disk
BOOL WritePayloadFile(IN PSTR cFileInput, IN LPCVOID pPayloadData, IN SIZE_T Size)
{
	HANDLE	hFile				= INVALID_HANDLE_VALUE;
	DWORD	dwNumberOfBytesWritten		= NULL;
	// constructing the output file name
	CHAR*	cFileName			= (CHAR*)malloc(strlen(cFileInput) + sizeof(PREFIX) + 1);
	wsprintfA(cFileName, "%s%s", cFileInput, PREFIX);



	hFile = CreateFileA(cFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return ReportError("CreateFileA");

	if (!WriteFile(hFile, (LPCVOID)pPayloadData, Size, &dwNumberOfBytesWritten, NULL) || (DWORD)Size != dwNumberOfBytesWritten) {
		printf("[i] Wrote %ld from %ld Bytes \n", dwNumberOfBytesWritten, Size);
		return ReportError("WriteFile");
	}

	// cleanup
	free(cFileName);
	CloseHandle(hFile);

	return TRUE;
}





