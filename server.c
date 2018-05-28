#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string.h>
#include<ctype.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<errno.h>
#include<strings.h>
#include<signal.h>
#include<sys/ioctl.h>
#include<pthread.h>
#include <arpa/inet.h>
#include <poll.h>
#include<time.h>

#define TRUE 1
#define START "Welcome : Enter the commands to your liking\n"
#define DIVIDE "Error : division by zero\n"
#define MESSAGE "Invalid command, type help\n"
#define NOELEMENTS "No elements in the list\n"
#define HELP "=========================\nWelcome to our program guide:\nType add number1 number2 ...\nType sub number1 number2 ...\nType mult number1 number2 ...\nType div number1 number2 ...\nType run argument1 ...\nType list : To view currenly running processes and list[ all] for all processes executed\nType kill PID, to kill an actively running prcocess\nType exit : to exit our program\nNote: All the operations are not case-sensitve, example: Add, aDD, ADd are valid\nMinimun number of inputs for arthimetic operation is Two\n=========================\n"

//##Structure defintions
struct showIpAndPortNode{
	
	int pid;
	int socket;
	int readServerChildId;
	int writeServerChildId;
	int readServerId;
	int writeServerId;
	int portNo;
	char ipAddress[INET_ADDRSTRLEN];
	
};//client information node

struct ipAndPortArrayNode{
	
	int connectionNo;
	struct showIpAndPortNode *ipAndPortArray[30];
}; //client list array 

struct ipAndPortArrayNode clientList;
//array of clients defined globally

struct acceptConnectionsNode{
	
	int identifier;
	struct sockaddr_in clientAddr;
	int lengthClient;
		
};
//structure passed into accept connection thread;


struct Node{
	int id;
	int active;
	time_t start;
	time_t end;
	char name[15];	
};
//individual process node 


struct Node * processArray[30];
//Node array defined
int processCounter = 0;
//process counter for process array

//###method definition#########
void printProcessArray(int fd);
//printing the aray

void printActiveProcessArray(int fd);
//printing the active array

void removeProcess(int processID);
//changing status to inactive

void printProcessIp(struct showIpAndPortNode * pipeInput);
//printing server port and ip on the screen

void printClientProcess(struct showIpAndPortNode * pipeInput);
//printing client processes on the screen; command given by the server

int checkDigit(char * token);
//checking if arguments for the arthimentics are valid or not

char* methodArithmetic(char * token, char * buff);
//method to handle arthimetic

void * acceptConnections(void * ptr);
//thread for accepting connection

int getIndexClient(int pid);
//this method is called by the child server method of locating the child in array with the help of pid

void removeProcessClient(int pid);
//this process removes the child from parent server list once the child has disconnected

void * serverRead(void *ptr);
//thread for parent process read

void printClients();
//server process method to get the list o

void saveEndTime(int pid, time_t end);

void * serverChildRead(void *ptr);
//thread for server Child

