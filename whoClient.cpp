#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include "iParser.h"
#include "linkedList.h"
#include "stringNode.h"
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

void perror_exit(char*);
void* print_self(void*);
void protocolWrite(int,char*);
char* protocolRead(int);

pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;
int servPort;
char* servIP;


int main(int argc, char* argv[]){
	int numThreads;
	char* queryFile;

	//Parsing the comandline input
	if(inputParserC(&queryFile,&numThreads,&servPort,&servIP,argc,argv)){
		std::cerr << "Error in input\n";
		return 1;
	}

	/* putting questions in a list */
	LinkedList<StringNode> questionList;
	std::ifstream file(queryFile);
	std::string question; 
	while (std::getline(file, question))
		questionList.addNode(new StringNode((char*)question.c_str()));

	/* test: see ques */
	for(StringNode* i = questionList.getHead(); i; i = i->getNext())
		std::cout << i->getString() << '\n';
	std::cout << '\n';

	int questionsLeft = questionList.getLength();
	StringNode* curQuestion = questionList.getHead();

	while(questionsLeft){
		int queRound = (numThreads > questionsLeft) ? questionsLeft : numThreads;
		pthread_t* threads = (pthread_t*)malloc(queRound*sizeof(pthread_t));
		/* initializing barrier */
		pthread_barrier_init (&barrier, NULL, queRound);
		/* Creating the threads */
		for(int i = 0; i<queRound; i++){
			pthread_create(&threads[i], NULL, print_self, (void*)curQuestion->getString());
			curQuestion = curQuestion->getNext();
		}
		/* Joining the threads */
		for(int i = 0; i<queRound; i++)
			pthread_join(threads[i], NULL);

		questionsLeft -= queRound;
	}
	return 0;
}

void perror_exit(char*message) {
	perror(message);
	exit(EXIT_FAILURE);
}

void* print_self(void *m){
	char* question = (char*)m;
	/* declarations */
	struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*)&server;
   	struct hostent *rem;
    int sock;

	/* making the socket to connect and send statisticts */
	if((sock = socket(AF_INET , SOCK_STREAM , 0)) ==  -1)
		perror_exit((char*)"Socket  creation  failed!");

	/* prepare for  connection */
	server.sin_family = AF_INET;       /* Internet domain */
    server.sin_addr.s_addr = inet_addr(servIP); /* Server IP */
    server.sin_port = htons(servPort);          /* Server port */

	/* wait for other threads */
    pthread_barrier_wait(&barrier);

	/* initiate connection */
    if (connect(sock, serverptr, sizeof(server)) < 0) 
	   	perror_exit((char*)"connect");
	//printf("Connecting to %s port %d\n", servIP, servPort);

	/* send question to server */
   	protocolWrite(sock,question);
   	/* get response from server */
   	char* response;
   	/* print Q&A */
	pthread_mutex_lock(&print_lock);
	std::cout << question << '\n';
	while((response = protocolRead(sock)))
		std::cout << response;
	std::cout << '\n';
	pthread_mutex_unlock(&print_lock);
	free(response);
	pthread_exit(0);
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


