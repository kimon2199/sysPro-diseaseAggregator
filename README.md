# sysPro-diseaseAggregator

[Systems Programming](https://cgi.di.uoa.gr/~mema/courses/k24/k24-spring-2020.html) @ [University of Athens](https://www.di.uoa.gr/en) Spring 2020

## 1 Description

This project’s aim was to become familiar with thread programming and inter-process communication on a network in C++. We were asked to create distributed system of processes that make up an application called diseaseAggregater that answers queries related to medical data. 

diseaseAggregater consists of the three following programs:

- `whoServer`: A multi-threaded program that collects summary statistics from the workers and forwarding questions asked by `whoClient` through the network
- `master`: Creates and manages a number of worker processes
- `whoClient`: A multi-threaded client program that distributes a list of queries to it’s threads, which in turn connect to `whoServer` in order to get the answer 

## 2 Compiling and Running app

To compile the code use the command:

    make

Note: The code is meant to be compiled on Linux. Compilation in Mac OS yields an error due to a difference in the implementation of the POSIX standard libraries imported.

To remove the object files (*.o) use the command:

    make clean

In order for the deseaseAggregator to work the executables should be executed in the following order:

1. `whoServer`
2. `master`
3. `whoClient`

Instructions on how to run each one can be found right bellow

### Executing whoServer

Execute `whoServer` using flags `-q`,`-s`,`-w` & `-b` as follows

    ./whoServer –q queryPortNum -s statisticsPortNum –w numThreads –b bufferSize

- `queryPortNum`: the port number in which whoServer is listening for connection with whoClient, expecting queries 
- `statisticsPortNum`: the port number in which whoServer is listening for summary statistics from Worker processes (created by `master`)
- `numThreads`: the number of threads created for serving incoming network connections. All threads are created upon `whoServer`’s startup
- `bufferSize`: the size of cyclic buffer shared between all threads created by `whoServer`. Essentially `bufferSize` measures the number of file/socket descriptors that can be saved in the buffer (e.x. 10, means 10 descriptors)

### Executing master

Execute `master` using flags `-w`,`-b`,`-s`,`-p` & `-i` as follows

    ./master –w numWorkers -b bufferSize –s serverIP –p serverPort -i input_dir

- `numWorkers`: the number of Worker processes that will be created by `master`
- `bufferSize`: the size of the buffer for reading over pipes, measured in bytes
- `serverIP`: whoServer’s IP address (workers connect to this IP in order to send summary statistics)
- `serverPort`: port number where whoServer is listening for statistics
- `input_dir`: a directory that contains subdirectories that contain the files that the Worker processes will be processing. Each subdirectory is named after a country and contains files that have names in the following format DD-MM-YYYY. Each DD-MM-YYYY file contains a list of entries (each line is a single entry) in the following format `id ENTER/EXIT Name Surname Desease Age` e.x. `74 ENTER Emma Jones Ebola 31`

### Executing whoClient

Execute `whoClient` using flags `-q`,`-s`,`-w` & `-b` as follows

    ./whoClient –q queryFile -w numThreads –sp servPort –sip servIP

- `queryFile`: a file that contains the queries that will be sent to whoServer (for more info read [Types of Queries](#4-Types-of-Queries))
- `numThreads`: the number of threads that will be created by `whoClient` in order to send the queries to `whoServer`
- `servPort`: the port number in which `whoServer` is listening for queries by `whoClient`
- `servIP`: `whoServer`’s IP address

## 3 Description of Executables

### How whoServer works

Upon execution `whoServer` creates a number (`numThreads`) of threads. The main process thread is listening at ports `queryPortNum` and `statisticsPortNum` and accepts connections through the `accept()` system call and then stores the file/socket descriptors that correspond to each connection in cyclic buffer of the given size (`bufferSize`). The created threads serve the connections in the cyclical buffer.

The main process continues listening at port `statisticsPortNum`, for connections from Worker processes in order to receive summary statistics as well as the port at which each Worker is listening, and `queryPortNum` for connections from `whoClient` to receive queries.

When `whoServer` receives a query it is forwarded to the designated Worker process through a socket and expects a reply. The query and the reply are then printed in `stdout` (`whoServer`’s `stdout`) and the reply is also sent to the `whoClient` thread that sent the question.

### How master works

Upon execution `master` will create a number (`numWorkers`) of of Worker child processes and distributes the subdirectories inside `input_dir` to the Workers. After their creation the workers are informed about which subdirectories they are responsible for and what the IP address and the port number of `whoServer` are through a named pipe. After the Worker processes have been created and the necessary data has been transmitted to them, `master` does not exit, it remains in the case where a worker exits because of unexpected circumstance. In that case `master` has to fork a new Worker process.

Each Worker process reads the files inside it’s assigned subdirectories and stores the data inside a linked list. Then, the Worker establishes a connection to `whoServer` and forwards the port number at which he will be listening for queries coming from `whoServer` (at some point summary statistics are sent to `whoServer` as well). After that the Worker process remains listening for incoming queries in the port mentioned. (Signal handling)

### How whoClient works

`whoClient` works in the following manner. First it opens and reads the input `queryFile` line by line. Each line contains one query. For each line/query a thread is created that is responsible for sending this query to `whoServer`. A thread does not immediately connect to `whoServer` after it’s creation, it remains idle until all threads have been created and then at once all threads try to connect together (in order to demonstrate how `whoServer` can handle multiple connections happening at once). In the case where given queries are more than the given number of threads the queries are handled in batches of threads, with each batch having at most `-w numThreads` threads. Once a thread receives an answer from `whoServer` the output is printed to `stdout` and the thread can exit. Once all threads have exited, `whoClient` can exit as well.

## 4 Types of Queries

**Note 1:** Arguments date1 and date 2 should be given in the format DD-MM-YYYY. Also date1 < date2 chronologically

**Note 2:** The country argument is optional for queries diseaseFrequency, numPatientAdmissions and numPatientDischarges (this is denoted below by the square brackets)

- `/diseaseFrequency disease date1 date2 [country]`

  `whoServer` computes the total number of cases of the given `disease` for the timeframe between `date1` and `date2` in the specified `country` (if `country` is not given, the total for all countries is computed)

- `/topk-AgeRanges k country disease date1 date2`

  `whoServer` finds the top `k` age ranges (0-20, 21-40, 41-60, 60+) that have had the given `disease` in the given `country` for the timeframe between `date1` and `date2`

- `/searchPatientRecord recordID`

  `recordID` refers to the ID of a record (the first argument of a record). `whoServer` pushes this ID to all worker processes. If one of the workers finds a record with such an ID then the country is returned, if not `No record found with given id` is printed.

- `/numPatientAdmissions disease date1 date2 [country]`

  `whoServer` computes the total number of patients that have **entered** the hospital with the given `disease` for the timeframe between `date1` and `date2` in the specified `country` (if `country` is not given, the total for all countries is computed)

- `/numPatientDischarges disease date1 date2 [country]`

  `whoServer` computes the total number of patients that have **exited** the hospital with the given `disease` for the timeframe between `date1` and `date2` in the specified `country` (if `country` is not given, the total for all countries is computed)
