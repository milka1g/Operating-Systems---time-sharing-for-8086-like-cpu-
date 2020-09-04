/*
 * idleT.cpp
 *
 *  Created on: Aug 4, 2019
 *      Author: OS1
 */

#include "PCB.h"
#include "idleT.h"
#include <iostream.h>

void idleThread::run(){
	while(1<2){
		//System::disablePreemption();
		//cout <<".";
		//System::enablePreemption();
	}
}

void idleThread::start(){
	if(myPCB->status == NEW){
		myPCB->status = READY;
		myPCB->initStack();
	}

}

