#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include "iParser.h"
#include "circleBuffer.h"
#include "workerNode.h"
#include <cstdio>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#include <ifaddrs.h> //
#define PRINT_STATS false

void perror_exit(char*);
void* question_serving(void*);
void* accepting_questions(void*);
char* protocolRead(int);
void protocolReadCountries(int,WorkerNode*);
void protocolReadPrintStats(int,WorkerNode*);
void protocolWrite(int,int);
void protocolWrite(int,char*);

void diseaseFrequency(char*,char*,char*,char*,int);
void diseaseFrequency(char*,char*,char*,int);
void topkAgeRanges(char*,char*,char*,char*,char*,int);
void searchPatientRecord(char*,int);
int readOneResponse(int);
void printWorkerResponse(int,WorkerNode*);
void operations56(char*,char*,char*,char*,char*,int);
WorkerNode* getWorkerForCountry(char*);
WorkerNode* getWorkerWithReadfd(int);
void setPfds(int);

CircleBuffer* cirBuff;
int queryPortNum;
LinkedList<WorkerNode> workerList;
int nfds;
struct pollfd *pfds;

pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;
pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;


int main(int argc, char* argv[]){
	int statisticsPortNum, numThreads, bufferSize;

	//Parsing the comandline input
	if(inputParserS(&queryPortNum,&statisticsPortNum,&numThreads,&bufferSize,argc,argv)){
        std::cerr << "Error in input\n";
		return 1;
	}

	////////////////////////////////////////////////
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
        }
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
    ////////////////////////////////////////////////////

	pthread_cond_init(&cond_nonempty, 0);
	pthread_cond_init(&cond_nonfull, 0);
	/* creating circle buffer */
	cirBuff = new CircleBuffer(bufferSize,&cond_nonempty,&cond_nonfull);

	/* Creating the threads */
	pthread_t* threads = (pthread_t*)malloc(numThreads*sizeof(pthread_t));
	for(int i = 0; i<numThreads; i++)
		pthread_create(&threads[i], NULL, question_serving, NULL);

	/* Creating thread for receiving questions */
	pthread_t qThread;
	pthread_create(&qThread, NULL, accepting_questions, (void*)&bufferSize);

	struct sockaddr_in server, client;
	socklen_t  clientlen, serverOthlen; //give this a value???
	struct sockaddr *serverptr =(struct sockaddr  *)&server;
	struct sockaddr *clientptr =(struct sockaddr  *)&client;
	struct hostent *rem;
	int sockSt, new_socket;

	/* making the socket to receive statistics */
	if((sockSt = socket(AF_INET, SOCK_STREAM, 0)) ==  -1){
		std::cout << 559 << std::endl;
		perror("Socket  creation  failed!");
	}

	server.sin_family = AF_INET; 
    server.sin_addr.s_addr = htonl(INADDR_ANY); 
    server.sin_port = htons( statisticsPortNum ); 

	/* binding sockSt to address */
	if (bind(sockSt, serverptr, sizeof(server)) < 0)
		perror_exit((char*)"bind");

	/*  Listen  for  connections  */
	if(listen(sockSt , 5) < 0)  
		perror_exit((char*)"listen");
	printf("Listening  for  connections  to port %d (for statistics)\n", statisticsPortNum);

	while(1){
		if ((new_socket = accept(sockSt, clientptr, &clientlen))<0) 
			perror_exit((char*)"accept");
	    printf("Accepted  connection (for statistics)\n");
	    WorkerNode* wr = new WorkerNode();

	    char* portNumber = protocolRead(new_socket);
	    std::cout << "Got this port number from client: " << portNumber << '\n';
	    int portNum = atoi(portNumber);
	    free(portNumber);
	    protocolReadCountries(new_socket,wr);
	    protocolReadPrintStats(new_socket,wr);

	    /* creating new socket to communicate with given port */
	    struct sockaddr_in serverOth;
		struct sockaddr *serverOthptr =(struct sockaddr  *)&serverOth;
		if((new_socket = socket(AF_INET , SOCK_STREAM , 0)) ==  -1)
			perror_exit((char*)"Socket  creation  failed!");
		/* Initiate connection */
		serverOth.sin_family = AF_INET;       /* Internet domain */
    	serverOth.sin_addr.s_addr = htonl(INADDR_ANY); /* Server IP */
    	serverOth.sin_port = htons(portNum);          /* Server port */
    	std::cout << "About to coonect to ip: " <<client.sin_addr.s_addr<< " port: " << serverOth.sin_port <<" ("<<portNum<<")"<< std::endl;
    	if (connect(new_socket, serverOthptr, sizeof(serverOth)) < 0)  
	   		perror_exit((char*)"connect");
		printf("Connecting to UNKNOWNIP port %d\n", portNum);

	    /* setting worker */
	    wr->setReadfd(new_socket);
	    wr->setWritefd(new_socket);
	    workerList.addNode(wr);

	    /* STEP 7: initializing class fields */
		nfds = workerList.getLength();
		free(pfds);
		pfds = (struct pollfd *)malloc(nfds*sizeof(struct pollfd));
		if (pfds == NULL)
			perror("malloc() failed");
		int j = 0;
		for (WorkerNode* wr = workerList.getHead(); wr; wr = wr->getNext()) {
			pfds[j].fd = wr->getReadfd();
			j++;
		}
	}

	return 0;
}

