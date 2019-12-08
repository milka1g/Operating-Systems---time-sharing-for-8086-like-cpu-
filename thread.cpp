/*
 * thread.cpp
 *
 *  Created on: Aug 4, 2019
 *      Author: OS1
 */

#include "thread.h"
#include "System.h"
#include "SCHEDULE.H"
#include "PCB.h"
#include <iostream.h>


Thread::Thread(StackSize stackSize, Time timeSlice){
	System::disablePreemption();
	parentT = System::running->myThr;
	myPCB = new PCB(this,stackSize,timeSlice);

	System::enablePreemption();
}

Thread::Thread(StackSize ss, Time ts, int type){
	// used if we are constructing idle and starting(main) threads, this is extra lvl of protection
	System::disablePreemption();
	parentT = 0;
		if(type == -1 || type == 0)
			myPCB = new PCB(this,ss,ts,type);

	System::enablePreemption();
}

Thread::~Thread(){
	// this will put the caller thread in the waitingThreads queue of
	// this thread so this thread has to finish first, otherwise
	// the destructor call might delete this thread before its run() method
	// ends (other threads in waitingThreads won't continue) and also deleting thread
	// will delete its stack so if it gets CPU it wont have stack.
	// so when "this" finishes it unblocks the thread what called destructor
	// and it can safely delete "this" thread = myPCB
	this->waitToComplete(); //blocks here and dispatches

	System::disablePreemption();
	delete myPCB;
	System::enablePreemption();
}

void Thread::start(){
	System::disablePreemption();
	//	this protects from putting the same thread in scheduler if start method is called multiple times
		if(myPCB->status == NEW){
			myPCB->status = READY;
			myPCB->initStack();
			Scheduler::put(myPCB);
		} else {
			//cout << "Thread with id = " << myPCB->getId() << " already started" << endl;
		}
	System::enablePreemption();
}

/////////// BLOCKS CALLER (running) THREAD UNTIL THREAD(this) ON WHICH IT IS CALLED FINISHES   ////////
void Thread::waitToComplete(){
	System::disablePreemption();
	// killed or over threads are finished
			if((this->myPCB->status == KILLED || this->myPCB->status == OVER)){
						System::enablePreemption();
						return;
				}
	// running won't wait for itself to finish
		if(this->myPCB == ((PCB*)System::running)){
			System::enablePreemption();
			return;
		}
	// no thread will wait for idle to finish, defeats the purpose
		if((this == System::idle)){
			System::enablePreemption();
			return;
		}
	// do not wait for starting thread to finish, it finishes last
		if(this->myPCB == ((PCB*)System::startingPCB)){
			System::enablePreemption();
			return;
		}
	// block running thread and put it in waitingThreads of this thread and dispatch. it will be reactivated when run()
	// method of this thread finishes in the wrapper function
		System::running->status = BLOCKED;
		this->myPCB->waitingThreads->enQueue((PCB*)System::running);
		System::enablePreemption();
		System::dispatch();
}

ID Thread::getId(){
	return myPCB->id;
}
ID Thread::getRunningId(){
	return System::running->id;
}

Thread* Thread::getThreadById(ID id){
	if(id==-1) return System::starting;
	if(id==0) return (Thread*)System::idle;

	PCB* ret = System::allThreads->findPCBbyID(id);
	if(ret != 0) return ret->myThr;
	if(ret == 0) ret = System::finishedThreads->findPCBbyID(id);
	if(ret != 0) return ret->myThr;
	return 0;
}

void dispatch(){
	//System::disablePreemption();
	System::dispatch();
	//System::enablePreemption();
}

void Thread::signal(SignalId id){
	myPCB->signal(id);
}

void Thread::signal0Handler(){
	//this handler is called inside timer while handling processes so if running has signal 0 it has to be KILLED

	PCB::signal0Handler();

}

void Thread::signal2Handler(){
	myPCB->signal2Handler();
}


void Thread::unblockSignal(SignalId signal) {
	myPCB->unblockSignal(signal);
}

void Thread::blockSignal(SignalId signal) {
	myPCB->blockSignal(signal);
}

void Thread::unblockSignalGlobally(SignalId signal) {
	PCB::unblockSignalGlobally(signal);
}

void Thread::blockSignalGlobally(SignalId signal) {
	PCB::blockSignalGlobally(signal);
}

void Thread::unregisterAllHandlers(SignalId id){
	myPCB->unregisterAllHandlers(id);
}

void Thread::swap(SignalId id, SignalHandler hand1, SignalHandler hand2) {
	myPCB->swap(id,hand1,hand2);
}

void Thread::registerHandler(SignalId signal, SignalHandler handler){
	myPCB->registerHandler(signal,handler);
}

void Thread::processSignals(){
	myPCB->processSignals();
}




