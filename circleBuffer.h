#include <pthread.h>
#include <unistd.h>

typedef struct {
	
} pool_t;

class CircleBuffer {
	int* data;
	int start;
	int end;
	int count;
	int size;
	pthread_mutex_t mtx;
	pthread_cond_t* cond_nonempty;
	pthread_cond_t* cond_nonfull;
public:
	CircleBuffer(int,pthread_cond_t*,pthread_cond_t*);
	void place(int);
	int obtain(); 
	~CircleBuffer();
} ;