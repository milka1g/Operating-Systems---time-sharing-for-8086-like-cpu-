/*
 * list.cpp
 *
 *  Created on: Aug 10, 2019
 *      Author: OS1
 */


#include "list.h"
#include "System.h"
#include "kersem.h"

List::List(){
	head = 0;
	size = 0;
	tail = 0;
}

void List::add(KernelSem* ks){
	System::disablePreemption();
		if(head == 0) head = tail = new Node(ks);
		else tail = tail->next = new Node(ks);
		size++;
	System::enablePreemption();
}

void List::remove(KernelSem* ks){
	System::disablePreemption();
	Node *cur = head, *prev=0;
		while(cur && cur->kernelSem != ks) {
			prev = cur;
			cur = cur->next;
		}
		if(!cur) return;
		if (prev) prev->next = cur->next;
		else head = cur->next;
		if(cur == tail) tail = prev;


	System::enablePreemption();
	delete cur;
}

void List::time(){ //go through all sems and decrement wait times od their threads
	Node* tmp = head;
	System::disablePreemption();
	while(tmp){
		int incr = tmp->kernelSem->limitedWait->timepassQ(); //important to call it on this limitedWait list
		tmp->kernelSem->value+=incr;
		tmp = tmp->next;
	}
	System::enablePreemption();

}

List::~List(){
	System::disablePreemption();
		Node* tmp = head;
			while (tmp) {
				Node* old = tmp;
				tmp = tmp->next;
				delete old->kernelSem;
				delete old;
			}
		head = tail = 0;
	System::enablePreemption();
}

