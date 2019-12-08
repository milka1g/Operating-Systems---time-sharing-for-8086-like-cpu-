/*
 * queue.h
 *
 *  Created on: Aug 3, 2019
 *      Author: OS1
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include "PCB.h"
#include "thread.h"
//#include "MACROS.h"
#include "iostream.h"

class Queue{
private:

	struct Node{
		PCB* pcb;
		Node* next;
		Node(PCB* _pcb){
			next = 0;
			pcb=_pcb;
		}
	}*front, *rear;

	unsigned int size;
protected:
	friend class PCB;
	friend class Thread;
	friend class System;
	friend class idleThread;
	friend class KernelSem;

public:

	Queue();

	void enQueue(PCB* pcbb);
	PCB* deQueue();

	PCB* deletePCB(ID id);
	PCB* find(ID);
	PCB* findPCBbyID(ID id);
	//Thread* findThreadByID(ID id); //overkill

	int getSize(){ return size; }

	void printQ();

	int timepassQ();

	~Queue();
};



#endif /* QUEUE_H_ */
