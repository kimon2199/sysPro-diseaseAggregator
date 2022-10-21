#include "workerNode.h"
#include <iostream>
#include <stdlib.h>

WorkerNode::WorkerNode(){
    pid = 0;
    writefd = 0;
    readfd = 0;
	next = NULL;
    fifoIndex = NULL;
} 

void WorkerNode::setNext(WorkerNode* n){
    this->next = n;
}

int WorkerNode::setReadfd(int a){
    return (this->readfd = a);
}

int WorkerNode::setWritefd(int a){
    return (this->writefd = a);
}

int WorkerNode::setPid(int a){
    return (this->pid = a);
}

void WorkerNode::setFifo(char* a){
    this->fifoIndex = (char*)malloc((strlen(a)+1)*sizeof(char));
    strcpy(this->fifoIndex,a);
}

WorkerNode* WorkerNode::getNext(){
    return next;
}

int WorkerNode::getPid(){
    return pid;
}

int WorkerNode::getReadfd(){
    return readfd;
}

int WorkerNode::getWritefd(){
    return writefd;
}

void WorkerNode::print(){
    std::cout << "pid: " << pid << '\n';
    std::cout << "writefd: " << writefd << '\n';
    std::cout << "readfd: " << readfd << '\n';
    if (fifoIndex)
        std::cout << "fifoInd: " << fifoIndex << '\n';
    countries.printList(0);
}

int WorkerNode::countriesCount(){
    return countries.getLength();
}

StringNode* WorkerNode::getFirstCountry(){
    return countries.getHead();
}

void WorkerNode::addCountry(char* c){
    countries.addNode(new StringNode(c));
}

bool WorkerNode::hasCountry(char* c){
    for (StringNode* i = countries.getHead(); i; i = i->getNext())
        if (!strcmp(c,i->getString()))
            return true;
    return false;
}

WorkerNode::~WorkerNode(){
    free(fifoIndex);
} 
