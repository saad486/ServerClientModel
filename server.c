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

#define TIMEOUT 5
#define TRUE 1
#define START "Welcome : Enter the commands to your liking\n"
#define DIVIDE "Error : division by zero\n"
#define MESSAGE "Invalid command, type help\n"
#define NOELEMENTS "No elements in the list\n"
#define HELP "=========================\nWelcome to our program guide:\nType add number1 number2 ...\nType sub number1 number2 ...\nType mult number1 number2 ...\nType div number1 number2 ...\nType run argument1 ...\nType list : To view currenly running processes and list[ all] for all processes executed\nType kill PID, to kill an actively running prcocess\nType exit : to exit our program\nNote: All the operations are not case-sensitve, example: Add, aDD, ADd are valid\nMinimun number of inputs for arthimetic operation is Two\n=========================\n"

//thread1
void * acceptConnections(void * ptr);

void setPortNo(int port);

int getPortNo();

int getIndexClient(int port);

void removeProcessClient(int port);

void * serverRead(void *ptr);

void printClientProcess(int fd);

void * serverChildRead(void *ptr);
//server child thread
struct serverChildPipeNode{
		int readID;
		int writeID;
};
//pipes array
struct serverParentChildNode{
	
	int writeEnd[10];
	int readEnd[10];
	
	int writeCount;
	int readCount;
};
struct showIpAndPortNode{

	int portNo;
	char ipAddress[INET_ADDRSTRLEN];
	
};

struct ipAndPortArrayNode{
	
	int connectionNo;
	struct showIpAndPortNode ipAndPortArray[30];
};
struct ipAndPortArrayNode clientList;

struct acceptConnectionsNode{
	
	int identifier;
	struct sockaddr_in clientAddr;
	int lengthClient;
		
};

struct Node{
	int id;
	int active;
	char name[15];	
};
//process arrays
void printProcessArray(int fd);

void printActiveProcessArray(int fd);

//making process appear in active in list
void removeProcess(int processID);

//Node array 
struct Node * processArray[30];//if typed exit then it will be freed
int processCounter = 0;

int checkDigit(char * token);

char* methodArithmetic(char * token, char * buff);

static void signalHandler()
{
	int status;
	
	int pid = wait(NULL);
	
	removeProcess(pid); //changing the status of the process to inactive...	
	
}

static int portNumber;

void setPortNo(int port)
{
	portNumber = port;
}

int getPortNo()
{
	return portNumber;
}

//SIGUSR1
static void signalHandlerClient()
{
		removeProcessClient(getPortNo());
	
}

void printIpAndPort(int portNumber, char * ipAddress)
{
	int port = ntohs(portNumber);
	
	char tempBuff[100];
	
	int count = sprintf(tempBuff,"Port number = %d and ip = %s\n", port,ipAddress);
	
	write(STDOUT_FILENO,tempBuff,count);

}

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
		 	{	int count = sprintf(buff,"%d",sum);

		 		buff[count] = '\n';

		 		count++;

		 		buff[count] = '\0';
		 	}
		
	return buff; 	

}	

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

void printArray(int fd)
{	
	int i = 0;
	
	char buff[2000];
	
	int count = 0;
	
	for(;i<processCounter;i++)
		{
			count += sprintf(&buff[count],"Process ID = %d , Active = %d, Process name = %s\n",processArray[i]-> id,processArray[i]->active,processArray[i]->name);
		}
	
		write(fd,buff,count);
}

