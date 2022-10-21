#include <iostream>
#include <cstdlib>
#include "circleBuffer.h"


CircleBuffer::CircleBuffer(int sz,pthread_cond_t* cne,pthread_cond_t* cnf){
	pthread_mutex_init(&mtx, 0);
	this->start = 0;
	this->end = -1;
	this->count = 0;
	this->size = sz;
	this->data = (int*)malloc(size*sizeof(int));
	this->cond_nonempty = cne;
	this->cond_nonfull = cnf;

}

void CircleBuffer::place(int dato){
	pthread_mutex_lock(&mtx);
	while (this->count >= this->size) {
		//printf(">> Found Buffer Full \n");
		pthread_cond_wait(this->cond_nonfull, &mtx);
		}
	this->end = (this->end + 1) % this->size;
	this->data[this->end] = dato;
	this->count++;
	pthread_mutex_unlock(&mtx);
}

int CircleBuffer::obtain(){
	int data = 0;
	pthread_mutex_lock(&mtx);
	while (this->count <= 0) {
		//printf(">> Found Buffer Empty \n");
		pthread_cond_wait(this->cond_nonempty, &mtx);
		}
	data = this->data[this->start];
	this->start = (this->start + 1) % this->size;
	this->count--;
	pthread_mutex_unlock(&mtx);
	return data;
}

CircleBuffer::~CircleBuffer(){
	pthread_mutex_destroy(&mtx);
	free(this->data);
}