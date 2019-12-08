/*
 * thread.
 *
 *  Created on: Aug 1, 2019
 *      Author: OS1
 */
#ifndef THREAD_H_
#define THREAD_H_

typedef unsigned long StackSize;
const StackSize defaultStackSize = 4096, maxStackSize = 65535;
typedef unsigned int Time; // time, x 55ms
const Time defaultTimeSlice = 2; // default = 2*55ms
typedef int ID;

typedef unsigned SignalId;
typedef void (*SignalHandler)();

class PCB; // Kernel's implementation of a user's thread

class Thread {
public:
	void start();
	void waitToComplete();
	virtual ~Thread();

	ID getId();
	static ID getRunningId();
	static Thread * getThreadById(ID id);

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

protected:
	friend class PCB;
	friend class System;
	friend class idleThread;
	Thread (StackSize stackSize = defaultStackSize, Time timeSlice = defaultTimeSlice);
	virtual void run() {}

private:
	Thread(StackSize stackSize, Time timeSlice, int type);
	PCB* myPCB;
	Thread* parentT;

	void signal2Handler(); //when thread finishes it gets this signal from system


};

void dispatch();

#endif /* THREAD_H_ */
