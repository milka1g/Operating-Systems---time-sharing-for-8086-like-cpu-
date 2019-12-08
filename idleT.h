/*
 * idleT.h
 *
 *  Created on: Aug 4, 2019
 *      Author: OS1
 */

#ifndef IDLET_H_
#define IDLET_H_

#include "thread.h"

class idleThread: public Thread{
public:
	idleThread():Thread(1024,1,0){}
protected:
	void run();
	void start();
};



#endif /* IDLET_H_ */
