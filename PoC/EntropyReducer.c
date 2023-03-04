#include <Windows.h>
#include <stdio.h>


#include "EntropyReducer.h"


struct LINKED_LIST;
typedef struct _LINKED_LIST
{
    BYTE					pBuffer[BUFF_SIZE];	    // payload's bytes
    BYTE					pNull[NULL_BYTES];	    // null padded bytes
    INT						ID;						// node id
    struct LINKED_LIST* Next;					    // next node pointer	

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


// used to insert a node at the end of the given linked list
// - LinkedList: a variable pointing to a 'LINKED_LIST' structure, this will represent the linked list head, this variable can be NULL, and thus will be initialized here
// - pBuffer: the payload chunk (of size 'BUFF_SIZE')
// - ID: the id of the node 
PLINKED_LIST InsertAtTheEnd(IN OUT PLINKED_LIST LinkedList, IN PBYTE pBuffer, IN INT ID)
{

    // new tmp pointer, pointing to the head of the linked list
    PLINKED_LIST pTmpHead = (PLINKED_LIST)LinkedList;

    // creating a new node
    PLINKED_LIST pNewNode = (PLINKED_LIST)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LINKED_LIST));
    if (!pNewNode)
        return NULL;
    memcpy(pNewNode->pBuffer, pBuffer, BUFF_SIZE);
    pNewNode->ID = ID;
    pNewNode->Next = NULL;

    // if the head is null, it will start at the new node we created earlier
    if (LinkedList == NULL) {
        LinkedList = pNewNode;
        return LinkedList;
    }

    // else we will keep walking down the linked list till we find an empty node 
    while (pTmpHead->Next != NULL)
        pTmpHead = pTmpHead->Next;

    // pTmpHead now is the last node in the linked list
    // setting the 'Next' value to the new node
    pTmpHead->Next = pNewNode;

    // returning the head of the linked list
    return LinkedList;
}


// covert raw payload bytes to a linked list
// - pPayload: Base Address of the payload
// - sPayloadSize: pointer to a SIZE_T variable that holds the size of the payload, it will be set to the serialized size of the linked list
// - ppLinkedList: pointer to a LINKED_LIST structure, that will represent the head of the linked list
BOOL InitializePayloadList(IN PBYTE pPayload, IN OUT PSIZE_T sPayloadSize, OUT PLINKED_LIST* ppLinkedList)
{

    // variable used to count the linked list elements (used to calculate the final size)
    // it is also used as the node's ID
    unsigned int x = 0;


    // setting the payload size to be multiple of 'BUFF_SIZE'
    SIZE_T	sTmpSize = NEAREST_MULTIPLE(*sPayloadSize, BUFF_SIZE);
    if (!sTmpSize)
        return FALSE;

    // new padded buffer 
    PBYTE	pTmpBuffer = (PBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sTmpSize);
    if (!pTmpBuffer)
        return FALSE;

    memcpy(pTmpBuffer, pPayload, *sPayloadSize);

    // for each 'BUFF_SIZE' in the padded payload, add it to the linked list
    for (int i = 0; i < sTmpSize; i++) {
        if (i % BUFF_SIZE == 0) {
            *ppLinkedList = InsertAtTheEnd((PLINKED_LIST)*ppLinkedList, &pTmpBuffer[i], x);
            x++;
        }
    }

    // updating the size to be the size of the whole *serialized* linked list
    *sPayloadSize = SERIALIZED_SIZE * x;

    // if the head is null
    if (*ppLinkedList == NULL)
        return FALSE;

    return TRUE;
}


//------------------------------------------------------------------------------------------------------------------------------------------------------
// the following is the mergesort algorithm implementation

// split the nodes of the list into two sublists
void Split(PLINKED_LIST top, PLINKED_LIST* front, PLINKED_LIST* back) {
    PLINKED_LIST fast = top->Next;
    PLINKED_LIST slow = top;

    /* fast pointer advances two nodes, slow pointer advances one node */
    while (fast != NULL) {
        fast = fast->Next;		/* "fast" moves on first time */
        if (fast != NULL) {
            slow = slow->Next;	/* "slow" moves on first time */
            fast = fast->Next;	/* "fast" moves on second time */
        }
    }

    /* "slow" is before the middle in the list, so split it in two at that point */
    *front = top;
    *back = slow->Next;
    slow->Next = NULL;			/* end of the input list */
}


