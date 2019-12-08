#include "queue.h"
#include "SCHEDULE.H"
#include "System.h"
#include <iostream.h>


Queue::Queue(){
	front = 0;
	size = 0;
	rear = 0;
}

Queue::~Queue() {
	size = 0;
	Node* tmp = front;
	front = rear = 0;
	while (tmp != 0) {
		Node* old = tmp;
		tmp = tmp->next;
		delete old;
	}
}

void Queue::enQueue(PCB* pcb) {
	System::disablePreemption();
	if (front == 0) front = rear = new Node(pcb);
	else rear = rear->next = new Node(pcb);
	size++;
	//cout << "size je " << size << endl;
	System::enablePreemption();
}

PCB* Queue::deQueue() {
	PCB* ret = 0;
	if (front != 0) {
		System::disablePreemption();
		Node* old = front;
		size--;
		//cout << "size je " << size << endl;
		front = front->next;
		if (front == 0) rear = 0;
		ret = old->pcb;
		delete old;
		System::enablePreemption();
	}
	return ret;
}

PCB* Queue::findPCBbyID(ID id) {
	Node* tmp = front;
	while(tmp!=0){
		if(tmp->pcb->id == id)
			return tmp->pcb;
		tmp = tmp->next;
	}
	return 0;
}

PCB* Queue::deletePCB(ID id) {
	System::disablePreemption();
	PCB* ret = 0;
	Node *tmp = front, *prev = 0;
	while(tmp!=0){
		if((tmp->pcb!=0) && (tmp->pcb->id == id)){
			if(prev){
				prev->next = tmp->next;
			} else {
				front = front->next;
				if(front == 0) rear = 0;
			}
			ret = tmp->pcb;
			delete tmp;
			size--;
			//cout << "size je " << size << endl;
			break;
		}
		prev = tmp;
		tmp = tmp->next;
	}
	System::enablePreemption();
	return ret;
}

int Queue::timepassQ() { //called on limitedWait only!
	Node* cur = front;
	PCB* pcbready = 0;
	int ret = 0;  //this much threads was woke up because time got to 0
	int j = size;
	for (int i = 0; i < j; i++) {
		if(cur->pcb->waitOnSem > 0) cur->pcb->waitOnSem--;
		if(cur->pcb->waitOnSem == 0) {
			System::disablePreemption();
			ret++;
			//cout << "size je " << size << endl;
			pcbready = deletePCB(cur->pcb->id);
			pcbready->status = READY; //blockType flag remains the same, it gets set to NONE in wait after dispatch
			Scheduler::put(pcbready);
			System::enablePreemption();
		}
		//System::enablePreemption();
		cur = cur->next;
	}
	return ret;
}
