#
# In order to execute this "Makefile" just type "make"
#

OBJS4 	= whoClient.o inputParser.o stringNode.o
SOURCE4	= whoClient.cpp inputParser.cpp stringNode.cpp
OUT4  	= whoClient

OBJS3 	= whoServer.o inputParser.o circleBuffer.o workerNode.o stringNode.o
SOURCE3	= whoServer.cpp inputParser.cpp circleBuffer.cpp workerNode.cpp stringNode.cpp
OUT3  	= whoServer

OBJS2 	= stringNode.o reportNode.o recordNode.o worker.o
SOURCE2	= stringNode.cpp reportNode.cpp recordNode.cpp worker.cpp
OUT2  	= worker

OBJS1 	= inputParser.o master.o stringNode.o reportNode.o recordNode.o workerNode.o
SOURCE1	= inputParser.cpp master.cpp stringNode.cpp reportNode.cpp recordNode.cpp workerNode.cpp
OUT1  	= master

CC	= g++
FLAGS   = -c
# -g option enables debugging mode 
# -c flag generates object code for separate files

all: $(OUT1) $(OUT2) $(OUT3) $(OUT4)

$(OUT4): $(OBJS4)
	$(CC) $(OBJS4) -o $@ -lpthread

$(OUT3): $(OBJS3)
	$(CC) $(OBJS3) -o $@ -lpthread

$(OUT2): $(OBJS2)
	$(CC) $(OBJS2) -o $@

$(OUT1): $(OBJS1)
	$(CC) $(OBJS1) -o $@



# create/compile the individual files separately
#main.o: main.cpp
#	$(CC) $(FLAGS) main.cpp

inputParser.o: inputParser.cpp
	$(CC) $(FLAGS) inputParser.cpp

reportNode.o: reportNode.cpp
	$(CC) $(FLAGS) reportNode.cpp 

recordNode.o: recordNode.cpp
	$(CC) $(FLAGS) recordNode.cpp

stringNode.o: stringNode.cpp
	$(CC) $(FLAGS) stringNode.cpp

workerNode.o: workerNode.cpp
	$(CC) $(FLAGS) workerNode.cpp

worker.o: worker.cpp
	$(CC) $(FLAGS) worker.cpp

master.o: master.cpp
	$(CC) $(FLAGS) master.cpp

whoServer.o: whoServer.cpp
	$(CC) $(FLAGS) whoServer.cpp

whoClient.o: whoClient.cpp
	$(CC) $(FLAGS) whoClient.cpp

circleBuffer.o: circleBuffer.cpp
	$(CC) $(FLAGS) circleBuffer.cpp

# clean house
clean:
	rm -f $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4)
