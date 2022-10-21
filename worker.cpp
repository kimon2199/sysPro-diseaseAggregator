#include <stdio.h>
#include <dirent.h>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <cmath>
#include "linkedList.h"
#include "stringNode.h"
#include "reportNode.h"
#include "recordNode.h"
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


bool performCheck(LinkedList<RecordNode>*,char**,char*,int);
bool isLets(char*);
bool isNums(char*);
bool isDisease(char*);
int countD(int);
bool between(char*,char*,char*);
bool less(char*,char*);
void protocolWrite(int,char*);
void protocolWrite(int,int);
void perror_exit(char*); 

void diseaseFrequency(char*,char*,char*,char*,int,LinkedList<StringNode>*,LinkedList<RecordNode>**);
void topkAgeRanges(char*,char*,char*,char*,char*,int,LinkedList<StringNode>*,LinkedList<RecordNode>**);
void searchPatientRecord(char*,int,int,LinkedList<RecordNode>**);
void numPatient(char,char*,char*,char*,char*,int,LinkedList<StringNode>*,LinkedList<RecordNode>**);

void handle_int_quit(int);
void handle_usr1(int);
bool thisIsTheEnd;
int readfd;

int total, success, fail;

int main(int argc, char* argv[]){
	thisIsTheEnd = false;
	signal(SIGINT,handle_int_quit);
	signal(SIGQUIT,handle_int_quit);
	signal(SIGUSR1,handle_usr1);
	int writefd = atoi(argv[0]);
	readfd = atoi(argv[1]);
	total = success = fail = 0;
	int returnStats = atoi(argv[3]);
	int buffSize = atoi(argv[4]);
	char* serverIP = argv[5];
	int serverPort = atoi(argv[6]);

	/* make list of countries */
	LinkedList<StringNode> countryList;
	if (!thisIsTheEnd){
		for (int i = 7; argv[i]!=NULL; i++)
			countryList.addNode(new StringNode(argv[i]));
	}

	/* array of recordLists */
	LinkedList<RecordNode>** recordLists;
	recordLists = (LinkedList<RecordNode>**)malloc(countryList.getLength()*sizeof(LinkedList<RecordNode>*));

	/*---------------*/
	/* .. Sockets .. */
	/*---------------*/


	/* declarations */
	struct sockaddr_in server, serverQ, clientQ;
    struct sockaddr *serverptr = (struct sockaddr*)&server;
    struct sockaddr *serverQptr =(struct sockaddr  *)&serverQ;
	struct sockaddr *clientQptr =(struct sockaddr  *)&clientQ;
	socklen_t  clientQlen,serverQlen; 
    int sockQ, sockSt;

   	/* making the socket to receive queries (this will be used later in the code) */
	if((sockQ = socket(AF_INET , SOCK_STREAM , 0)) ==  -1)
		perror_exit((char*)"Socket  creation  failed!");
	serverQ.sin_family = AF_INET; 
	serverQ.sin_addr.s_addr = htonl(INADDR_ANY); 
	serverQ.sin_port = 0; 
	/* binding sockSt to address */
	if (bind(sockQ, serverQptr, sizeof(serverQ)) < 0)
		perror_exit((char*)"bind");
	serverQlen = sizeof(serverQ);
	if (getsockname(sockQ, (struct sockaddr *)&serverQ, &serverQlen) == -1)
    	perror("getsockname");
	std::cout << "The assigned port for questions was " << ntohs(serverQ.sin_port) << std::endl;

	/* making the socket to connect and send statisticts */
	if((sockSt = socket(AF_INET , SOCK_STREAM , 0)) ==  -1)
		perror_exit((char*)"Socket  creation  failed!");
	/* Initiate connection */
	server.sin_family = AF_INET;       /* Internet domain */
    server.sin_addr.s_addr = inet_addr(serverIP); /* Server IP */
    server.sin_port = htons(serverPort);          /* Server port */
    if (connect(sockSt, serverptr, sizeof(server)) < 0)      //     < --- se afto to kommati exv meinei kalytera as kanw prwta ton server
	   	perror_exit((char*)"connect");
	printf("Connecting to %s port %d\n", serverIP, serverPort);

	/* write socket that will listen for questions */ 
	protocolWrite(sockSt,ntohs(serverQ.sin_port));
	/* write countries it will have */
	for (StringNode* i = countryList.getHead(); i; i = i->getNext())
		protocolWrite(sockSt,i->getString());
	/* indicator that end is reached for reader */
	if (write(sockSt, "6$$stop$", 8) != 8) 
        perror("client: data write error \n");

	int j = 0;
	for (StringNode* curCountryNode = countryList.getHead(); curCountryNode; curCountryNode = curCountryNode->getNext()){
		if (thisIsTheEnd)
			break;
		char* curCountry = curCountryNode->getString();


    	/* make path to country */
    	char* dirPath = (char*)malloc((strlen(argv[2])+ 1 +strlen(curCountry)+1)*sizeof(char));
    	strcpy(dirPath,argv[2]);
    	strcat(dirPath,"/");
    	strcat(dirPath,curCountry);


		DIR *countryDirectory;
		struct dirent *ent;
		LinkedList<StringNode> fileList;
		if ((countryDirectory = opendir(dirPath)) != NULL) {
  			/* print all the files and directories within directory */
  			while ((ent = readdir(countryDirectory)) != NULL) {
  				if((ent->d_name)[0] != '.')
  					fileList.addNode(new StringNode(ent->d_name));
  			}
  			closedir(countryDirectory);
		} 
		else {
  			/* could not open directory */
  			std::cout << "zeza: " << dirPath << '\n';
  			perror("");
  			return EXIT_FAILURE;
		}

		fileList.sortList();
		// initializing list that will contain all records
		recordLists[j] = new LinkedList<RecordNode>;


		// about to read each individual file
		StringNode* cur = fileList.getHead();
		while (cur != NULL){
			if (thisIsTheEnd)
				break;
			LinkedList<ReportNode> reportList; // <--------------------------
			char* date = cur->getString();
			char* path = (char*)malloc((strlen(dirPath)+1+strlen(date)+1)*sizeof(char));
			strcpy(path,dirPath);
			strcat(path,"/");
			strcat(path,date);
			std::ifstream dateFile(path);
			if (!dateFile){
    			std::cerr << "error: Unable to open file \"" << path << "\"\n";
    			::exit(1);
			}
  			std::string aLine;
  			while (std::getline(dateFile, aLine)){
  				int argc = 0;
				char* argv[6];
				// initializing new stream for each record
  				std::istringstream streamm(aLine);
    			std::string arg; 
    			do {
    				// extracting word by word
        			streamm >> arg;
      				if(streamm){
        				argv[argc] = (char*)malloc((arg.length()+1)*sizeof(char));
        				strcpy(argv[argc],arg.c_str());
        				argc++;
        			}
    			} while (streamm && argc < 6);
    			streamm >> arg;
    			if(streamm){
    		    	std::cerr << "error: Record with too many words (skipping record) " << "\n";
    			}
    			if(performCheck(recordLists[j],argv,date,returnStats)){ // if all is good with record
  		    		if (!reportList.incr(argv[4],argv[5])) // if there is no node for this disease
  		    			reportList.addNode(new ReportNode(argv[4],argv[5]));
  		    		recordLists[j]->addNode(new RecordNode(argv[0],argv[2],argv[3],argv[4],argv[5],date));
  		    	}
  			}
  			if (!thisIsTheEnd){

  				if (returnStats){
	  				/* changing from std out to sockSt */
	  				int stdoutBack = dup(1);
	  				close(1);
	  				dup2(sockSt, 1);


	  				std::cout << strlen(date)+1 << '$' << date << '\n';
	  				std::cout << strlen(curCountry)+1 << '$' << curCountry << '\n';
	  				reportList.printList(0); // <--------------------------
	        		std::cout << 1 << '$' << '\n';

	  				/* changing back to std out*/
	  				dup2(stdoutBack, 1);
	  				//close(sockSt);
	  			}
				cur = cur->getNext();
  			}
		}
		if (!thisIsTheEnd){
			j++;
		}
	}

	/* indicator that end is reached for reader */
	if (returnStats)
		if (write(sockSt, "6$$stop$", 8) != 8) 
        	perror("client: data write error \n");
	
	/*--------------------------*/
	/* Waiting for commands part */
    /*--------------------------*/

	/*  Listen  for  connections  */
	if(listen(sockQ , 5) < 0)  
		perror_exit((char*)"listen");
	printf("Listening  for  connections  to port %d (for questions)\n", ntohs(serverQ.sin_port));

	int n,new_socket;
	if ((new_socket = accept(sockQ, clientQptr, &clientQlen))<0) 
		perror_exit((char*)"accept");
	printf("Accepted  connection (for questions)\n");

	char* buff = (char*)malloc(buffSize*sizeof(char));
	if (!returnStats)
		std::cout <<"Worker replaced\n";
	while (1){
		/* read number of arguments first */
		int myArgc;
		if ((n = read(new_socket, buff, 1)) < 0)
	        myArgc = 0;
		else
	    	myArgc = buff[0] - '0';
	    /* is sigint or sigquit has been received */
		if (thisIsTheEnd)
			break;
	    char* myArgv[6];
	    /* read command and args from pipe */
		for (int i = 0; i <myArgc;i++){
	        buff[0] = '0';
	        int bytesToRead = 0;
	        while (buff[0]!='$'){
	            bytesToRead = 10*bytesToRead + buff[0] - '0';
	            if ((n = read(new_socket, buff, 1)) < 0) 
	                perror("client: data read error \n");
	        }
	        /* make place to hold result */
	        char* localBuffer = (char*)malloc((bytesToRead+1)*sizeof(char));
	        int ii = 0;
	        /* read the argument */
	        while (bytesToRead){
	        	int bytesIWillRead = (buffSize < bytesToRead) ? buffSize : bytesToRead;
	        	if ((n = read(new_socket, buff, bytesIWillRead)) < 0)
	            	perror("client: data read error \n");
	            for (int j = 0; j < bytesIWillRead ; j++)
	            	localBuffer[ii+j] = buff[j];
	            bytesToRead -= bytesIWillRead;
	            ii += bytesIWillRead;
	        }
	        localBuffer[ii] = '\0';
	        /* copy argument to array */
	        myArgv[i] = (char*)malloc((strlen(localBuffer)+1)*sizeof(char));
	        for (int j = 0; j<strlen(localBuffer); j++)
	        	myArgv[i][j] = localBuffer[j];
	        myArgv[i][strlen(localBuffer)] = '\0';
    	}
    	std::cout << "Question received: ";
    	for(int i=0; i<myArgc; i++)
        	std::cout << myArgv[i] << " ";
        std::cout << std::endl;

    	if (!strcmp("df",myArgv[0]))            //op1
        	diseaseFrequency(myArgv[1],myArgv[2],myArgv[3],myArgv[4],new_socket,&countryList,recordLists);
        else if (!strcmp("tar",myArgv[0]))      //op2
			topkAgeRanges(myArgv[1],myArgv[2],myArgv[3],myArgv[4],myArgv[5],new_socket,&countryList,recordLists);
        else if (!strcmp("spr",myArgv[0]))      //op3
            searchPatientRecord(myArgv[1],new_socket,countryList.getLength(),recordLists);
        else if (!strcmp("npa",myArgv[0]))      //op4
            numPatient('a',myArgv[1],myArgv[2],myArgv[3],myArgv[4],new_socket,&countryList,recordLists);
        else if (!strcmp("npd",myArgv[0]))      //op5
            numPatient('d',myArgv[1],myArgv[2],myArgv[3],myArgv[4],new_socket,&countryList,recordLists); 
        else
        	fail++;

        total++;
    	/* free memory */
    	for(int i=0; i<myArgc; i++)
        	free(myArgv[i]);
	}


    std::cout << "i'm out too\n";

    /* make name for log file */
    std::ostringstream pid;
    pid << getpid();
    char logName[64];
    strcpy(logName,"./log_file.");
    strcat(logName,pid.str().c_str());
    /* create log file */
    std::ofstream aFile(logName);
    /* write each country in log file */
    for (StringNode* c = countryList.getHead(); c; c = c->getNext())
        aFile << c->getString() << std::endl;
    /* write request's info in log file */
    aFile << "TOTAL " << total << std::endl;
    aFile << "SUCCESS " << success << std::endl;
    aFile << "FAIL " << fail << std::endl;
    /* close log file */
    aFile.close();

	return 0;
}

