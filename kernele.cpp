/*
 * kernelev.cpp
 *
 *  Created on: Aug 12, 2019
 *      Author: OS1
 */

#include "kernele.h"
#include "System.h"
#include "PCB.h"
#include "SCHEDULE.H"
#include "ivtentry.h"


KernelEv::KernelEv(IVTNo ivt) {
	holder = (PCB*)System::running;
	blocked = 0;
	value = 0;
	ivtNo = ivt;
	//(IVTEntry::IVT[ivtNo])->myKernelEv = this;
	System::kernelEvents[ivtNo] = this;
}

KernelEv::~KernelEv() {
	if (blocked != 0) {
		if(blocked->status != KILLED){
			blocked->status = READY;
			Scheduler::put(blocked);
		}
		blocked = 0;
		holder = 0;
	}
	System::kernelEvents[ivtNo] = 0;
}

void KernelEv::wait() {
	if (System::running == holder) {
		if (blocked == 0) {
			System::disablePreemption();
				System::running->status = BLOCKED;
				blocked = (PCB*)System::running;
			System::enablePreemption();
			System::dispatch();
		}
	}
}

void KernelEv::signal() {
	if (blocked != 0){
		System::disablePreemption();
			if(blocked->status != KILLED){
			blocked->status = READY;
			Scheduler::put(blocked);
		}
		blocked = 0;
		System::enablePreemption();
		System::dispatch();
	}

}


