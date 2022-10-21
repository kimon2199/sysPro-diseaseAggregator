#include <iostream>
#include <cstring>

template<typename T>
LinkedList<T>::LinkedList(){
	first = NULL;
	last = NULL;
	length = 0;
}

template<typename T>
void LinkedList<T>::addNode(T* g){
	length++;
	if (!first){
		first = g;
		last = g;
		return ; 
	}
	last->setNext(g);
	last = g;
}

template<typename T>
T* LinkedList<T>::getNode(){
	return 0;
}

template<typename T>
T* LinkedList<T>::getHead(){
	return first;
}

template<typename T>
T* LinkedList<T>::getLast(){
	return last;
}

template<typename T>
T* LinkedList<T>::getNodeWithKey(char* key){
	T* curNode = first;
	while(!keyIsInNode(key,curNode) && curNode)
		curNode = curNode->getNext();
	return curNode;
}

template<typename T>
bool LinkedList<T>::keyIsInNode(char* key,T* node){
	if(!node)
		return 0;
	else{
		char* oneOfTheKeys = node->getNextKeyOfNode(NULL);
		while (oneOfTheKeys){
			if (!strcmp(key,oneOfTheKeys))
				break;
			oneOfTheKeys = node->getNextKeyOfNode(oneOfTheKeys);
		}
		return oneOfTheKeys ? 1 : 0;
	}
}

template<typename T>
LinkedList<T>::~LinkedList(){
	T* tmp = first;
	T* nx;
	while(tmp){
		nx = tmp->getNext();
		delete(tmp);
		tmp = nx;
	}
}

template<typename T>
void LinkedList<T>::printList(bool flag){
	T* tmp = first;
	while(tmp){
		tmp->print();
		tmp = tmp->getNext();
		if (flag) 
			std::cout << "         |\n         |\n         v\n";
	}
	if (flag) 
		std::cout << "       +---+\n       |END|\n       +---+\n";
}

template<typename T>
void LinkedList<T>::sortList(){
	bool isSorted = false;
	while(!isSorted) {
    	isSorted = true;
    	T* cur = first;         

    	while(cur->getNext() != NULL) {
        	T* next = cur->getNext();
        	if(cur->less(next,cur)) {
        		cur->swap(next,cur);
            	isSorted = false;
        	}

        	cur = cur->getNext();
    	}       
	}
}

template<typename T>
bool LinkedList<T>::incr(char* key,char* age){
	T* node = getNodeWithKey(key);
	if (node == NULL)
		return 0;
	node->incr(age);
	return 1;
}

template<typename T>
void LinkedList<T>::removeNode(T* tbr){
	length--;
	if (tbr == first){
		first = first->getNext();
		delete(tbr);
		return;
	}
	T* prevNode = first;
	T* curNode = first->getNext();
	while( tbr != curNode && curNode){
		curNode = curNode->getNext();
		prevNode = prevNode->getNext();
	}
	if (curNode == last){
		last = prevNode;
	}
	if (curNode){
		prevNode->setNext(curNode->getNext());
		delete(tbr);
	}
}

template<typename T>
int LinkedList<T>::getLength(){
	return length;
}