int bind_on_port(int sock ,short port) {
	struct sockaddr_in  server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(port);
	return bind(sock , (struct sockaddr  *) &server ,sizeof(server));
}

void perror_exit(char*message) {
	perror(message);
	exit(EXIT_FAILURE);
}

//operations ----------------------------------------------


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

void protocolWrite(int wfd,int nmsg){
	/* turn int msg to string in order to send */
    std::ostringstream sl;
    sl << nmsg;
    protocolWrite(wfd,(char*)sl.str().c_str());
}

void diseaseFrequency(char* virusName,char* date1,char* date2,char* country,
	int writefd,LinkedList<StringNode>* countries,LinkedList<RecordNode>** recordLists){

	int sum = 0;
	int i = 0;
	for (StringNode* c = countries->getHead(); c; c = c->getNext()){
		char* curCountry = c->getString();
		if (!strcmp(country,curCountry) || !strcmp(country,"-")){
			for (RecordNode* rec = recordLists[i]->getHead(); rec; rec = rec->getNext()){
				char* cVir = rec->getVir();
				char* cDat = rec->getInDate();
				if (!strcmp(virusName,cVir) && between(cDat,date1,date2))
					sum++;
			}
		}
		i++;
	}

  	protocolWrite(writefd,sum);
  	protocolWrite(writefd,(char*)"$stop$");
  	success++;
}

