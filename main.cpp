/*
 * main.cpp
 *
 *  Created on: Aug 6, 2019
 *      Author: OS1
 */

#include <iostream.h>
#include "queue.h"
#include "PCB.h"
#include <stdio.h>
#include <stdlib.h>
#include "thread.h"

extern int userMain(int argc, char* argv[]);

int main(int argc, char* argv[]){


	System::init();
	int ret = userMain(argc,argv);
	System::disablePreemption();
	printf("User main return value = %d\n", ret);
	System::enablePreemption();
	System::restore();
	System::disablePreemption();
	printf("main->exit");
	System::enablePreemption();
	return ret;


}
