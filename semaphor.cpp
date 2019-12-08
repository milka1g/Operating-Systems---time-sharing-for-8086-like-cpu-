/*
 * semaphor.cpp
 *
 *  Created on: Aug 9, 2019
 *      Author: OS1
 */

#include "semaphor.h"


#include "semaphor.h"
#include "kersem.h"
#include "System.h"


Semaphore::Semaphore(int init) {
	System::disablePreemption();
	myImpl=new KernelSem(init);
	System::enablePreemption();
}

Semaphore::~Semaphore () {
	System::disablePreemption();
	delete myImpl;
	myImpl=0;
	System::enablePreemption();
}

int Semaphore::wait (Time maxTimeToWait) {
	return myImpl->wait(maxTimeToWait);
}

int Semaphore::signal(int n) {
	return myImpl->signal(n);
}

int Semaphore::val () const {
	return myImpl->val();
}