void perror_exit(char*message) {
	perror(message);
	exit(EXIT_FAILURE);
}

void* accepting_questions(void*m){
	struct sockaddr_in serverQ, client;
	socklen_t  clientlen; //give this a value???
	struct sockaddr *serverQptr =(struct sockaddr  *)&serverQ;
	struct sockaddr *clientptr =(struct sockaddr  *)&client;
	struct hostent *rem;
	int sockQ, new_socket;

	/* making the socket to receive questions */
	if((sockQ = socket(AF_INET, SOCK_STREAM, 0)) ==  -1)
		perror("Socket  creation  failed!");

	serverQ.sin_family = AF_INET; 
	serverQ.sin_addr.s_addr = htonl(INADDR_ANY); 
	serverQ.sin_port = htons( queryPortNum ); 

	/* binding sockSt to address */
	if (bind(sockQ, serverQptr, sizeof(serverQ)) < 0)
		perror_exit((char*)"bind");

	/*  Listen  for  connections  */
	if(listen(sockQ , 5) < 0)  
		perror_exit((char*)"listen");
	printf("Listening  for  connections  to port %d (for questions)\n", queryPortNum);

	while(1){
		if ((new_socket = accept(sockQ, clientptr, &clientlen))<0) 
			perror_exit((char*)"accept");

	    printf("Accepted  connection (for questions)\n");

	    cirBuff->place(new_socket);
	    pthread_cond_signal(&cond_nonempty); /* signaling for new item available */
	}
}