void topkAgeRanges(char* k,char* country,char* disease,char* date1,char* date2,
	int wrfd,LinkedList<StringNode>* countries,LinkedList<RecordNode>** recordLists){
	bool isSuccess = false;
	if (atoi(k) < 0){
		fail++;
		return;
	}
	int i = 0;
	for (StringNode* c = countries->getHead(); c; c = c->getNext()){
		char* curCountry = c->getString();
		if (!strcmp(country,curCountry)){
			ReportNode report;
			for (RecordNode* rec = recordLists[i]->getHead(); rec; rec = rec->getNext()){
				char* cVir = rec->getVir();
				char* cDat = rec->getInDate();
				if (!strcmp(disease,cVir) && between(cDat,date1,date2)){
					report.incr(rec->getAge());
				}
			}
			int r = report.printTopK(atoi(k),wrfd);
			if (r){
				protocolWrite(wrfd,(char*)"No record with ");
				protocolWrite(wrfd,disease);
				protocolWrite(wrfd,(char*)" in ");
				protocolWrite(wrfd,country);
				protocolWrite(wrfd,(char*)" for ");
				protocolWrite(wrfd,date1);
				protocolWrite(wrfd,(char*)" to ");
				protocolWrite(wrfd,date2);
				protocolWrite(wrfd,(char*)"\n");
			}
			if (!isSuccess){
				success++;
				isSuccess = true;
			}
			protocolWrite(wrfd,(char*)"$stop$");
			break;
		}
		i++;
	}
}