static void signalHandler()
{
	int status;
	int pid;
	
	do{
	pid = waitpid(-1,&status,WNOHANG);
	
	if(pid == -1)
		{
			perror("error");
			exit(1);
		}
	
	if(pid > 0)
		{	time_t timeNoted = time(NULL);
			saveEndTime(pid,timeNoted);
			removeProcess(pid); //changing the status of the process to inactive...	
		}
	}while(pid == 0);
}
//signal handler for server child to remove make its processes appear inactive
static void signalHandlerClient()
{
		int pid = wait(NULL);
		removeProcessClient(pid);
		
}
//signal handler for server parent to remove client from its client list
void printIpAndPort(int portNumber, char * ipAddress)
{
	int port = ntohs(portNumber);
	
	char tempBuff[100];
	
	int count = sprintf(tempBuff,"Port number = %d and ip = %s\n", port,ipAddress);
	
	write(STDOUT_FILENO,tempBuff,count);

}
void killProcessByName(char * token, int fd)
{
	if(strcasecmp(token,"all") == 0)
	{
		int activePCount;	
		
		for(int i = 0;i<processCounter;i++)
		{
			if(processArray[i]->active == 1)
				{
					activePCount++;
					int c = kill(processArray[i]->id,SIGTERM);
						if(c == -1)
						perror("error");
				}			
		}	
		if(activePCount == 0)
			write(fd,"No active process to terminate\n",strlen("No active process to terminate\n"));	
		else write(fd,"All active processes terminated\n",strlen("All active processes terminated\n"));	
		
	}
	else{
			int returnID;
			
			for(int i = 0; i<processCounter; i++)
			{
				if(strcmp(processArray[i]->name, token) == 0 && processArray[i]->active == 1) 
						{	
							returnID = i;
							break;
						}
			}
		
			int c = kill(processArray[returnID]->id,SIGTERM);
		
			if(c == 0 && returnID != 0)
			{
				char buff[100];
		
				int count = sprintf(buff,"Prcocess %s, pid %d terminated\n",processArray[returnID]->name,processArray[returnID]->id);
		
				write(fd,buff,count);
			}
			else write(fd,"Process has already terminated or is not in the list\n",strlen("Process has already terminated or is not in the list\n"));
		}	
}
//killing process by name
char* methodArithmetic(char * token, char * buff)
{
	char * identifier = token;
	
	int sum = 0;
	
	int firstNumber = 0;
	
	int divideZero = 0;
	
	int check = 0;
	
	int arguments = 0;
	
	if((token = strtok(NULL," \n")) == NULL)
		{
			strcpy(buff,MESSAGE);
			
			return buff;
		}
	
	while(token != NULL)
	{
		check = checkDigit(token);
					if(check == 0)
						break;
			
						
		if(strcasecmp("add",identifier) == 0)
			{
				sum += atoi(token);
			}
		else if(strcasecmp("sub",identifier) == 0)
			{	
			
				if(firstNumber == 0)
					{	
						sum = atoi(token);
						
						firstNumber = 1;
					}
				else sum -= atoi(token);
			}
		else if(strcasecmp("mult",identifier) == 0)
			{
				if(firstNumber == 0)
					{	
						sum = atoi(token);
						
						firstNumber = 1;
					}
				else sum *= atoi(token);
			}
		else if(strcasecmp("div",identifier) == 0)
			{
				if(firstNumber == 0)
					{	
						sum = atoi(token);
						
						firstNumber = 1;
					}
				else 
					{ 
						if( atoi(token) == 0)
								{	
									divideZero = 1;
					
									break;
								}
						sum = sum / atoi(token);
					}	
						
			}
	
			arguments++;
	
			token = strtok(NULL," \n");
		}
		
	
	if(check == 0)
		strcpy(buff,"Invalid arguments\n");
	
	else if(divideZero == 1)
				strcpy(buff,DIVIDE);
				
			
	else if ( arguments == 1)
			strcpy(buff,"Too few arguments! Atleast two required\n");
			
	else
		 	{	int count = sprintf(buff,"result of %s : %d",identifier,sum);

		 		buff[count] = '\n';

		 		count++;

		 		buff[count] = '\0';
		 	}
		
	return buff; 	

}	
//check digit for finding any character other than numbers
int checkDigit(char * token)
{
	int i = 0;
	
	char buff[20];
	
	strcpy(buff, token);
	
	int c = 0;
	
	while(i < strlen(token))
	{
		 c  = isdigit(buff[i]);
		 
			if(c == 0)
				break;
		i++;
	}

	if(c  == 0)
		return 0;

	else return 1;
}
//recording end time
void saveEndTime(int pid, time_t end)
{
	for(int i = 0;i<processCounter;i++)
		{
			if(processArray[i]->id == pid)
				{
					processArray[i]->end = end;
					break;
				}
		}
		
}
void printArray(int fd)
{	
	int i = 0;
	
	char buff[2000];
	
	int count = 0;
	
	struct tm * time_sec;
	
	struct tm * time_secE;
	
	count += sprintf(&buff[count],"[ Process ID  ] [ Active ] [ start time  ] [   End time   ] [ Elapsed time ] [  Process name ]\n");
	
	
	for(;i<processCounter;i++)
		{
			time_sec = localtime(&(processArray[i]->start));
			int hrS = time_sec->tm_hour;
			int mmS = time_sec->tm_min;
			int ssS = time_sec->tm_sec;
			
			int hrE = 0;
			int mmE = 0;
			int ssE = 0;
			if(processArray[i] -> end != -1)
				{
					time_secE = localtime(&(processArray[i]->end));
					hrE = time_sec->tm_hour;
					mmE = time_sec->tm_min;
					ssE = time_sec->tm_sec;
				}
			count += sprintf(&buff[count],"  %d : %d  :%u:%u:%u : %u:%u:%u : %d :: %s :\n",processArray[i]-> id,processArray[i]->active,hrS,mmS,ssS,hrE,mmE,ssE,(int)time(NULL) - (int)processArray[i]->start,processArray[i]->name);
		}
	
		write(fd,buff,count);
}

