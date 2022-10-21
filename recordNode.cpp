#include "recordNode.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>

RecordNode::RecordNode(char* id,char* f,char* l,char* d,char* a,char* dt){
	this->id = (char*)malloc((strlen(id)+1)*sizeof(char));
	strcpy(this->id,id);
    this->fn = (char*)malloc((strlen(f)+1)*sizeof(char));
    strcpy(this->fn,f);
    this->ln = (char*)malloc((strlen(l)+1)*sizeof(char));
    strcpy(this->ln,l);
    this->disease = (char*)malloc((strlen(d)+1)*sizeof(char));
    strcpy(this->disease,d);
    this->age = (char*)malloc((strlen(a)+1)*sizeof(char));
    strcpy(this->age,a);
    this->inDate = (char*)malloc((strlen(dt)+1)*sizeof(char));
    strcpy(this->inDate,dt);
    this->outDate = (char*)malloc((strlen("--")+1)*sizeof(char));
    strcpy(this->outDate,"--");
    this->next = 0; //NULL
} 

void RecordNode::setNext(RecordNode* n){
    this->next = n;
}

RecordNode* RecordNode::getNext(){
    return next;
}

char* RecordNode::getInDate(){
    return inDate;
}

char* RecordNode::getOutDate(){
    return outDate;
}

void RecordNode::setOutDate(char* dt){
    free(outDate);
    this->outDate = (char*)malloc((strlen(dt)+1)*sizeof(char));
    strcpy(this->outDate,dt);
}

char* RecordNode::getId(){
    return id;
}

char* RecordNode::getAge(){
    return age;
}

char* RecordNode::getVir(){
    return disease;
}

char* RecordNode::getNextKeyOfNode(char* key){
	if (key==NULL)
		return id;
	return NULL;
}

void RecordNode::print(){
    std::cout << id << ' ' << fn << ' ' << ln << ' ' << disease;
    std::cout << ' ' << age << ' ' << inDate << ' ' << outDate << std::endl;
}

int RecordNode::getNumOfCharsInPrint(){
    return strlen(id) + strlen(fn) + strlen(ln) + strlen(disease) + strlen(age) + strlen(inDate) + strlen(outDate) + 7;
}

bool RecordNode::same(char* f,char* l,char* d,char* a){
    return !strcmp(fn,f) && !strcmp(ln,l) && !strcmp(disease,d) && !strcmp(age,a);
    
}

RecordNode::~RecordNode(){
    free(id);
    free(fn);
    free(ln);
    free(disease);
    free(age);
    free(inDate);
    free(outDate);
}

bool RecordNode::less(char* l,char* r){
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