void* question_serving(void*){
	while(1){
		int socket = cirBuff->obtain();
		pthread_cond_signal(&cond_nonfull);

		char* com = protocolRead(socket);

		int argc = 0;
		char* argv[6];
	    std::istringstream streamm(com);
	    std::string arg; 
	    do {
	        streamm >> arg;
	      	if(streamm){
	        	argv[argc] = (char*)malloc((arg.length()+1)*sizeof(char));
	            if (argv[argc] == NULL)
	                perror("malloc() failed");
	        	strcpy(argv[argc],arg.c_str());
	        	argc++;
	        }
	    } while (streamm && argc < 6);
	    streamm >> arg;
	    if(streamm){
	    	pthread_mutex_lock(&print_lock);
	    	std::cout << '\n' << "Processing: " << com << '\n';
	    	protocolWrite(socket,(char*)"-diseaseAggregator: too many arguments\n");
	    	protocolWrite(socket,(char*)"$stop$");
	    	pthread_mutex_unlock(&print_lock);
	        for(int i=0; i<argc; i++)
	            free(argv[i]);
	        free(com);
	        continue;
	    }

	    pthread_mutex_lock(&print_lock);
		std::cout << '\n' << "Processing: " << com << '\n';
	        
	    /* find the command that was asked */
	    if (!strcmp("/diseaseFrequency",argv[0])){ //op2
	        if(!(argc == 5 || argc == 4)){
				protocolWrite(socket,(char*)"-/diseaseFrequency: wrong number of arguments\n");
	        }
	        else if (argc == 4)
	        	diseaseFrequency(argv[1],argv[2],argv[3],socket);
	        else
	            diseaseFrequency(argv[1],argv[2],argv[3],argv[4],socket);
	    }
	    else if (!strcmp("/topk-AgeRanges",argv[0])){ //op3
	        if(argc != 6){
	        	protocolWrite(socket,(char*)"-/topk-AgeRanges: wrong number of arguments\n");
	        }
	        else
	            topkAgeRanges(argv[1],argv[2],argv[3],argv[4],argv[5],socket);
	    }
	    else if (!strcmp("/searchPatientRecord",argv[0])){ //op4
	        if(argc != 2){
				protocolWrite(socket,(char*)"-/searchPatientRecord: wrong number of arguments\n");
	        }
	        else
	            searchPatientRecord(argv[1],socket);
	    }
	    else if (!strcmp("/numPatientAdmissions",argv[0])){ //op5
	        if(!(argc == 5 || argc == 4)){
	        	protocolWrite(socket,(char*)"-/numPatientAdmissions: wrong number of arguments\n");
	        }
	        else if (argc == 4)
	        	operations56((char*)"3$npa",argv[1],argv[2],argv[3],(char*)"-",socket);
	        else
	            operations56((char*)"3$npa",argv[1],argv[2],argv[3],argv[4],socket);
	    }
	    else if (!strcmp("/numPatientDischarges",argv[0])){ //op6
	        if(!(argc == 5 || argc == 4)){
				protocolWrite(socket,(char*)"-/numPatientDischarges: wrong number of arguments\n");
	        }
	    	else if (argc == 4)
	        	operations56((char*)"3$npd",argv[1],argv[2],argv[3],(char*)"-",socket);
	        else
	            operations56((char*)"3$npd",argv[1],argv[2],argv[3],argv[4],socket);
	    }
	    else{
			protocolWrite(socket,(char*)"-diseaseAggregator: ");
			protocolWrite(socket,argv[0]);
			protocolWrite(socket,(char*)": command not found\n");
	    }
	    protocolWrite(socket,(char*)"$stop$");
	    pthread_mutex_unlock(&print_lock);
	    for(int i=0; i<argc; i++)
	       	free(argv[i]);

	    free(com);
	}
    return NULL;
}

char* protocolRead(int readfd){
	int n;
	char buff[1];
	buff[0] = '0';
    int bytesToRead = 0;
    while (buff[0]!='$'){
        bytesToRead = 10*bytesToRead + buff[0] - '0';
        if ((n = read(readfd, buff, 1)) < 0) 
            perror("server: data read error \n");
    }
    /* make place to hold result */
    char* localBuffer = (char*)malloc((bytesToRead+1)*sizeof(char));
    if ((n = read(readfd, localBuffer, bytesToRead)) < 0) 
        perror("server: data read error \n");
    localBuffer[bytesToRead] = '\0';
    if (localBuffer[0]=='$'&&localBuffer[1]=='s'&&localBuffer[2]=='t'&&localBuffer[3]=='o'&&localBuffer[4]=='p'&&localBuffer[5]=='$')
    	return NULL;
    return localBuffer;
}

