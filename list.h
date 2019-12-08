/*
 * list.h
 *
 *  Created on: Aug 10, 2019
 *      Author: OS1
 */

#ifndef LIST_H_
#define LIST_H_

class KernelSem;

class List{
public:
	List();
	void add(KernelSem* ks);
	void remove(KernelSem* ks);
	int getSize() const {return size;}
	void time();
	~List();
protected:
	friend class KernelSem;
private:
	struct Node{
		KernelSem* kernelSem;
		Node* next;
		Node(KernelSem* ks){
			kernelSem = ks;
			next = 0;
		}
	};

	Node* head, *tail;
	int size;
};



#endif /* LIST_H_ */
