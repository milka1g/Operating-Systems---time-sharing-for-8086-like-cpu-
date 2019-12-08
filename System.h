/*
 * System.h
 *
 *  Created on: Aug 3, 2019
 *      Author: OS1
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_

#include "thread.h"
#include "idleT.h"
#include "kernele.h"
#include "PCB.h"
//#include "queue.h"

class Queue;
class Thread;

typedef void interrupt(*pInterruptR)(...);
typedef enum {YES, NO} InterruptRequested;
typedef enum {Y, N} processSignals;


class System{
public:
	static pInterruptR oldRoutine;
	static volatile PCB* running, *startingPCB;
	static Thread *starting, *runningThread;
	static KernelEv *kernelEvents[256];
	static Thread* idle;
	static Queue* allThreads, *finishedThreads;
	static volatile int counter; // counts TimeSlice of running thread

	static void init();
	static void restore();
	static void interrupt timer(...);
	static void disablePreemption();
	static void enablePreemption();
	static void dispatch();

protected:
	friend class Queue;
	friend class Thread;
	friend class PCB;
	friend class idleThread;
	friend class Event;

private:
	static volatile InterruptRequested interruptRequested; // to request explicitly interrupt and for pending interrupts
	static volatile processSignals processingSignals;
	static volatile int locker; // for avoiding context switch without disabling interrupts


};

#endif /* SYSTEM_H_ */