void protocolReadCountries(int new_socket,WorkerNode* wr){
	char* country;
	while(1){
		if ((country = protocolRead(new_socket))){
			std::cout << "Got this counry: " << country << '\n';
			wr->addCountry(country);
		}
		else
			break;
	}
	return;
}
void protocolReadPrintStats(int new_socket,WorkerNode* i){
	char* buff = (char*)malloc(43*sizeof(char));
    int n;
        while (1){
            buff[0] = '0';
            int bytesToRead = 0;
            while (buff[0]!='$'){
                bytesToRead = 10*bytesToRead + buff[0] - '0';
                if ((n = read(new_socket, buff, 1)) < 0) 
                    perror("server: data read error \n");
            }
            /* make place to hold result */
            char* localBuffer = (char*)malloc((bytesToRead+1)*sizeof(char));
            int ii = 0;
            /* read the argument */
            while (bytesToRead){
                int bytesIWillRead = (43 < bytesToRead) ? 43 : bytesToRead;
                if ((n = read(new_socket, buff, bytesIWillRead)) < 0)
                    perror("client: data read error \n");
                for (int j = 0; j < bytesIWillRead ; j++)
                    localBuffer[ii+j] = buff[j];
                bytesToRead -= bytesIWillRead;
                ii += bytesIWillRead;
            }
            localBuffer[ii] = '\0';
            /* check if end is reached */
            if(ii > 5)
                if (localBuffer[0]=='$' && localBuffer[1]=='s' && localBuffer[2]=='t' && localBuffer[3]=='o' && localBuffer[4]=='p' && localBuffer[5]=='$')
                    break;
            if (PRINT_STATS)
            	if (write(1, localBuffer, ii) != ii)    //fd 1 = stdout
                	perror("server: data write error \n"); 
        }
	return;
}

void protocolWrite(int wfd,int nmsg){
    /* turn int msg to string in order to send */
    std::ostringstream sl;
    sl << nmsg;
    protocolWrite(wfd,(char*)sl.str().c_str());
}

void protocolWrite(int wfd,char* msg){
    /* get message length */
    int l = strlen(msg);
    /* turn length to string in order to send */
    std::ostringstream sl;
        sl << l;
    /* get length of the string showing the msg length */
    int lol = strlen(sl.str().c_str());
    /* send data according to protocol */
    if (write(wfd, sl.str().c_str(), lol) != lol) perror("server: data write error \n"); 
    if (write(wfd, "$", 1) != 1) perror("server: data write error \n"); 
    if (write(wfd, msg, l) != l) perror("server: data write error \n"); 
}

//operations ----------------------------------------------

//operation 2
void diseaseFrequency(char* virusName,char* date1,char* date2,char* country,int sock){
    WorkerNode* worker = getWorkerForCountry(country);
    if (!worker){
        perror("No such country");
        return ;
    }
    /* sending command */
    int wr = worker->getWritefd();
    /* changing from std out to writefd */
    int stdoutBack = dup(1);
    close(1);
    if(dup2(wr, 1) < 0)
        perror("problem with dup2");

    std::cout << 5 ;
    std::cout << "2$df";
    std::cout << strlen(virusName) << '$' << virusName;
    std::cout << strlen(date1) << '$' << date1;
    std::cout << strlen(date2) << '$' << date2;
    std::cout << strlen(country) << '$' << country << std::flush;
        

    /* changing back to std out*/
    dup2(stdoutBack, 1);

    /* reading and summing each worker's response */
    int sum = 0;
    char buff[1];
    int n;
    while (1){
        buff[0] = '0';
        int bytesToRead = 0;
        while (buff[0]!='$'){
            bytesToRead = 10*bytesToRead + buff[0] - '0';
            if ((n = read(worker->getReadfd(), buff, 1)) < 0) 
                perror("server: data read error \n");
        }
        /* make place to hold result */
        char* localBuffer = (char*)malloc((bytesToRead+1)*sizeof(char));
        int ii = 0;
        /* read the argument */
        if ((n = read(worker->getReadfd(), localBuffer, bytesToRead)) < 0)
            perror("client: data read error \n");
        localBuffer[bytesToRead] = '\0';
        /* check if end is reached */
        if(bytesToRead > 5)
            if (localBuffer[0]=='$' && localBuffer[1]=='s' && localBuffer[2]=='t' && localBuffer[3]=='o' && localBuffer[4]=='p' && localBuffer[5]=='$')
                break;

        sum += atoi(localBuffer);
    }
    protocolWrite(sock,sum);
    //std::cout << sum << '\n';
    
}

