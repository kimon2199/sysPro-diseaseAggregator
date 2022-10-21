#include "reportNode.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <unistd.h> 
#include <fcntl.h> 

ReportNode::ReportNode(){
    ztot = 0;
    ttof = 0;
    ftos = 0;
    sp = 0;
    disease = NULL;
}

ReportNode::ReportNode(char* st,char* i){
	this->disease = (char*)malloc((strlen(st)+1)*sizeof(char));
	strcpy(this->disease,st);
    ztot = 0;
    ttof = 0;
    ftos = 0;
    sp = 0;
    this->next = 0; //NULL
    incr(i);
} 

void ReportNode::setNext(ReportNode* n){
    this->next = n;
}

void ReportNode::setReport(char* n){
    this->disease = n;
}

ReportNode* ReportNode::getNext(){
    return next;
}

char* ReportNode::getNextKeyOfNode(char* key){
	if (key==NULL)
		return disease;
	return NULL;
}


char* ReportNode::getReport(){
	return disease;
}

void ReportNode::print(){
    if (!disease)
        return;
    std::cout << strlen(disease)+1 << '$' << disease << std::endl;
    std::cout << 22+countD(ztot)+7 << '$' << "Age range 0-20 years: ";
    std::cout << ztot << " cases" << std::endl;
    std::cout << 23+countD(ttof)+7 << '$' << "Age range 21-40 years: ";
    std::cout << ttof << " cases" << std::endl;
    std::cout << 23+countD(ftos)+7 << '$' << "Age range 41-60 years: ";
    std::cout << ftos << " cases" << std::endl;
    std::cout << 21+countD(sp)+7 << '$' << "Age range 60+ years: ";
    std::cout << sp << " cases" << std::endl;
}

int ReportNode::printTopK(int k,int fd){
    if (!(ztot+ttof+ftos+sp))
        return 1;

    /* changing from std out to sockSt */
    int stdoutBack = dup(1);
    close(1);
    dup2(fd, 1);

    int zA = ztot*100 /(ztot+ttof+ftos+sp);
    int tA = ttof*100 /(ztot+ttof+ftos+sp);
    int fA = ftos*100 /(ztot+ttof+ftos+sp);
    int sA = sp*100 /(ztot+ttof+ftos+sp);
    bool zP = false;
    bool tP = false;
    bool fP = false;

    if (k > 4)
        k = 4;

    while (k > 0){
        if ((zA >= tA) && (zA >= fA) && (zA >= sA) && !zP){
            std::cout << countD(zA)+8 << '$' << "0-20: "<< zA <<"%\n";
            zA = 0;
            zP = true;
        }
        else if ((tA >= zA) && (tA >= fA) && (tA >= sA) && !tP){
            std::cout << countD(tA)+9 << '$' << "21-40: "<< tA <<"%\n";
            tA = 0;
            tP = true;
        }
        else if ((fA >= tA) && (fA >= zA) && (fA >= sA) && !fP){
            std::cout << countD(fA)+9 << '$' << "41-60: "<< fA <<"%\n";
            fA = 0;
            fP = true;
        }
        else {
            std::cout << countD(sA)+7 << '$' << "60+: "<< sA <<"%\n";
            sA = 0;
        }
        k--;
    }
    /* changing back to std out*/
    dup2(stdoutBack, 1);
    return 0;
}


void ReportNode::incr(char* i){
    int b = atoi(i);
    if (b<21)
        ztot++;
    else if (b<41)
        ttof++;
    else if (b<61)
        ftos++;
    else
        sp++;
}

int ReportNode::countD(int n) { 
    return n ? std::floor(log10(n) + 1) : 1; 
} 

ReportNode::~ReportNode(){
    if (disease)
        free(disease);
}