void printActiveArray(int fd)
{
	int i = 0;
	
	int count=0;
	
	int activeProcesses = 0;
	
	char buff[1000];
	
	struct tm * time_sec;
	
	struct tm * time_secE;
	
	count += sprintf(&buff[count],"[ Process ID  ][ start time  ][ Elapsed time ][ Process name ]\n");
	
	for(;i<processCounter;i++)
		{
			if(processArray[i]->active == 1) 
			  {
			  	time_sec = localtime(&(processArray[i]->start));
				int hrS = time_sec->tm_hour;
				int mmS = time_sec->tm_min;
				int ssS = time_sec->tm_sec;
				
				count += sprintf(&buff[count],"%d  %u:%u:%u  %d  %s\n ",processArray[i]->id,hrS,mmS,ssS,(int)time(NULL) - (int)processArray[i]->start,processArray[i]->name);
			  	
			  	activeProcesses++;
			  }
			 
			 buff[count] = '\0';
			 
			 count++;
		}
		
	if( activeProcesses > 0)	
		write(fd,buff,count);
	
	else write(fd,"Currently no active processes listed\n",strlen("Currently no active processes listed\n"));	
}
//remove active array
void removeProcess(int pid)
{
	int i;
	
	for(i = 0; i<processCounter;i++)
	{
		if(processArray[i]->id == pid)
		{ 
				processArray[i]->active = 0;
			  
				break;
		}		
	}
}
//struct serverParentChildNode pipeVariable;
//#######Server###############
int main()
{
	int sock, length;
	struct sockaddr_in server, client;
	int writeCountSocket = 0;
	int readCountSocket = 0;
	char * runArray[20];
	int moreThanOneThreads = 0;//to ensure only one server read thread is connected
	
	if(signal(SIGCHLD, signalHandlerClient) == SIG_ERR)
		{
			perror("error binding handling SIGCHLD"); 
		}
	/*Create Sockets*/
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if( sock < 0)
			{
				perror("error: making socket");
				exit(1);
			}
		
	/*server wildcard entries*/	
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = 0;
	
	if(bind(sock,(struct sockaddr *) &server, sizeof(server)) < 0)
		{
			perror("error");
			exit(1);
		}
	
	length = sizeof(server);
	
	if(getsockname(sock,(struct sockaddr *) &server,(socklen_t *) &length) < 0)
		{
			perror("error");
			exit(1);
		} 
		
	char str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, (void *)&server.sin_addr.s_addr, str,INET_ADDRSTRLEN);
	
	printIpAndPort(server.sin_port,str);//print port number on the server screen
	
	/* listening for connections*/
	listen(sock, 5);
	
	//detach thread attributes
	pthread_t readThread;
	pthread_attr_t myattr;
	pthread_attr_init(&myattr);
	pthread_attr_setdetachstate(&myattr , PTHREAD_CREATE_DETACHED);
	//user control of server
	
	do{
		pthread_t connectThread;//connection accepting thread
		int clientLength = sizeof(client);
		
		//connector Node setting which will be passed to connectThread
		struct acceptConnectionsNode connector;
		connector.identifier = sock;
		connector.clientAddr = client;
		connector.lengthClient = clientLength;
		struct showIpAndPortNode *clientInformation;
		
		//thread creation
		int iret1 = pthread_create(&connectThread, NULL ,acceptConnections,(void *)&connector);
		//waiting for the thread
		void *readMessage;
		//waiting for the thread
		pthread_join(connectThread, &readMessage);
		
		//storing the information in clientlist node
		clientInformation = (struct showIpAndPortNode *)readMessage;
	
		int msgsock =  clientInformation->socket;
		
		clientList.ipAndPortArray[clientList.connectionNo] = clientInformation;
		clientList.connectionNo++;
		
		//creating pipes between server thread of parent and the child server thread
		int pipeDescriptors[2];
		int pipeDescriptorsOne[2];
		
		pipe(pipeDescriptors);
		
		pipe(pipeDescriptorsOne);
		
		//saving pipes id in clientlist node, so that they are available to both server parent and child thread
		clientInformation->writeServerId = pipeDescriptors[1];
		clientInformation->readServerId = pipeDescriptorsOne[0];
		
		clientInformation->readServerChildId = pipeDescriptors[0];
		clientInformation->writeServerChildId = pipeDescriptorsOne[1];
		
		//one time thread creation for the parent server
		int iret2;
		
		if(moreThanOneThreads == 0)
		 	{	
		 		iret2 = pthread_create(&readThread, &myattr ,serverRead,NULL);
				moreThanOneThreads = 1;
			}
		int serverChild = fork();
		
		if(serverChild == 0)
			{
				close(pipeDescriptors[1]);
		
				close(pipeDescriptorsOne[0]);
				
				pthread_t serverChildThread;
				
				/*Implementing SIGCHLD handling in signalhandler*/
				if(signal(SIGCHLD, signalHandler) == SIG_ERR)
				{
				perror("error binding handling SIGCHLD"); 
				}
				
				//creation of thread for child server		
				int iret3 = pthread_create(&serverChildThread, &myattr ,serverChildRead,(void *)clientInformation);
				
				do{
						char readBuffSocket[100];
						
						bzero(readBuffSocket,100);
						
						int status = 1;
						
						 writeCountSocket = 0;
						
						 readCountSocket = 0;
						
						int bufferOverFlow = 0;
						
						char readBuff;
						
						while(status != 0)
							{															
								int readFromSocket = read(msgsock,&readBuff,1);
								
								if(readFromSocket == 0)
									break;
								
								
								readBuffSocket[readCountSocket] = readBuff;
								
								if(status > 100) //note: not the best way to handle buffer overflow error but gets the job done in my case
										{
											bufferOverFlow = 1;
											break;
										}
								readCountSocket++;
							
								ioctl(msgsock,FIONREAD, &status);
							}
							
						char * arthimeticBuff = (char *)malloc(200*sizeof(char));
						
						int argumentRunArray = 0;//for run arguments counter
						
						char * resultBuff;//for arthimetic operations
						
						int ignore = 0;//for run method, an ignore check
						
						if(readCountSocket == 0)//client disconnected for some reasons
						 		{
						 			write(STDOUT_FILENO, "Connection ended\n",strlen("Connection ended\n"));
						 			killProcessByName("all", STDOUT_FILENO);//i have used the same method for killing by name (all) 
						 			exit(EXIT_SUCCESS);
						 		}
					
						char * token;
						
						if(readCountSocket - 1 == 0 || bufferOverFlow == 1)
							token = "a"; //garbase value so that no if else conditions becomes true
							
						else token = strtok(readBuffSocket," \n");
						
						//case comparing 

						if(strcasecmp("add",token) == 0)
						{
							char * resultBuff = methodArithmetic(token, arthimeticBuff);
							
							writeCountSocket = write(msgsock, resultBuff, strlen(arthimeticBuff));
						}
						else if(strcasecmp("sub",token) == 0)
						{
							resultBuff = methodArithmetic(token, arthimeticBuff);
							
							writeCountSocket = write(msgsock, resultBuff, strlen(arthimeticBuff));
		 			
						}	
						else if(strcasecmp("mult",token) == 0)
						{
							resultBuff = methodArithmetic(token, arthimeticBuff);
		
							writeCountSocket = write(msgsock, resultBuff, strlen(arthimeticBuff));
		 			
						}
						else if(strcasecmp("div",token) == 0)
						{
							 resultBuff = methodArithmetic(token, arthimeticBuff);
		
							writeCountSocket = write(msgsock, resultBuff, strlen(arthimeticBuff));
		 			
						}
						else if(strcasecmp("run",token) == 0)
						{
							if((token = strtok(NULL," \n")) == NULL)
							  		write(msgsock,MESSAGE,strlen(MESSAGE));
							
							else{  	
								
								int i = 0;
								
								while(token != NULL)
									{	
										if( i == 0)
										{	
											runArray[i] = (char *)malloc(30*sizeof(char));//filling the arguments for exec
											
											strcpy(runArray[i],token);
											
											i++;
										}
								
										runArray[i] = (char *)malloc(30*sizeof(char));
							
										strcpy(runArray[i],token);
							
										i++;
								
										token = strtok(NULL, " \n");
										
									}
									
								runArray[i] = NULL;
								
								argumentRunArray = i;
								
								ignore = 1;
							
								}
						}
						//listing processes	
						else if(strcasecmp("list",token) == 0)
						{
							if(processCounter > 0)
								{	
									token = strtok(NULL," \n");
			
									if(token == NULL)
										printActiveArray(msgsock);
					
									else if(strcmp(token,"all") == 0)
									{	
										if((token = strtok(NULL," \n")) == NULL)
												printArray(msgsock);
						
										else write(msgsock,"Invalid command\n",strlen("Invalid command\n"));
									}
	
									else write(msgsock,"Invalid command\n",strlen("Invalid command\n"));
	
								}	
								
							else write(msgsock,"List is currently Empty\n",strlen("List is currently Empty\n"));	
						}
						else if(strcasecmp("kill",token) == 0)
						{
							if(processCounter > 0)
								{	token = strtok(NULL," \n");
		
									if(token==NULL)
									{
										write(msgsock,"Invalid command\n",strlen("Invalid command\n"));
									}
									else {
							
											char * stoken = token;
				
											token = strtok(NULL," \n");
				
											if(token == NULL)
											{
												int pid = 0;
											
												int kcount = sscanf(stoken,"%d",&pid);
					
												int check = checkDigit(stoken);
					
												if(check == 0)
													{
														killProcessByName(stoken, msgsock);
												
													}
					
												else {
														kcount = kill(pid,SIGTERM);
							
														if(kcount == -1)
															write(msgsock,"Invalid process ID\n",strlen("Invalid process ID\n"));
															
														else write(msgsock,"Process Terminated\n",strlen("Process Terminated\n"));
					
														}
													}
												else write(msgsock,"Invalid arguments\n",strlen("Invalid arguments\n"));	
										}
									}
									else write(msgsock,"No processes listed\n",strlen("No processes listed\n"));
						}
						else if(strcasecmp("help",token) == 0)
						{
							token = strtok(NULL," \n");
		
							if(token == NULL)
								write(msgsock,HELP,strlen(HELP));
									
							else write(msgsock,MESSAGE,strlen(MESSAGE));		
						}
						else if(strcasecmp("disconnect",token) == 0)
						{
							token = strtok(NULL," \n");
		
							if(token == NULL)
								{	killProcessByName("all", STDOUT_FILENO);
									write(msgsock,"Connection disconnected\n",strlen("Connection Disconnected\n"));
									exit(EXIT_SUCCESS);//test
								}
									
							else write(msgsock,MESSAGE,strlen(MESSAGE));		
						}
						else if(strcasecmp("exit",token) == 0)
						{
							token = strtok(NULL," \n");
		
							if(token == NULL)
									{	write(msgsock,"Connection terminated\n",strlen("Connection terminated\n"));
										write(STDOUT_FILENO,"Exited\n",strlen("Exited\n"));
										killProcessByName("all", STDOUT_FILENO);
										exit(EXIT_SUCCESS);//test
									}
							else write(msgsock,MESSAGE,strlen(MESSAGE));		
						}
						else{
							if(bufferOverFlow == 1)
							{
								char dumpBuffer[1000];
								
								read(msgsock,dumpBuffer,1000); //if bufferOverFlow is true, then to dump rest of the data from the socket empty, a dumpBuffer is user so that socket becomes empty
								
								writeCountSocket = write(msgsock, "Buffer overflow!\n", strlen("Buffer overflow!\n"));
							}
							else writeCountSocket = write(msgsock, "Invalid command\n", strlen("Invalid command\n"));
						}
						
						//###Running processes on the server#####
						
						if(ignore == 1)//ignore check becomes true when a run command is given
							{
							 
							 	int status;
		
									int sd[2];
									
									int serverPipe = pipe(sd);
	
									if(serverPipe == -1 )
										{
											perror("error");
										}
											fcntl(sd[1],F_SETFD,FD_CLOEXEC);//pipe closing on exec
		
									int serverPid = fork();
									 
									 	if(serverPid == -1)
									 		perror("error");
									 		
									if(serverPid == 0)
									{	
										
										int count = execvp(runArray[0],&runArray[1]);
										
											if(count == -1)
												{	
												
													close(sd[0]);
													
													perror("error");
													
													write(sd[1],"salman",6);//some random character to make condition in child false
													
													close(sd[1]);
													
													exit(1);
												}	
				
									}
									else if(serverPid > 0)
									{
										int waitChild = waitpid(serverPid, &status,WNOHANG);
			
										close(sd[1]);
			
										char writingbuff[50];
			
										int count = read(sd[0],writingbuff,50);
										
										
										if(count == 0)
											{
												//filling the process array
												processArray[processCounter] = (struct Node *)(malloc(50*sizeof(struct Node))); 
												processArray[processCounter] -> id = serverPid;
												strcpy(processArray[processCounter] -> name, runArray[0]);
												processArray[processCounter] -> active = 1;
												processArray[processCounter] -> start = time(NULL);
												processArray[processCounter] -> end = -1;
												processCounter++;
										  		write(msgsock,"Executing...\n",strlen("Executing...\n"));
										  		
											}	
				
										else 
											{		
													write(msgsock,"Invalid program execution\n",strlen("Invalid program execution\n"));
											}
											
									close(sd[0]);	
									
									}	
									
						
						
							}
						
						for(int j = 0 ;j<argumentRunArray; j++)
									free(runArray[j]);//freeing the memory
						
						
						free(arthimeticBuff); 	
							
						resultBuff = NULL;
						
						token = NULL;
				//client and child server communication###########
					
					}while(readCountSocket != 0); 
				close(msgsock);//closing the socket after the disconnet or exit
			}
		
		else if(serverChild > 0)
		{
		clientInformation->pid = serverChild;//storing the pid in clientList node
		
		close(pipeDescriptors[0]);
		
		close(pipeDescriptorsOne[1]);
			
		}

	}while(TRUE);

}

