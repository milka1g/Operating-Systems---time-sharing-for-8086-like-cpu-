/*
 * kersem.cpp
 *
 *  Created on: Aug 10, 2019
 *      Author: OS1
 */

#include "kersem.h"
#include "list.h"
#include "queue.h"
#include "SCHEDULE.H"

List* KernelSem::allSemaphores = new List();

KernelSem::KernelSem(int init) {
	value = init;
	System::disablePreemption();
	limitedWait = new Queue();
	unlimitedWait = new Queue();
	allSemaphores->add(this);
	System::enablePreemption();
}

int KernelSem::wait(Time maxTimeToWait){  // running thread is calling wait on semaphore
		if(--value<0){ 						  // depending on maxTimeToWait running is either
				System::disablePreemption();	  // waiting for 55*maxTimeToWait ms or waiting until it gets unblocked
				System::running->status = BLOCKED;
				if(maxTimeToWait == 0){				//if it is 0 running is getting blocked and waiting until signal unblocks it
					unlimitedWait->enQueue((PCB*)System::running);
					System::running->blockType = UNLIMITED;
				}
				else { 								// here running is waiting until signal unblocks it or enough time passes
					System::running->waitOnSem = maxTimeToWait;
					limitedWait->enQueue((PCB*)System::running);
					System::running->blockType = LIMITED;
				}
				System::enablePreemption();
				System::dispatch();					// running is getting blocked and another ready thread is picked
			}
			System::disablePreemption();
			int ret = 1;
			if(System::running->blockType == LIMITED && System::running->waitOnSem == 0) // if all waiting time passed ret is 0
				ret = 0;
			System::running->blockType = NONE; //only do it here so you can check above condition (was nasty mistake!)
			System::enablePreemption();
			return ret;
}


int KernelSem::signal(int n){  // negative value means that much threads are waiting on it
	System::disablePreemption();
	int ret;
	if(n==0){
		ret = 0;
		if(++value<=0){ //means that it was <0 and there was at least 1 thread waiting so unblock it
			unblock();
			System::enablePreemption();
			return ret;
		} else {
			System::enablePreemption();
			return ret;
		}
	}
	else if(n>0){
		for(ret = 0; ret < n && ((limitedWait->size > 0) || (unlimitedWait->size > 0)); ret++)
			unblock();
		value+=n;
		System::enablePreemption();
		return ret;
	}
	System::enablePreemption();
	return n;
}

void KernelSem::unblock(){
	System::disablePreemption();
	PCB* ready = limitedWait->deQueue(); // first we unblock those that wait unlimited because limited waiting ones will get unblocked anyways ^-1
		if(ready == 0) ready = unlimitedWait->deQueue();
		//if(ready->status != BLOCKED && ready != 0) cout << "Something wrong, wasn't BLOCKED!";
		if(ready == 0) {
			System::enablePreemption(); //nothing waiting, init value was less than 0 so signal has no effect
			return; // has to be done otherwise you mess nested locker
		}
		if(ready->status != KILLED){
		ready->status = READY;
		Scheduler::put(ready);
		}
	System::enablePreemption();
}

KernelSem::~KernelSem(){
	System::disablePreemption();
	//if(limitedWait->size>0) cout << "Limited VECE"<< limitedWait->size;
	//if(unlimitedWait->size>0) cout << "UNlimited VECE" << unlimitedWait->size;
	//while(unlimitedWait->size>0 || limitedWait->size>0)
		//unblock();
	allSemaphores->remove(this);
	delete unlimitedWait; unlimitedWait = 0;
	delete limitedWait; limitedWait = 0;
	System::enablePreemption();
}

void KernelSem::timePass(){
	allSemaphores->time();
}