void printActiveArray(int fd)
{
	int i = 0;
	
	int count=0;
	
	int activeProcesses = 0;
	
	char buff[1000];
	
	for(;i<processCounter;i++)
		{
			if(processArray[i]->active == 1) 
			  {
			  	count += sprintf(&buff[count],"Process ID = %d, Process name = %s\n",processArray[i]->id,processArray[i]->name);
			  	
			  	activeProcesses++;
			  }
			 
			 buff[count] = '\0';
			 
			 count++;
		}
		
	if( activeProcesses > 0)	
		write(fd,buff,count);
	
	else write(fd,"Currently no active processes listed\n",strlen("Currently no active processes listed\n"));	
}

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
//#######Server###############
int main()
{
	int sock, length;
	struct sockaddr_in server, client;
	int writeCountSocket = 0;
	int readCountSocket = 1;
	char * runArray[20];
	struct serverParentChildNode pipeVariable;
	pipeVariable.writeCount = 0;
	pipeVariable.readCount = 0;
	int moreThanOneThreads = 0;
	
	//pipe array between parent server and client
	int readEndPipe[10];
	int writeEndPipe[10];
	
	
	/*Implementing SIGCHLD handling in signalhandler*/
	if(signal(SIGCHLD, signalHandler) == SIG_ERR)
		{
			perror("error binding handling SIGCHLD"); 
		}

	if(signal(SIGUSR1, signalHandlerClient) == SIG_ERR)
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
	
	printIpAndPort(server.sin_port,str);
	
	/* listening for connections*/
	listen(sock, 5);
	
	pthread_t readThread;
	pthread_attr_t myattr;
	pthread_attr_init(&myattr);
	pthread_attr_setdetachstate(&myattr , PTHREAD_CREATE_DETACHED);
	//user control of server
	
	do{
		pthread_t connectThread;
		int clientLength = sizeof(client);
		
		//connector Node setting which will be passed to connectThread
		struct acceptConnectionsNode connector;
		connector.identifier = sock;
		connector.clientAddr = client;
		connector.lengthClient = clientLength;
		
		int iret1 = pthread_create(&connectThread, NULL ,acceptConnections,(void *)&connector);
		//waiting for the thread
		void *readMessage;
		
		pthread_join(connectThread, &readMessage);
		
		
		int * returnMessage = (int *)readMessage;
	
		int msgsock =  *returnMessage;
		
		free(returnMessage);
		
		int pipeDescriptors[2];
		int pipeDescriptorsOne[2];
		
		pipe(pipeDescriptors);
		
		pipe(pipeDescriptorsOne);
		
		pipeVariable.writeEnd[pipeVariable.writeCount] = pipeDescriptors[1];
		pipeVariable.writeCount++;		
		
		pipeVariable.readEnd[pipeVariable.readCount] = pipeDescriptorsOne[0];
		pipeVariable.readCount++;		
		
		int iret2;
		
		if(moreThanOneThreads == 0)
		 	{	
		 		iret2 = pthread_create(&readThread, &myattr ,serverRead,(void *)&pipeVariable);
				moreThanOneThreads = 1;
			}
		int serverChild = fork();
		
		if(serverChild == 0)
			{
				close(pipeDescriptors[1]);
		
				close(pipeDescriptorsOne[0]);
				
				//client and child server communication###########
						struct serverChildPipeNode threadPipe;
						
						threadPipe.readID = pipeDescriptors[0];
						threadPipe.writeID = pipeDescriptorsOne[1];
						
						pthread_t serverChildThread;
						
						int iret3 = pthread_create(&serverChildThread, &myattr ,serverChildRead,(void *)&threadPipe);
				
				do{
						char readBuffSocket[100];
						
						bzero(readBuffSocket,100);
						
						int status = 1;
						
						int readCountSocket = 0;
						
						char readBuff;
						
						while(status != 0 )
							{															
								int readFromSocket = read(msgsock,&readBuff,1);
								
								if(readFromSocket == 0)
									break;
							
								readBuffSocket[readCountSocket] = readBuff;
								
								readCountSocket++;
							
								ioctl(msgsock,FIONREAD, &status);
							}
						
						char * arthimeticBuff = (char *)malloc(50*sizeof(char));
						
						int argumentRunArray = 0;
						
						char * resultBuff;
						
						int ignore = 0;
						
						if(readCountSocket == 0)
						 		{
						 			write(STDOUT_FILENO, "Connection ended\n",strlen("Connection ended\n"));
						 			kill(getppid(),SIGUSR1);
						 			kill(getpid(),SIGTERM);
						 		}
					
						char * token;
						
						if(readCountSocket - 1 == 0)
							token = "a"; //garbage
							
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
											runArray[i] = (char *)malloc(30*sizeof(char));
											
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
						else if(strcasecmp("list",token) == 0)
						{
							if(processCounter > 0)
								{	
									token = strtok(NULL," \n");
			
									if(token == NULL)
										printActiveArray(msgsock);
					
									else if(strcmp(token,"all") == 0)	//check why tokenizing first and checking caused the problem
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
							token = strtok(NULL," \n");
		
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
												write(msgsock,"Invalid arguments\n",strlen("Invalid arguments\n"));
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
								write(msgsock,"Connection disconnected\n",strlen("Connection Disconnected\n"));
									
							else write(msgsock,MESSAGE,strlen(MESSAGE));		
						}
						else if(strcasecmp("exit",token) == 0)
						{
							token = strtok(NULL," \n");
		
							if(token == NULL)
									{	write(msgsock,"Connection terminated\n",strlen("Connection terminated\n"));
										write(STDOUT_FILENO,"Exited\n",strlen("Exited\n"));
										exit(EXIT_SUCCESS);//test
									}
							else write(msgsock,MESSAGE,strlen(MESSAGE));		
						}
						else{
							writeCountSocket = write(msgsock, "Invalid command\n", strlen("Invalid command\n"));
						}
						
						//###Running processes on the server#####
						
						if(ignore == 1)
							{
							 
							 	int status;
		
									int sd[2];
									
									int serverPipe = pipe(sd);
	
									if(serverPipe == -1 )
										{
											perror("error");
										}
											fcntl(sd[1],F_SETFD,FD_CLOEXEC);
		
									int serverPid = fork();
									 
									 	if(serverPid == -1)
									 		perror("error");
									 		
									if(serverPid == 0)
									{	
										//write(sd[1],"saad",4);
										int count = execvp(runArray[0],&runArray[1]);
										
											if(count == -1)
												{	
												
													close(sd[0]);
													
													perror("error");
													
													write(sd[1],"salman",6);
													
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
					
												processArray[processCounter] = (struct Node *)(malloc(50*sizeof(struct Node))); 
												processArray[processCounter] -> id = serverPid;
												strcpy(processArray[processCounter] -> name, runArray[0]);
												processArray[processCounter] -> active = 1;
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
									free(runArray[j]);
						
						//free(readBuffSocket);	
						
						free(arthimeticBuff);	
							
						resultBuff = NULL;
						
						token = NULL;
				//client and child server communication###########
					
					}while(readCountSocket != 0); 
				close(msgsock);
			}
		
		else if(serverChild > 0)
		{
			/*int status;
			
			int pid = waitpid(serverChild, &status, WNOHANG);*/
			//read and write end of no functionality closed
		close(pipeDescriptors[0]);
		
		close(pipeDescriptorsOne[1]);
			
		}

	}while(TRUE);

}

//thread 1
void * acceptConnections(void * ptr)
{
	int * sock = (int *)malloc(sizeof(int));
	
	struct acceptConnectionsNode *acceptConnector;
	
	struct showIpAndPortNode ipAndPortStorage;
	
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

	clientList.ipAndPortArray[clientList.connectionNo].portNo = ntohs(acceptConnector->clientAddr.sin_port);

	strcpy(clientList.ipAndPortArray[clientList.connectionNo].ipAddress,str);
	
	setPortNo(clientList.ipAndPortArray[clientList.connectionNo].portNo);
	
	clientList.connectionNo++;
	
	pthread_exit((void *)sock);//return struct
	}
}

void * serverRead(void *ptr)
{
	while(1)
		{	struct serverParentChildNode * pipeVariable = (struct serverParentChildNode *)ptr;
	
			char readBuff[100];
	
			write(STDOUT_FILENO,"Welcome\n",strlen("Welcome\n"));
	
			int readCount = read(STDIN_FILENO,readBuff,100);
	
			char * token = strtok(readBuff," \n");
	
			char result[1000];
	
			int count;
	
			if(strcasecmp(token,"list") == 0)
				{
					for(int i = 0; i < pipeVariable->readCount;i++)
							{	write(pipeVariable->writeEnd[i],"Give Processes\n",strlen("Give Processes\n"));	
								count = read(pipeVariable->readEnd[i],result,1000);
								write(STDOUT_FILENO,result,count);
							}	
					
				}

			}
}

void * serverChildRead(void *ptr)
{
	while(1)
	{	
		struct serverChildPipeNode * pipeVariable = (struct serverChildPipeNode *)ptr;
	
		char buff[100];
	
		int count = read(pipeVariable->readID, buff, 100);
	
		printClientProcess(pipeVariable->writeID);  
	}

}
void printClientProcess(int fd)
{
int i = 0;
	
	char buff[2000];
	
	int count = 0;
	
	for(;i<processCounter;i++)
		{
			count += sprintf(&buff[count],"Process ID = %d , Active = %d, Process name = %s\n",processArray[i]-> id,processArray[i]->active,processArray[i]->name);
		}
	
		write(fd,buff,count);

}

int getIndexClient(int port)
{
	int returnIndex;
	
	for(int i = 0; i<clientList.connectionNo; i++)
		{
			if(clientList.ipAndPortArray[i].portNo == port)
				{
					returnIndex = i;
					break;
				}	
		}
	return returnIndex;
}

void removeProcessClient(int port)
{
	int index = getIndexClient(port);
	
	if(clientList.connectionNo == 1)
		clientList.connectionNo -= 1;
	 	
	else if(index == clientList.connectionNo - 1)
		clientList.connectionNo -= 1;
	else{
		
		struct showIpAndPortNode temp;
		temp.portNo = clientList.ipAndPortArray[clientList.connectionNo -1].portNo;
		strcpy(temp.ipAddress, clientList.ipAndPortArray[clientList.connectionNo -1].ipAddress);
		
		clientList.ipAndPortArray[clientList.connectionNo -1].portNo = clientList.ipAndPortArray[index].portNo;
		strcpy(clientList.ipAndPortArray[clientList.connectionNo -1].ipAddress, clientList.ipAndPortArray[index].ipAddress);
		
		clientList.ipAndPortArray[index].portNo = temp.portNo;
		strcpy(clientList.ipAndPortArray[index].ipAddress, temp.ipAddress);
		 
		clientList.connectionNo -= 1;
	}
}


