/*
 * event.cpp
 *
 *  Created on: Aug 12, 2019
 *      Author: OS1
 */

#include "event.h"
#include "ivtentry.h"
#include "kernele.h"
#include "System.h"

Event::Event(IVTNo ivtNo) {
	System::disablePreemption();
	myImpl = new KernelEv(ivtNo);
	System::enablePreemption();
}

Event::~Event() {
	System::disablePreemption();
	delete myImpl;
	myImpl = 0;
	System::enablePreemption();
}

void Event::wait() {
	//if(System::kernelEvents[myImpl->ivtNo]==0) cout << "0 BAKIIIII";
	 if(myImpl != 0)
		 myImpl->wait();
}

void Event::signal() {
	//if(System::kernelEvents[myImpl->ivtNo]==0) cout << "0 BAKIIIII";
	if(myImpl != 0)
		myImpl->signal();
}






