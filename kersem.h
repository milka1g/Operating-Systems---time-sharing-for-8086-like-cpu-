/*
 * kersem.h
 *
 *  Created on: Aug 9, 2019
 *      Author: OS1
 */

#ifndef KERSEM_H_
#define KERSEM_H_

#include "semaphor.h"
#include "list.h"

class Queue;

class KernelSem{
public:
	KernelSem(int init);
	~KernelSem();
	int wait (Time maxTimeToWait);
	int signal(int n);
	int val () const { return value; }
	static List* allSemaphores;
	static void timePass();
	void unblock();
protected:
	friend class Semaphore;
	friend class PCB;
	friend class List;
private:
	int value;
	Queue* limitedWait, *unlimitedWait;
	//*limitedWait; // threads blocked on this semaphore
};



#endif /* KERSEM_H_ */
