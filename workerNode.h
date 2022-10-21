#include "linkedList.h"
#include "stringNode.h"

class WorkerNode{
	int pid;
	int writefd;
	int readfd;
	char* fifoIndex;
	LinkedList<StringNode> countries;
	WorkerNode* next;
public:
	WorkerNode();
	void setNext(WorkerNode*);
	int setReadfd(int);
	int setWritefd(int);
	int setPid(int);
	void setFifo(char*);
	int countriesCount();
	WorkerNode* getNext();
	StringNode* getFirstCountry();
	int getPid();
	int getReadfd();
	int getWritefd();
	void print();
	void addCountry(char*);
	bool hasCountry(char*);
	~WorkerNode();
};
