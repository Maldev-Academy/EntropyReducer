#pragma once

#include <Windows.h>


#ifndef HELPER_H
#define HELPER_H

// these values should be the same as 'EntropyReducer.exe'
// if you modified them there, you need to modify these here as well
#define BUFF_SIZE				0x04			
#define NULL_BYTES				0x01			


// Deobfuscate the payload
// - pFuscatedBuff: base address of the obfuscated payload
// - sFuscatedSize: the size of the obfuscated payload
// - ptPayload: pointer to a PBYTE variable that will recieve the deobfuscated payload base address
// - psSize: pointer to a PSIZE_T variable that will recieve the deobfuscated payload size
BOOL Deobfuscate(IN PBYTE pFuscatedBuff, IN SIZE_T sFuscatedSize, OUT PBYTE* ptPayload, OUT PSIZE_T psSize);


#endif // !HELPER_H
