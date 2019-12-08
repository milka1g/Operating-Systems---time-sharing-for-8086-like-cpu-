/*
 * ivtentry.cpp
 *
 *  Created on: Aug 12, 2019
 *      Author: OS1
 */

#include <dos.h>
#include "ivtentry.h"

#include "kernele.h"
#include "ivtentry.h"
#include "System.h"


IVTEntry* IVTEntry::IVT[256] = { 0 };

IVTEntry::IVTEntry(IVTNo numEntry, pInterruptR interruptRoutine) {
	oldRoutine = 0;
	ivtNo = numEntry;
	if(IVT[ivtNo]==0){
#ifndef BCC_BLOCK_IGNORE
			asm pushf;
			asm cli;
			oldRoutine = getvect(ivtNo);
			setvect(ivtNo, interruptRoutine);
			asm popf;
#endif
	IVT[ivtNo] = this;
	}

}

IVTEntry::~IVTEntry() {
#ifndef BCC_BLOCK_IGNORE
	asm pushf;
	asm cli;
	setvect(ivtNo, oldRoutine);
	asm popf;
#endif
	IVT[ivtNo] = 0;
	//we return the old routine to that IVT entry
}

void IVTEntry::signal() {
	if (System::kernelEvents[ivtNo])
		System::kernelEvents[ivtNo]->signal();
}


IVTEntry* IVTEntry::getEntry(IVTNo num) {
	if (num >= 0 && num < 256) return IVT[num];
	return 0;
}