void searchPatientRecord(char* id,int writefd,int numCountries,LinkedList<RecordNode>** recordLists){
	for(int i=0; i < numCountries; i++){
		for (RecordNode* rec = recordLists[i]->getHead(); rec; rec = rec->getNext()){
				char* cId = rec->getId();
				if (!strcmp(id,cId)){    
					/* changing from std out to writefd */
  					int stdoutBack = dup(1);
  					close(1);
  					dup2(writefd, 1);

  					std::cout << rec->getNumOfCharsInPrint() << '$';
  					rec->print();
  					std::cout << "6$$stop$" << std::flush;

					/* changing back to std out*/
  					dup2(stdoutBack, 1);
  					success++;
  					return;
				}
			}
	}
	fail++;
}

void numPatient(char z,char* virusName,char* date1,char* date2,char* country,
	int writefd,LinkedList<StringNode>* countries,LinkedList<RecordNode>** recordLists){

	int i = 0;
	for (StringNode* c = countries->getHead(); c; c = c->getNext()){
		int sum = 0;
		char* curCountry = c->getString();
		if (!strcmp(country,curCountry) || !strcmp(country,"-")){
			for (RecordNode* rec = recordLists[i]->getHead(); rec; rec = rec->getNext()){
				char* cVir = rec->getVir();
				if (z=='a'){     /* Admissions */
					char* cDat = rec->getInDate();
					if (!strcmp(virusName,cVir) && between(cDat,date1,date2)){
						sum++;
					}
				}
				else if (z=='d'){ /* Discharges */
					char* cDat = rec->getOutDate();
					if (strcmp(cDat,(char*)"--"))
						if (!strcmp(virusName,cVir) && between(cDat,date1,date2))
							sum++;
				}
			}
	  		protocolWrite(writefd,curCountry);
	  		protocolWrite(writefd,(char*)" ");
	  		protocolWrite(writefd,sum);
	  		protocolWrite(writefd,(char*)"\n");
	    }

		i++;
	}

	protocolWrite(writefd,(char*)"$stop$");
	success++;
}



