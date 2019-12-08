/*
 * PCB.cpp
 *
 *  Created on: Aug 3, 2019
 *      Author: OS1
 */
#include <iostream.h>
#include "System.h"
#include "PCB.h"
#include <dos.h>
#include "thread.h"
#include "SCHEDULE.H"
#include "queue.h"


ID PCB::_id = 1; //-1 STARTING(MAIN) THREAD, 0 IDLE THREAD
int PCB::blockedSignalsGlobally[16] = {0};


PCB::PCB(Thread *myTh, StackSize ssize, Time tslice, int type){
	System::disablePreemption();
	if(ssize > maxStackSize) ssize = maxStackSize;
	if(ssize < 1024) ssize =  1024; //maybe too much? no? ok

	myThr = myTh;
	myStack = 0;
	stackSize = ssize;
	timeSlice = tslice;
	waitOnSem = 0;
	status = NEW;
	blockType = NONE;
	parent = myTh->parentT; //initialized prior to this constructor

	ss = 0; sp = 0; bp = 0;
	sig1 = 0;
	if(type>0){
		id = _id++;
		for(int i = 0; i<16;i++){
				blockedSignals[i] = parent->myPCB->blockedSignals[i];
				handlerHeads[i] = handlerTails[i] = 0;
				SignalHandlerNode* tmp = parent->myPCB->handlerHeads[i];
				while(tmp!=0){
					if(handlerHeads[i] == 0) handlerHeads[i] = handlerTails[i] = new SignalHandlerNode(tmp->handler);
					else handlerTails[i] = handlerTails[i]->next = new SignalHandlerNode(tmp->handler);
					tmp = tmp->next;
				}
			}
		allSignalsTail = allSignalsHead = 0;
		handlerTails[0] = handlerHeads[0] = new SignalHandlerNode(&Thread::signal0Handler);
	} else {
		id = type;
		for(int i = 0; i<16;i++){
				blockedSignals[i] = 0;
				handlerHeads[i] = handlerTails[i] = 0;
			}
		handlerTails[0] = handlerHeads[0] = new SignalHandlerNode(&Thread::signal0Handler);
		allSignalsTail = allSignalsHead = 0;
	}
	waitingThreads = new Queue();

	if(this->id > 0) System::allThreads->enQueue(this); //-1 starting, 0 idle

	System::enablePreemption();
}

void PCB::initStack(){

	System::disablePreemption();

		unsigned int size_int = stackSize/sizeof(unsigned); //exmpl: unsigned == unsigned int == 2B -> long/int = 2 == 4/2

		myStack = new unsigned int[size_int];  //exmpl: size in long = 100 == 400B, size in int = 200 == 200*2 = 400


		unsigned int PSW = 0x200; // 0000 0010 0000 0000 I bit is SET
		unsigned int SS, SP, CS, IP;

	#ifndef BCC_BLOCK_IGNORE
		CS = FP_SEG(PCB::wrapper);
		IP = FP_OFF(PCB::wrapper);
		SS = FP_SEG(myStack+size_int-12);
		SP = FP_OFF(myStack+size_int-12);
	#endif

		myStack[size_int-1] = PSW;
		myStack[size_int-2] = CS;
		myStack[size_int-3] = IP;
		//myStack[size_int-4] = AX
		//myStack[size_int-5] = BX
		//myStack[size_int-6] = CX
		//myStack[size_int-7] = DX
		//myStack[size_int-8] = ES
		//myStack[size_int-9] = DS
		//myStack[size_int-10] = SI
		//myStack[size_int-11] = DI
		//myStack[size_int-12] = BP


		this->sp = SP;
		this->ss = SS;
		this->bp = SP;

	System::enablePreemption();

}

void PCB::wrapper(){
	// a mechanism to get the address of run() method.
	// when classes load their run() methods reside somewhere unknown
	// but we can get the address of PCB's static function that will call the
	// run method of its thread
	System::running->myThr->run();
	// run() ended here
	System::disablePreemption();
	System::running->status = OVER;
	System::allThreads->deletePCB(((PCB*)System::running)->id);
	System::finishedThreads->enQueue((PCB*)System::running);
	PCB* tmp;

	while(System::running->waitingThreads->size > 0){ // puts all waiting threads into scheduler
		tmp = System::running->waitingThreads->deQueue();
			if(tmp->status != KILLED){
			tmp->status = READY;
			Scheduler::put(tmp);
			//if(tmp->id == -1) cout << "main UNBLOCKED!!!";
		}
	}
	if (System::running->parent) {
		System::running->parent->myPCB->sig1 = 1;
		System::running->parent->myPCB->signal(1);
		System::running->parent->myPCB->sig1 = 0;
		}
	System::running->myThr->signal2Handler();
	System::enablePreemption();
	System::dispatch(); // this thread will be deleted
}

PCB::~PCB(){
	System::disablePreemption();
	
	//if(waitingThreads->size > 0) {
	//	cout << "WAITING THREADS IS FULL, SOMETHING WENT WRONG, BAD DELETE" << endl ;
	//}
	while(System::running->waitingThreads->size > 0){ // puts all waiting threads into scheduler
			PCB* tmp = System::running->waitingThreads->deQueue();
				if(tmp->status != KILLED){
				tmp->status = READY;
				Scheduler::put(tmp);
			}
		}
	
	for (int i = 0; i < 16; i++) {
					while (handlerHeads[i]) {
						SignalHandlerNode* old = handlerHeads[i];
						handlerHeads[i] = handlerHeads[i]->next;
						delete old;
					}
						handlerHeads[i] = handlerTails[i] = 0;
					}
					while (allSignalsHead) {
						SignalIdNode* old = allSignalsHead;
						allSignalsHead = allSignalsHead->next;
						delete old;
					}
	allSignalsHead = allSignalsTail = 0;
	delete myStack;
	delete waitingThreads;
	myStack = 0;
	waitingThreads = 0;
	System::enablePreemption();
}