void diseaseFrequency(char* virusName,char* date1,char* date2,int sock){
    /* sending command */
    for (WorkerNode* i = workerList.getHead(); i; i = i->getNext()){
        int wr = i->getWritefd();
        /* changing from std out to writefd */
        int stdoutBack = dup(1);
        close(1);
        if(dup2(wr, 1) < 0)
            perror("problem with dup2");

        std::cout << 5 ;
        std::cout << "2$df";
        std::cout << strlen(virusName) << '$' << virusName;
        std::cout << strlen(date1) << '$' << date1;
        std::cout << strlen(date2) << '$' << date2;
        std::cout << "1$-" << std::flush;
        

        /* changing back to std out*/
        dup2(stdoutBack, 1);
        
    }
    /* reading and summing each worker's response */
    int sum = 0;
    char buff[1];
    int n;
    for (WorkerNode* i = workerList.getHead(); i; i = i->getNext()){
    	while (1){
        	buff[0] = '0';
        	int bytesToRead = 0;
        	while (buff[0]!='$'){
            	bytesToRead = 10*bytesToRead + buff[0] - '0';
            	if ((n = read(i->getReadfd(), buff, 1)) < 0) 
                	perror("server: data read error \n");
        	}
        	/* make place to hold result */
        	char* localBuffer = (char*)malloc((bytesToRead+1)*sizeof(char));
        	int ii = 0;
        	/* read the argument */
        	if ((n = read(i->getReadfd(), localBuffer, bytesToRead)) < 0)
            	perror("client: data read error \n");
        	localBuffer[bytesToRead] = '\0';
        	/* check if end is reached */
        	if(bytesToRead > 5)
            	if (localBuffer[0]=='$' && localBuffer[1]=='s' && localBuffer[2]=='t' && localBuffer[3]=='o' && localBuffer[4]=='p' && localBuffer[5]=='$')
                	break;

        	sum += atoi(localBuffer);
        }
    }
    //std::cout << sum << '\n';
    protocolWrite(sock,sum);
}

//operation 3
void topkAgeRanges(char* k,char* country,char* disease,char* date1,char* date2,int sock){
    /* sending command to all workers */ 
	for (WorkerNode* i = workerList.getHead(); i; i = i->getNext()){
        int wr = i->getWritefd();

        if (write(wr, "6", 1) != 1) perror("server: data write error \n");
        protocolWrite(wr,(char*)"tar");
        protocolWrite(wr,k);
        protocolWrite(wr,country);
        protocolWrite(wr,disease);
        protocolWrite(wr,date1);
        protocolWrite(wr,date2);
        
    }

    setPfds(POLLIN);
    int pollR = readOneResponse(sock);
    if (!pollR)
        std::cout << "Country not found\n";
}

//operation 4
void searchPatientRecord(char* recordID,int sock){
    /* sending command */
    for (WorkerNode* i = workerList.getHead(); i; i = i->getNext()){
        int wr = i->getWritefd();
        /* changing from std out to writefd */
        int stdoutBack = dup(1);
        close(1);
        if(dup2(wr, 1) < 0)
            perror("problem with dup2");

        std::cout << 2 ;
        std::cout << "3$spr";
        std::cout << strlen(recordID) << '$' << recordID << std::flush;
        
        /* changing back to std out*/
        dup2(stdoutBack, 1);
    }


    setPfds(POLLIN);
    int pollR = readOneResponse(sock);
    if (!pollR)
        std::cout << "No record found with given id\n";

}

