/*
 * ivtentry.h
 *
 *  Created on: Aug 12, 2019
 *      Author: OS1
 */

#ifndef IVTENTRY_H_
#define IVTENTRY_H_

#include "event.h"

class KernelEv;
typedef void interrupt (*pInterruptR)(...);


#define PREPAREENTRY(num,old)\
	void interrupt newRoutine##num(...);\
	IVTEntry ivtEntry##num = IVTEntry(num,newRoutine##num);\
	void interrupt newRoutine##num(...){\
		ivtEntry##num.signal();\
		if (old == 1) ivtEntry##num.oldRoutine();\
	}


class IVTEntry{
public:
	IVTEntry(IVTNo no, pInterruptR interrupt);
	~IVTEntry();
	void signal();
	pInterruptR oldRoutine;
	static IVTEntry* IVT[256];
	static IVTEntry* getEntry(IVTNo i);
private:
	friend class KernelEv;
	IVTNo ivtNo;
};



#endif /* IVTENTRY_H_ */
