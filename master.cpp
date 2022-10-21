#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include "iParser.h"
#include "recordNode.h"
#include "linkedList.h"
#include "stringNode.h"
#include "workerNode.h"
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <sys/wait.h>
#define PERMS 0666

void handle_int_quit(int);
void handle_chld(int);
void forkWithFifoHandling(WorkerNode*,int);
void createChild(char*,char*,WorkerNode*,bool);
WorkerNode* getWorkerWithPid(int);

bool thisIsTheEnd;
int fifoCount;
char* dirPath;
int bufferSize, serverPort;
char* serverIP;
LinkedList<WorkerNode> workerList;


int main(int argc, char *argv[]){
	char* input_dir;
	int numWorkers;

	//Parsing the comandline input
	if(inputParserM(&input_dir,&numWorkers,&bufferSize,&serverIP,
		&serverPort,argc,argv)){
        std::cerr << "Error in input\n";
		return 1;
	}
	std::cout << "input_dir: " << input_dir << std::endl;
	std::cout << "numWorkers: " << numWorkers << std::endl;
	std::cout << "bufferSize: " << bufferSize << std::endl;
	std::cout << "serverIP: " << serverIP << std::endl;
	std::cout << "serverPort: " << serverPort << std::endl;

    thisIsTheEnd = fifoCount = 0;
    signal(SIGINT,handle_int_quit);
    signal(SIGQUIT,handle_int_quit);
    signal(SIGCHLD, handle_chld);

	LinkedList<StringNode> countryList;
	int workersWorking;
	int nfds;
    struct pollfd *pfds;

    /* STEP 1: create path for input_dir */
    dirPath = (char*)malloc((strlen("./")+strlen(input_dir)+1)*sizeof(char));
    if (dirPath == NULL)
        perror("malloc() failed");
    strcpy(dirPath,"./");
    strcat(dirPath,input_dir);

    /* STEP 2: read list of countries  */
    DIR *inDirectory;
    struct dirent *ent;
    // open dir to get file names
    if ((inDirectory = opendir(dirPath)) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir(inDirectory)) != NULL) {
            if((ent->d_name)[0] != '.')
                countryList.addNode(new StringNode(ent->d_name));
        }
        closedir(inDirectory);
    } 
    else {
        /* could not open directory */
        perror("");
        ::exit(1);
    }

    /* STEP 4: create list of future children */
    for (int i=0; i< numWorkers; i++)
        workerList.addNode(new WorkerNode());

    /* STEP 5: assign countries */
    workersWorking = 0;
    int rep = false;
    WorkerNode* curWorker = workerList.getHead();
    for (StringNode* i = countryList.getHead(); i; i = i->getNext()){
        if (!rep)
            workersWorking++;
        curWorker->addCountry(i->getString());
        if ((curWorker = curWorker->getNext()) == NULL){
            curWorker = workerList.getHead();
            rep = true;
        }
    }

    /* STEP 6: create workers */
    for (WorkerNode* i = workerList.getHead(); i; i = i->getNext())
        forkWithFifoHandling(i,0);


    /* STEP 7: initializing class fields */
    nfds = workerList.getLength();
    pfds = (struct pollfd *)malloc(nfds*sizeof(struct pollfd));
    if (pfds == NULL)
        perror("malloc() failed");
    int j = 0;
    for (WorkerNode* wr = workerList.getHead(); wr; wr = wr->getNext()) {
        pfds[j].fd = wr->getReadfd();
        j++;
    }

    while(!thisIsTheEnd); /* Wait until its time to go */

    /* send sigkill to children and wait for them to terminate */
	for (WorkerNode* i = workerList.getHead(); i; i = i->getNext()){
    	kill(i->getPid(),SIGKILL);
    }
    signal(SIGCHLD, SIG_DFL);
    int pidd;
    while ((pidd = wait(NULL)) > 0);
    std::cout << "i'm out\n";

    for (int i = 0; i < fifoCount; i++){
        std::ostringstream num;
        num << i;
        char fifo1Name[64];
        strcpy(fifo1Name,"./fifo1.");
        strcat(fifo1Name,num.str().c_str());
        char fifo2Name[64];
        strcpy(fifo2Name,"./fifo2.");
        strcat(fifo2Name,num.str().c_str());
        unlink(fifo1Name);
        unlink(fifo2Name);
    }

    /*
    //afta kanei enas reader
    char* buff = (char*)malloc(bufferSize*sizeof(char));
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
            /* make place to hold result /
            char* localBuffer = (char*)malloc((bytesToRead+1)*sizeof(char));
            int ii = 0;
            /* read the argument /
            while (bytesToRead){
                int bytesIWillRead = (bufferSize < bytesToRead) ? bufferSize : bytesToRead;
                if ((n = read(i->getReadfd(), buff, bytesIWillRead)) < 0)
                    perror("client: data read error \n");
                for (int j = 0; j < bytesIWillRead ; j++)
                    localBuffer[ii+j] = buff[j];
                bytesToRead -= bytesIWillRead;
                ii += bytesIWillRead;
            }
            localBuffer[ii] = '\0';
            /* check if end is reached /
            if(ii > 5)
                if (localBuffer[0]=='$' && localBuffer[1]=='s' && localBuffer[2]=='t' && localBuffer[3]=='o' && localBuffer[4]=='p' && localBuffer[5]=='$')
                    break;
            if (write(1, localBuffer, ii) != ii)    //fd 1 = stdout
                perror("server: data write error \n"); 
        }
    }
    */
    return 0;
}