int readOneResponse(int fdd){
    int ready;
    ready = poll(pfds, nfds, 3000);
    if (ready == -1)
        perror("poll() failed");

    if (ready){
        for (int j = 0; j < nfds; j++) {
            char buf[100];
            if (pfds[j].revents & POLLIN) {
                printWorkerResponse(fdd,getWorkerWithReadfd(pfds[j].fd));
                pfds[j].revents = 0;
            }
        }
    }
    return ready;
}

void printWorkerResponse(int fd,WorkerNode* i){
	char buff[1];
    int n;
    /* reading and printing worker's response */
    while (1){
        buff[0] = '0';
        int bytesToRead = 0;
        while (buff[0]!='$'){
            bytesToRead = 10*bytesToRead + buff[0] - '0';
            if ((n = read(i->getReadfd(), buff, 1)) < 0) 
                perror("server: data read error \n");
        }
        /* make place to hold result */
        char* localBuffer = (char*)malloc((bytesToRead+1)*sizeof(char));
        /* read the argument */
        if ((n = read(i->getReadfd(), localBuffer, bytesToRead)) < 0)
            perror("client: data read error \n");
        localBuffer[bytesToRead] = '\0';
        /* check if end is reached */
        if(bytesToRead > 5)
            if (localBuffer[0]=='$' && localBuffer[1]=='s' && localBuffer[2]=='t' && localBuffer[3]=='o' && localBuffer[4]=='p' && localBuffer[5]=='$')
                break;
        //if (write(1, localBuffer, bytesToRead) != bytesToRead)    //fd 1 = stdout
        //    perror("server: data write error \n"); 
        protocolWrite(fd,localBuffer);
    }
}

//operations 5 & 6
void operations56(char* cmd,char* virusName,char* date1,char* date2,char* country,int sock){
    /* sending command */
    for (WorkerNode* i = workerList.getHead(); i; i = i->getNext()){
        int wr = i->getWritefd();
        /* changing from std out to writefd */
        int stdoutBack = dup(1);
        close(1);
        if(dup2(wr, 1) < 0)
            perror("problem with dup2");

        std::cout << 5 ;
        std::cout << cmd;
        std::cout << strlen(virusName) << '$' << virusName;
        std::cout << strlen(date1) << '$' << date1;
        std::cout << strlen(date2) << '$' << date2;
        std::cout << strlen(country) << '$' << country << std::flush;
        

        /* changing back to std out*/
        dup2(stdoutBack, 1);
        
    }

	setPfds(POLLIN);
	int i = 0;
    while(readOneResponse(sock));

}

WorkerNode* getWorkerForCountry(char* country){
    for (WorkerNode* i = workerList.getHead(); i; i = i->getNext())
        if (i->hasCountry(country))
            return i;
    return NULL;
}

WorkerNode* getWorkerWithReadfd(int rfd){
    for (WorkerNode* i = workerList.getHead(); i; i = i->getNext())
        if (i->getReadfd()==rfd)
            return i;
    return NULL;
}

void setPfds(int poll){
    int j = 0;
    for (WorkerNode* wr = workerList.getHead(); wr; wr = wr->getNext()) {
        pfds[j].events = poll;
        pfds[j].revents = 0;
        j++;
    }
}

// /diseaseFrequency Malaria 01-01-1990 01-01-2017

// /numPatientAdmissions Malaria 01-01-1980 01-01-2017

// /numPatientDischarges Malaria 01-01-1990 01-01-2017

// g++ -o worker stringNode.cpp reportNode.cpp recordNode.cpp worker.cpp

// g++ -o main main.cpp inputParser.cpp diseaseAggregator.cpp stringNode.cpp workerNode.cpp

// ./main -w 2 -b 50 -i hh

// /topk-AgeRanges 3 USA Malaria 01-01-0011 01-01-2018