void handle_int_quit(int sig){
	thisIsTheEnd = true;
	close(readfd);
}

void handle_usr1(int sig){
    std::cout << "15\n";
}








bool performCheck(LinkedList<RecordNode>* list,char** a,char* date,int returnStats){
	if (!isLets(a[2]) || !isLets(a[3]) || !isDisease(a[4]) || !isNums(a[5])){
		if (returnStats)
			std::cout << "ERROR" << std::endl;
		return false;
	}
	if (atoi(a[5]) > 120){
		if (returnStats)
			std::cout << "ERROR" << std::endl;
		return false;
	}
	if (!strcmp(a[1],"ENTER")){
		if (list->getNodeWithKey(a[0]) != NULL){
			if (returnStats)
				std::cout << "ERROR" << std::endl;
			return false;
		}
	}
	else if (!strcmp(a[1],"EXIT")){
		RecordNode* node = list->getNodeWithKey(a[0]);
		// if there is no one with this ID
		if (node == NULL){
			if (returnStats)
				std::cout << "ERROR" << std::endl;
			return false;
		}
		// if name,age or disease dont match
		if (!node->same(a[2],a[3],a[4],a[5])){
			if (returnStats)
				std::cout << "ERROR" << std::endl;
			return false;
		}
		// if exit date is before enter date
		if (node->less(date,node->getInDate())){
			if (returnStats)
				std::cout << "ERROR" << std::endl;
			return false;
		}
		node->setOutDate(date);
		return false; // since it is an EXIT
	}
	else {
		if (returnStats)
			std::cout << "ERROR" << std::endl;
		return false;
	}
	return true;
}

bool isLets(char* a){
	for (int i=0; i<strlen(a);i++)
		if (!(a[i]>='A' && a[i]<='Z') && !(a[i]>='a' && a[i]<='z'))
			return false;
	return true;
}
bool isNums(char* a){
	for (int i=0; i<strlen(a);i++)
		if (!(a[i]>='0' && a[i]<='9'))
			return false;
	return true;
}
bool isDisease(char* a){
	bool flag = 0;
	for (int i=0; i<strlen(a);i++){
		if (!flag && a[i]=='-'){
			flag = 1;
			continue;
		}
		if (flag && a[i]=='-')
			return false;
		if (!(a[i]>='A' && a[i]<='Z') && !(a[i]>='a' && a[i]<='z') && !(a[i]>='0' && a[i]<='9'))
			return false;
	}
	return true;
}

int countD(int n) { 
    return n ? std::floor(log10(n) + 1) : 1; 
} 

bool between(char* my,char* l,char* r){
	return !less(my,l) && !less(r,my);
}

bool less(char* l,char* r){
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

// /diseaseFrequency Malaria 01-01-1990 01-01-2017

// /numPatientAdmissions Malaria 01-01-1980 01-01-2017

// /numPatientDischarges Malaria 01-01-1990 01-01-2017

// g++ -o worker stringNode.cpp reportNode.cpp recordNode.cpp worker.cpp

// g++ -o main main.cpp inputParser.cpp diseaseAggregator.cpp stringNode.cpp workerNode.cpp

// ./main -w 2 -b 50 -i hh

// /topk-AgeRanges 3 USA Malaria 01-01-0011 01-01-2018