void handle_int_quit(int sig){
    thisIsTheEnd = true;
    close(0);
}

void handle_chld(int sig){ 
    signal(SIGCHLD, handle_chld);
    int cpid = wait(NULL);
    forkWithFifoHandling(NULL,cpid);
} 

void forkWithFifoHandling(WorkerNode* i,int pidOfDeadChild){
    int childpid;
    /* if i is NULL this was called by the signal handler and a child is being replaced */
    int resurection = false;
    if (!i){
        i = getWorkerWithPid(pidOfDeadChild);
        resurection = true;
    }
    /* make name for fifos */
    std::ostringstream num;
    num << fifoCount;
    char fifo1Name[64];
    strcpy(fifo1Name,"./fifo1.");
    strcat(fifo1Name,num.str().c_str());
    char fifo2Name[64];
    strcpy(fifo2Name,"./fifo2.");
    strcat(fifo2Name,num.str().c_str());

    /* make fifos  */
    if ( (mkfifo(fifo1Name, PERMS) < 0) && (errno != EEXIST) )
        perror("can't create fifo");
    if ((mkfifo(fifo2Name, PERMS) < 0) && (errno != EEXIST)) {
        unlink(fifo1Name);
        perror("can't create fifo"); 
    }

    /* do actual forks */
    if ( (childpid = fork()) < 0 ) { 
        ::exit(1);
    }
    else if (childpid == 0){   //is child
        createChild(fifo1Name,fifo2Name,i,resurection);
    }
    else {                     //is parent
        if ( (i->setReadfd(open(fifo1Name, O_RDONLY))) < 0)
            perror("server: can't open read fifo");
        if ( (i->setWritefd(open(fifo2Name, O_WRONLY))) < 0) 
            perror("server: can't open write fifo");
        i->setPid(childpid);
        i->setFifo((char*)num.str().c_str());
    }
    fifoCount++;
}

void createChild(char* fifo1Name,char* fifo2Name,WorkerNode* i,bool resu) {
    int writefd, readfd;
    /* open fifos from child's end */
    if ( (writefd = open(fifo1Name, O_WRONLY))  < 0)  
        perror("client: can't open write fifo \n");
    if ( (readfd = open(fifo2Name, O_RDONLY))  < 0)  
        perror("client: can't open read fifo \n");

    /* turn ints writefd,readfd to char* */
    std::ostringstream s1,s2;
    s1 << writefd;
    s2 << readfd;

    /* making argument list for exec */
    char **argss = (char**)malloc((8+i->countriesCount())*sizeof(char*));
    if (argss == NULL)
        perror("malloc() failed");
    argss[0] = (char*)s1.str().c_str(); // #0 -> writefd
    argss[1] = (char*)s2.str().c_str(); // #1 -> readfd
    argss[2] = dirPath; 				// #2 -> dirPath
    // #3 -> is it substituting a dead worker
    argss[3] = (char *)malloc(2*sizeof(char)); 
    if (resu)
        strcpy(argss[3],(char*)"0");
    else
        strcpy(argss[3],(char*)"1");
    // #4 -> bufferSize
    std::ostringstream sl,sp;
    sl << bufferSize;
    argss[4] = (char *)malloc((strlen((char*)sl.str().c_str())+1)*sizeof(char));
    strcpy(argss[4],(char*)sl.str().c_str());
    // #5 -> serverIP
    argss[5] = (char *)malloc((strlen(serverIP+1)*sizeof(char)));
    strcpy(argss[5],serverIP);
    // #6 -> serverPort
    sp << serverPort;
    argss[6] = (char *)malloc((strlen((char*)sp.str().c_str())+1)*sizeof(char));
    strcpy(argss[6],(char*)sp.str().c_str());
    // #fin -> NULL
    argss[8 + i->countriesCount() - 1] = NULL;
    // #7+ -> list of countries
    int ii = 7;
    for (StringNode* cur = i->getFirstCountry(); cur; cur = cur->getNext()){
        argss[ii] = cur->getString();
        ii++;
    }
    /* executing worker */
    execvp("./worker",argss);
}

WorkerNode* getWorkerWithPid(int pid){
    for (WorkerNode* i = workerList.getHead(); i; i = i->getNext())
        if (i->getPid()==pid)
            return i;
    return NULL;
}
