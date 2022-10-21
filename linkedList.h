#ifndef LINKEDLIST_H
#define LINKEDLIST_H

template <class T>
class LinkedList{
	T* first;
	T* last;
	int length;
public:
	LinkedList();
	void addNode(T*);
	T* getNode();
	T* getHead();
	T* getLast();
	T* getNodeWithKey(char*);
	bool keyIsInNode(char*,T*);
	~LinkedList();
	void printList(bool i);
	void sortList();
	bool incr(char*,char*);
	void removeNode(T*);
	int getLength();
};

#include "linkedList.tpp"

#endif