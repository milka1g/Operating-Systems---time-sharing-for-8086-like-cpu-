/*
 * PCB.h
 *
 *  Created on: Aug 3, 2019
 *      Author: OS1
 */

#ifndef PCB_H_
#define PCB_H_

#include "thread.h"
#include "System.h"
#include "queue.h"

typedef enum{NEW, READY, RUNNING, BLOCKED, OVER, KILLED} Status;
typedef enum{LIMITED, UNLIMITED, NONE} blockedType;

typedef unsigned SignalId;
typedef void (*SignalHandler)();

class PCB{
private:

	static ID _id; //int 2 bajta
	ID id;

	StackSize stackSize;   //unsigned long 4B
	Time timeSlice, waitOnSem; //unsigned int 2B

	Status status;
	Thread *myThr;
	blockedType blockType;
	unsigned int *myStack;
	volatile unsigned int sp, ss, bp; //16bit registers 2B int enough

	volatile unsigned int sig1;

		Thread* parent;

				struct SignalIdNode{
					SignalId id;
					SignalIdNode* next;
					SignalIdNode(SignalId _id){
						id = _id;
						next = 0;
					}
				};

				struct SignalHandlerNode{
					SignalHandler handler;
					SignalHandlerNode* next;
					SignalHandlerNode(SignalHandler h){
						handler = h;
						next = 0;
					}
				};

				SignalIdNode* allSignalsHead, *allSignalsTail; // all arrived signals
				SignalHandlerNode* handlerHeads[16], *handlerTails[16]; // pointer to array of 16 signalhandlernodes
				static int blockedSignalsGlobally[16]; //signals blocked for every instance of this class
				int blockedSignals[16]; //signals blocked in one particular thread
				void signal2Handler(); //when thread finishes it gets this signal from system

protected:

	friend class Thread;
	friend class idleThread;
	friend class Queue;
	friend class System;
	friend class TestThread;
	friend class KernelSem;
	friend class KernelEv;


public:
	Queue* waitingThreads; //threads waiting for this thread to finish
	PCB(Thread *myTh, StackSize ssize, Time tslice, int type = 1);
	void initStack();
	static void wrapper();
	~PCB();

	ID getId() const {
		return id;
	}

	Thread* getMyThr() const {
		return myThr;
	}

	StackSize getStackSize() const {
		return stackSize;
	}

	Status getStatus() const {
		return status;
	}

	void setStatus(Status status) {
		this->status = status;
	}


	Time getTimeSlice() const {
		return timeSlice;
	}

	void signal(SignalId signal);
	void registerHandler(SignalId signal, SignalHandler handler);
	void unregisterAllHandlers(SignalId id);
	void swap(SignalId id, SignalHandler hand1, SignalHandler hand2);

	void blockSignal(SignalId signal);
	static void blockSignalGlobally(SignalId signal);
	void unblockSignal(SignalId signal);
	static void unblockSignalGlobally(SignalId signal);

	static void signal0Handler(); //this kills thread
	void processSignals(); // do the signal handling in timer after selecting thread from scheduler


};



#endif /* PCB_H_ */