//thread 1
//It accepts information and stores some in clientList node. And passes that to return value in pthread_join
void * acceptConnections(void * ptr)
{
	int * sock = (int *)ptr;
	
	struct acceptConnectionsNode *acceptConnector;
	
	struct showIpAndPortNode *clientInformation = (struct showIpAndPortNode*)malloc(sizeof(struct showIpAndPortNode));
	
	//struct returnFromAccept *returnPortAndSocket = (struct returnFromAccept *)malloc(sizeof(struct returnFromAccept));
	
	acceptConnector = (struct acceptConnectionsNode *) ptr;
	
	int returnValue = accept(acceptConnector->identifier,(struct sockaddr *)&acceptConnector->clientAddr, &acceptConnector->lengthClient);
	
	if(returnValue == -1)
		{	
			pthread_exit(NULL);
		}	
	else {*sock = returnValue;
	
	char str[INET_ADDRSTRLEN];
	
	inet_ntop(AF_INET, (void *)&acceptConnector->clientAddr.sin_addr.s_addr, str,INET_ADDRSTRLEN);

	clientInformation->socket = returnValue;
	
	clientInformation->portNo = ntohs(acceptConnector->clientAddr.sin_port);
	
	strcpy(clientInformation->ipAddress,str);
	
	write(clientInformation->socket,"Welcome to the server developed by saad486::\n",strlen("Welcome to the server developed by saad486::\n"));
	
	pthread_exit((void *)clientInformation);//return struct
	}
}
//server Read thread which makes server parent interactivr
/*
some basic commands it can respond to\
-list processes
-list client
-list processes IP
-say ----  //write anything after say followed by space; it will be broadcasted to every client

*/
void * serverRead(void *ptr)
{
	while(1)
		{	
			char readBuff[100];
			
			bzero(readBuff,100);
			
			write(STDOUT_FILENO,"Enter the command:\n",strlen("Enter the command:\n"));
	
			int readCount = read(STDIN_FILENO,readBuff,100);
	
			char * token = strtok(readBuff," ");
	
			char result[1000];
	
			int count;
			
			if(token == NULL)
				token = "a";//garbage
	
			if(strcasecmp(token,"list") == 0)
				{
					token = strtok(NULL," \n");
					
					if(token == NULL)
						{
							write(STDOUT_FILENO,"Invalid command\n",strlen("Invalid command\n"));
						} 
					
					else if(strcasecmp(token,"processes") == 0)
						{
							
							if(clientList.connectionNo == 0)
									write(STDOUT_FILENO,"No clients processes listed\n",strlen("No clients processes listed\n"));
									
							else if((token = strtok(NULL," \n")) != NULL)
							{
							
								if(strcasecmp(token,"IP") == 0)
									{	
										 for(int i = 0; i < clientList.connectionNo;i++)
											{	int readCount = write(clientList.ipAndPortArray[i]->writeServerId,"Give IP\n",strlen("Give IP\n"));
									
													count = read(clientList.ipAndPortArray[i]->readServerId,result,1000);
														write(STDOUT_FILENO,result,count);
											}			
										}	
									else write(STDOUT_FILENO,"Invalid command\n",strlen("Invalid command\n"));
							}
							else{
									for(int i = 0; i < clientList.connectionNo;i++)
										{	int readCount = write(clientList.ipAndPortArray[i]->writeServerId,"Give Processes\n",strlen("Give Processes\n"));//some random string to pass the value to the pipes
									
											count = read(clientList.ipAndPortArray[i]->readServerId,result,1000);
												write(STDOUT_FILENO,result,count);
										}
								}			
						}
					else if(strcasecmp(token,"clients") == 0)
					{
						printClients();
					}
					
					else write(STDOUT_FILENO,"Invalid command\n",strlen("Invalid command\n"));
					
				}
				else if(strcasecmp(token,"say") == 0)
					{
						if(clientList.connectionNo !=0 )
						{	char sayBuff[500];
							//readBuff[readCount - 1] = '\n';
							token = strtok(NULL,"\n");
							strcpy(sayBuff,token);
							
							char respondBuff[500];
							int sCount = sprintf(respondBuff,"Server says : %s \n",sayBuff);
							
							for(int i=0; i<clientList.connectionNo; i++)
								{
									
									write(clientList.ipAndPortArray[i] -> socket,respondBuff,sCount);
								}
							
						}
					else write(STDOUT_FILENO,"No clients to broadcast\n",strlen("No clients to broadcast\n"));
				}
			else write(STDOUT_FILENO,"Invalid command\n",strlen("Invalid command\n"));
		}
}
//thread of child responding to the parent server read
void * serverChildRead(void *ptr)
{
	while(1)
	{	
		struct showIpAndPortNode * pipeInput = (struct showIpAndPortNode *)ptr;
	
		char buff[100];
	
		int count = read(pipeInput->readServerChildId, buff, 100);
		
		buff[count - 1] = '\0';
		
		if(strcmp(buff,"Give Processes") == 0)
			printClientProcess(pipeInput);
		
		else if (strcmp(buff,"Give IP")==0)
			printProcessIp(pipeInput);
	}

}
//print client on parent server
void printClients()
{
	char readListBuff[1000];
	int noOfClients = 0;

	if(clientList.connectionNo == 0)
	{
		write(STDOUT_FILENO, "No clients listed\n",strlen("No clients listed\n"));
	}
	else{					
		for(int i = 0; i<clientList.connectionNo;i++)
		{
			noOfClients += sprintf(&readListBuff[noOfClients],"Ip = %s and port = %d\n",clientList.ipAndPortArray[i]->ipAddress,clientList.ipAndPortArray[i]->portNo);
		}
	
		write(STDOUT_FILENO, readListBuff, noOfClients);
	}	
}
//print processes plus ip on server parent
void printProcessIp(struct showIpAndPortNode * pipeInput)
{
	int i = 0;
	
	char buff[2000];
	
	int count = 0;
	
	if(processCounter == 0 )
		{
			count += sprintf(&buff[count],"No processes listed\n");
		}
	else{
		for(;i<processCounter;i++)
			{
				count += sprintf(&buff[count],"IP address = %s : Process ID = %d,Process name = %s\n",pipeInput->ipAddress, processArray[i]-> id,processArray[i]->name);
			}
		}
		write(pipeInput->writeServerChildId,buff,count);
} 

