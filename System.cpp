/*
 * System.cpp
 *
 *  Created on: Aug 3, 2019
 *      Author: OS1
 */
#include "System.h"
#include "queue.h"
#include <iostream.h>
#include "SCHEDULE.H"
#include "thread.h"
#include "idleT.h"
#include <dos.h>
#include "PCB.h"
#include "kersem.h"

extern void tick();

volatile InterruptRequested System::interruptRequested = NO;
volatile processSignals System::processingSignals = NO;
volatile int System::locker = 0;
volatile int System::counter = 0; //it is main
volatile PCB* System::startingPCB = 0;
KernelEv* System::kernelEvents[256] = { 0 };
Thread* System::starting = 0;
Thread* System::runningThread = 0;
Thread* System::idle = 0;
volatile PCB* System::running = 0;
pInterruptR System::oldRoutine  = 0;
Queue* System::allThreads = 0;
Queue* System::finishedThreads = 0;

void System::disablePreemption() { // this will enable nesting, one "sti" won't mess multiple "cli"s
	System::locker--;
}

void System::enablePreemption() {
	if(System::locker == 0) return;
	System::locker++;
}

void interrupt System::timer(...){
	static unsigned int tsp, tss, tbp, old, switched;
	old = 0; switched = 0;
	if((interruptRequested == NO) && counter > 0) counter--; // if dispatch() not called explicitly it is a real timer interrupt

	if(interruptRequested == NO){ // real timer interrupt so you call tick for tests to measure how bad this all is and for sems timepass
		old = 1;
		tick();
		KernelSem::timePass();
	} else {
		//you do nothing because it is explicit call to dispatch
	}

	if(((counter == 0 && running->timeSlice!=0) || (interruptRequested == YES)) &&  processingSignals == N){ // if thread's not executing infinitely AND if counter's reached 0
																		   // you have to switch context OR it is requested in dispatch
		if(locker == 0){ //lockFlag from lab, if it is 0 you are allowed to switch
			interruptRequested = NO; // if there was context switch on demand it is processed here
 // sp,ss,bp in asm block are CPU's real stack segment, stack pointer and base pointer
			switched = 1;
#ifndef BCC_BLOCK_IGNORE
		asm{
				mov tsp, sp
				mov tss, ss
				mov tbp, bp
			}
#endif
// saving those for later execution
		running->sp = tsp;
		running->ss = tss;
		running->bp = tbp;

		if(running->status == RUNNING) running->status = READY;
		if(running->id > 0 && running->status == READY){ //id = 0 is idle thread, 1 is main, it is not scheduled
			Scheduler::put((PCB*)running);
		}
		
		if(running->status == KILLED){
			delete running->myStack;
			running->myStack = 0;
		}

		System::running = Scheduler::get();
		
		while((running!=0)&&(running->status==KILLED)){
			running=Scheduler::get();
		}

		if(System::running == 0){
			if(System::startingPCB!=0 && System::startingPCB->status == READY){
				running = startingPCB;
			}
			else
			System::running = System::idle->myPCB;
		}

		running->status = RUNNING;
// getting ss,sp,bp of newly selected thread and putting it in CPU's registers

		tsp = running->sp;
		tss = running->ss;
		tbp = running->bp;
		System::counter = running->timeSlice;

#ifndef BCC_BLOCK_IGNORE
	asm{
		mov sp, tsp
		mov ss, tss
		mov bp, tbp
	}
#endif

		} //locker
		else {
			interruptRequested = YES; // this happens when locker!=0 so you have to say it is requested whether
										// it was requested before or not, as soon it can be done it has to be done
		}
	}

	if(old == 1){ // if we did context switch that was requested we have to go to old routine so the host OS does his work
								 // or simply there was host OS's interrupt that has to be tunneled towards real interrupt routine
#ifndef BCC_BLOCK_IGNORE
		asm int 60h;
#endif
	}
	
	if(switched == 1){
		processingSignals == Y;
		System::running->myThr->processSignals(); //if it has signal 0 it gets killed and calls dispatch again, make sure processingSignals is RESET
		processingSignals == N;
	}


}

void System::dispatch(){
	interruptRequested = YES; // we just set this to 1 and if locker==0 it will be done, we go to 8h because that is where our timer resides
								// we use int instruction because that way it is thread safe, calling timer as regular method is NOT (PSWI = 1)
#ifndef BCC_BLOCK_IGNORE
	asm int 8h;
#endif
}

void System::init(){
#ifndef BCC_BLOCK_IGNORE
	asm cli;
#endif
	starting = new Thread(maxStackSize, 1, -1); // private thread constructor with id = -1
	startingPCB = starting->myPCB;
	starting->myPCB->status = RUNNING; // no initStack cuz it is a PC's main thread, it has everything it needs
	idle = new idleThread(); //private thread constructor with id 0
	idle->start();  //it's blocked because main is running freely
	running = startingPCB;
	allThreads = new Queue();
	finishedThreads = new Queue();
#ifndef BCC_BLOCK_IGNORE
	oldRoutine = getvect(0x8); // from entry 0x8 you get host OS's interrupt routine address
	setvect(0x8,timer); // set yours there
	setvect(0x60,oldRoutine); // move host's to 0x60
#endif
#ifndef BCC_BLOCK_IGNORE
	asm sti;
#endif
	//create mainThread and idleThread

}

void System::restore(){
	#ifndef BCC_BLOCK_IGNORE
	asm cli;
	#endif

	//restoring oldRoutine back to its original entry
	
	while(System::allThreads->getSize()>0){ // all threads must be finished to restore
		Thread* tmp = System::allThreads->deQueue()->getMyThr();
		tmp->waitToComplete();
	}
	//cout << "bybye";

	delete KernelSem::allSemaphores; KernelSem::allSemaphores = 0;
	delete finishedThreads; finishedThreads = 0;
	delete allThreads; allThreads = 0;
	delete idle; idle = 0;
	delete starting; starting = 0;

#ifndef BCC_BLOCK_IGNORE
	setvect(0x8,oldRoutine);
	asm sti;
#endif

}