void PCB::signal(SignalId id){
	if((id==1 && sig1==0) || id==2 || id>15 || handlerHeads[id] == 0) return; // signal 1 is called by system (must be sig1==1),
																		 //signal 2 is handled immediately so you dont register handlers
	System::disablePreemption();											//if that signal has no handlers you dont register it
		allSignalsTail = (!allSignalsHead? allSignalsHead : allSignalsTail->next) = new SignalIdNode(id); //nice
	System::enablePreemption();
}

void PCB::signal0Handler(){
	System::disablePreemption();
	//#ifndef BCC_BLOCK_IGNORE
  		//asm sti;
	//#endif
	//this handler is called inside timer while handling processes so if running has signal 0 it has to be KILLED
	System::running->status = KILLED; //same as OVER but ok, it was killed with signal 0

	PCB* tmp;
	while(System::running->waitingThreads->size > 0){
		tmp = System::running->waitingThreads->deQueue();
		if(tmp->status != KILLED){
		tmp->status = READY;
		Scheduler::put(tmp);
		}
	}

	System::allThreads->deletePCB(((PCB*)System::running)->id);
	System::finishedThreads->enQueue((PCB*)System::running);

	if(System::running->parent != 0){
		System::running->parent->myPCB->sig1 = 1; //ensuring that system calls this signal
		System::running->parent->myPCB->signal(1);
		System::running->parent->myPCB->sig1 = 0;
	}
	System::running->myThr->signal2Handler(); //this thread dying, signal(2) here, handled immediately


	delete System::running->myStack; //delete everything
	System::running->myStack = 0;
	delete System::running->waitingThreads;
	System::running->waitingThreads = 0;
	for (int i = 0; i < 16; i++) {
					while (System::running->handlerHeads[i]) {
						SignalHandlerNode* old = System::running->handlerHeads[i];
						System::running->handlerHeads[i] = System::running->handlerHeads[i]->next;
						delete old;
					}
					System::running->handlerHeads[i] = System::running->handlerTails[i] = 0;
					}
					while (System::running->allSignalsHead) {
						SignalIdNode* old = System::running->allSignalsHead;
						System::running->allSignalsHead = System::running->allSignalsHead->next;
						delete old;
					}
	System::running->allSignalsHead = System::running->allSignalsTail = 0;
	System::processingSignals = N; // maybe, cuz there's no switch if this is Y
	//#ifndef BCC_BLOCK_IGNORE
  	//	asm cli;
	//#endif
	System::enablePreemption();
	System::dispatch();
}

void PCB::signal2Handler(){
	if(this->blockedSignals[2] || this->blockedSignalsGlobally[2]) return;
	SignalHandlerNode* tmp = handlerHeads[2];
	while(tmp!=0){
		System::processingSignals == Y;
		tmp->handler();
		System::processingSignals == N;
		tmp = tmp->next;
	}
}


void PCB::unblockSignal(SignalId signal) {
	if (signal > 15) return;
	blockedSignals[signal] = 0;
}

void PCB::blockSignal(SignalId signal) {
	if (signal > 15) return;
	blockedSignals[signal] = 1;
}

void PCB::unblockSignalGlobally(SignalId signal) {
	if (signal > 15) return;
	blockedSignalsGlobally[signal] = 0;
}

void PCB::blockSignalGlobally(SignalId signal) {
	if (signal > 15) return;
	blockedSignalsGlobally[signal] = 1;
}

void PCB::unregisterAllHandlers(SignalId id){
	SignalHandlerNode* tmp = handlerHeads[id], *old;
	while(tmp){
		old = tmp;
		tmp = tmp->next;
		delete old;
	}
	handlerHeads[id] = handlerTails[id] = 0;
}

void PCB::swap(SignalId id, SignalHandler hand1, SignalHandler hand2) {
	System::disablePreemption();
	if (id == 0 || id > 15) return; //signal0 has specific handler, it would kill thread
	SignalHandlerNode* han1 = 0, *han2 = 0;

	SignalHandlerNode* tmp = handlerHeads[id];
	while(tmp!=0){
		if(tmp->handler == hand1) han1 = tmp;
		if(tmp->handler == hand2) han2 = tmp;
		tmp = tmp->next;
	}
	if ((han1==0) || (han2==0)) return;
	han1->handler = hand2;
	han2->handler = hand1;
	System::enablePreemption();
}

void PCB::registerHandler(SignalId signal, SignalHandler handler){
	if (signal == 0 || signal > 15) return;
		System::disablePreemption();
		handlerTails[signal] = (!handlerHeads[signal]? handlerHeads[signal] : handlerTails[signal]->next) = new SignalHandlerNode(handler);
		System::enablePreemption();
}

void PCB::processSignals(){
	SignalIdNode *tmp = allSignalsHead, *prev = 0, *old = 0;
	SignalId sig = 0;
		while(tmp!=0) {
			if (blockedSignalsGlobally[tmp->id] || blockedSignals[tmp->id]){
				prev = tmp; tmp = tmp->next; continue;
			}
			sig = tmp->id;
			if (prev) prev->next = tmp->next;
			else allSignalsHead = allSignalsHead->next;
			if (!allSignalsHead) allSignalsTail = 0;
			old = tmp;
			tmp = tmp->next;
			delete old;

			SignalHandlerNode* tmpp = handlerHeads[sig];
			while(tmpp!=0){
				tmpp->handler();
				tmpp=tmpp->next;
			}
		}
}





















