

#ifndef KERNELE_H_
#define KERNELE_H_

class PCB;
typedef unsigned char IVTNo;


class KernelEv {
public:
	KernelEv(IVTNo ivtNo);
	~KernelEv ();
	void wait ();
	void signal();
private:
	friend class IVTEntry;
	friend class Event;
	IVTNo ivtNo;
	int value;
	PCB* holder;
	PCB* blocked;

};


#endif /* KERNELE_H_ */
