#include "stringNode.h"
#include <stdlib.h>
#include <string.h>
#include <iostream>

StringNode::StringNode(char* st){
	this->str = (char*)malloc((strlen(st)+1)*sizeof(char));
	strcpy(this->str,st);
    this->next = 0; //NULL
} 

void StringNode::setNext(StringNode* n){
    this->next = n;
}

void StringNode::setString(char* n){
    this->str = n;
}

StringNode* StringNode::getNext(){
    return next;
}

char* StringNode::getNextKeyOfNode(char* key){
	/*if (key==NULL)
		return str->getID();*/
	return NULL;
}


char* StringNode::getString(){
	return str;
}

void StringNode::print(){
    std::cout << str << std::endl;
}

StringNode::~StringNode(){
    free(str);
}

bool StringNode::less(StringNode* ll,StringNode* rr){
	char* l = ll->getString();
	char* r = rr->getString();
	if (l[6]<r[6])
        return 1;
    else if (l[6]==r[6]){
        if (l[7]<r[7])
            return 1;
        else if (l[7]==r[7]){
            if (l[8]<r[8])
                return 1;
            else if (l[8]==r[8]){
                if (l[9]<r[9])
                    return 1;
                else if (l[9]==r[9]){
                    if (l[3]<r[3])
                        return 1;
                    else if (l[3]==r[3]){
                        if (l[4]<r[4])
                            return 1;
                        else if (l[4]==r[4]){
                            if (l[0]<r[0])
                                return 1;
                            else if (l[0]==r[0]){
                                if (l[1]<r[1])
                                    return 1;
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

void StringNode::swap(StringNode* ll,StringNode* rr){
	char* tmp = ll->getString();
	ll->setString(rr->getString());
	rr->setString(tmp);
}