//print all of the client processes on parent server
void printClientProcess(struct showIpAndPortNode * pipeInput)
{
	int i = 0;
	
	char buff[2000];
	
	int count = 0;
	
	count = sprintf(&buff[count],"IP = %s and Port no = %d\n",pipeInput->ipAddress,pipeInput->portNo);
	
	if(processCounter == 0 )
		{
			count += sprintf(&buff[count],"No processes listed\n");
		}
	else{
		for(;i<processCounter;i++)
			{
				count += sprintf(&buff[count],"Process ID = %d, Active = %d, Process name = %s\n", processArray[i]-> id,processArray[i]->active,processArray[i]->name);
			}
		}
		write(pipeInput->writeServerChildId,buff,count);
}
//search the index in clientList by the help of the pid extracted from the signal handler
int getIndexClient(int pid)
{
	int returnIndex;
	
	for(int i = 0; i<clientList.connectionNo; i++)
		{
			if(clientList.ipAndPortArray[i]->pid == pid)
				{
					returnIndex = i;
					break;
				}	
		}
	return returnIndex;
}
//remove the node from client list, searching by the pid
void removeProcessClient(int pid)
{
	int index = getIndexClient(pid);
	free(clientList.ipAndPortArray[index]);
	
	int pos;
	
	for(pos = index+1;pos<clientList.connectionNo;pos++)
	{
		int i = pos - 1;
		clientList.ipAndPortArray[i] = clientList.ipAndPortArray[pos];
		
	}
	clientList.connectionNo--;
	
}