// merge two linked lists 
PLINKED_LIST Merge(PLINKED_LIST top1, PLINKED_LIST top2, enum SORT_TYPE eType) {
    if (top1 == NULL)
        return top2;
    else
        if (top2 == NULL)
            return top1;

    PLINKED_LIST pnt = NULL;

    int iValue1 = 0;
    int iValue2 = 0;

    switch (eType) {
        // this is used to deobfuscate
    case SORT_BY_ID: {
        iValue1 = (int)top1->ID;
        iValue2 = (int)top2->ID;
        break;
    }
                   // this is used to obfuscate
    case SORT_BY_BUFFER: {
        iValue1 = (int)(top1->pBuffer[0] ^ top1->pBuffer[1] ^ top1->pBuffer[2]);   // calculating a value from the payload buffer chunk
        iValue2 = (int)(top2->pBuffer[0] ^ top2->pBuffer[1] ^ top2->pBuffer[2]);   // calculating a value from the payload buffer chunk
        break;
    }
    default: {
        return NULL;
    }
    }

    /* pick either top1 or top2, and merge them */
    if (iValue1 <= iValue2) {
        pnt = top1;
        pnt->Next = Merge(top1->Next, top2, eType);
    }
    else {
        pnt = top2;
        pnt->Next = Merge(top1, top2->Next, eType);
    }
    return pnt;
}


// the main sorting function
// - pLinkedList : is the head node of the linked list
// - eType :
//      * is set to SORT_BY_BUFFER to obfuscate
//      * is set to SORT_BY_ID to deobfuscate
VOID MergeSort(PLINKED_LIST* top, enum SORT_TYPE eType) {
    PLINKED_LIST tmp = *top, * a, * b;

    if (tmp != NULL && tmp->Next != NULL) {
        Split(tmp, &a, &b);				/* (divide) split head into "a" and "b" sublists */

        /* (conquer) sort the sublists */
        MergeSort(&a, eType);
        MergeSort(&b, eType);

        *top = Merge(a, b, eType);				/* (combine) merge the two sorted lists together */
    }
}


//------------------------------------------------------------------------------------------------------------------------------------------------------


BOOL Deobfuscate(IN PBYTE pFuscatedBuff, IN SIZE_T sFuscatedSize, OUT PBYTE* ptPayload, OUT PSIZE_T psSize) 
{
    PLINKED_LIST	pLinkedList = NULL;

    // deserialize (from buffer to linked list - this must be done to re-order the payload's bytes)
    for (size_t i = 0; i < sFuscatedSize; i++) {
        if (i % SERIALIZED_SIZE == 0)
            pLinkedList = InsertAtTheEnd(pLinkedList, &pFuscatedBuff[i], *(int*)&pFuscatedBuff[i + BUFF_SIZE + NULL_BYTES]);
    }

    // re-ordering the payload's bytes
    MergeSort(&pLinkedList, SORT_BY_ID);

    PLINKED_LIST	pTmpHead = pLinkedList;
    SIZE_T			BufferSize = NULL;
    PBYTE			BufferBytes = (PBYTE)LocalAlloc(LPTR, BUFF_SIZE);
    unsigned int	x = 0x00;

    while (pTmpHead != NULL) {

        BYTE TmpBuffer[BUFF_SIZE] = { 0 };

        // copying the 'pBuffer' element from each node
        memcpy(TmpBuffer, pTmpHead->pBuffer, BUFF_SIZE);

        BufferSize += BUFF_SIZE;

        // reallocating to fit the new buffer
        if (BufferBytes != NULL) {
            BufferBytes = (PBYTE)LocalReAlloc(BufferBytes, BufferSize, LMEM_MOVEABLE | LMEM_ZEROINIT);
            memcpy((PVOID)(BufferBytes + (BufferSize - BUFF_SIZE)), TmpBuffer, BUFF_SIZE);
        }

        pTmpHead = pTmpHead->Next;
        x++; // number if nodes
    }

    *ptPayload = BufferBytes;  // payload base address 
    *psSize = x * BUFF_SIZE; // payload size


    // free linked list's nodes
    pTmpHead = pLinkedList;
    PLINKED_LIST pTmpHead2 = pTmpHead->Next;
    
    while (pTmpHead2 != NULL) {

        if (!HeapFree(GetProcessHeap(), 0, (PVOID)pTmpHead)) {
            // failed
        }
        pTmpHead    = pTmpHead2;
        pTmpHead2   = pTmpHead2->Next;
    }
  


    if (*ptPayload != NULL && *psSize < sFuscatedSize)
        return 1;
    else
        return 0;
}


