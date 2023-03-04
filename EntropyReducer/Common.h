#pragma once

#include <Windows.h>


#ifndef HELPER_H
#define HELPER_H

// these can be modified, and are are fun to play with, but:
//	- 'BUFF_SIZE' should be at least 2
//	- 'NULL_BYTES' should be at least 1

#define BUFF_SIZE				0x04			// for every BUFF_SIZE bytes in the payload there will be NULL_BYTES bytes empty (0x00)
#define NULL_BYTES				0x01			// change this value to further reduce the entropy (this add more size to the final payload) - the bigger the lower entropy


struct LINKED_LIST;
typedef struct _LINKED_LIST
{
	BYTE					pBuffer	[BUFF_SIZE];	// payload's bytes
	BYTE					pNull	[NULL_BYTES];	// null padded bytes
	INT						ID;						// node id
	struct LINKED_LIST*		Next;					// next node pointer	

}LINKED_LIST, * PLINKED_LIST;

// this will represent the seraizlized size of one node
#define SERIALIZED_SIZE			(BUFF_SIZE + NULL_BYTES + sizeof(INT))		

// serialized payload size:		SERIALIZED_SIZE * (number of nodes)
// number of nodes: (padded payload size) / BUFF_SIZE

typedef enum SORT_TYPE {
	SORT_BY_ID,
	SORT_BY_BUFFER
};

// set the 'sPayloadSize' variable to be equal to the next nearest number that is multiple of 'N'
#define NEAREST_MULTIPLE(sPayloadSize, N)(SIZE_T)((SIZE_T)sPayloadSize + (int)N - ((SIZE_T)sPayloadSize % (int)N))

// functions prototypes from Helper.c
BOOL WritePayloadFile(IN PSTR cFileInput, IN LPCVOID pPayloadData, IN SIZE_T Size);
BOOL ReadPayloadFile(IN PCSTR cFileInput, OUT PBYTE* pPayloadData, OUT PSIZE_T sPayloadSize);


// functions prototypes from LinkedList.c
BOOL InitializePayloadList(IN PBYTE pPayload, IN OUT PSIZE_T sPayloadSize, OUT PLINKED_LIST* ppLinkedList);
PLINKED_LIST InsertAtTheEnd(IN OUT PLINKED_LIST LinkedList, IN PBYTE pBuffer, IN INT ID);
VOID MergeSort(PLINKED_LIST* top, enum SORT_TYPE eType);


#endif // !HELPER_H
