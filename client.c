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
#include<sys/ioctl.h>
#include<strings.h>
#include<pthread.h>

struct writeThreadStruct{
	
	int socket;
	pthread_t thread;
};

#define WELCOME "Welcome\n"

void * readThread(void *ptr);

void * writeThread(void *ptr);

int main(int argc, char * argv[])
{
	while(1)
		{	
				int sock;
				struct sockaddr_in server;
				struct hostent *hp;
	
				/*Create socker*/
				sock = socket(AF_INET, SOCK_STREAM, 0);
				if(sock < 0)
					{
						perror("socket creating");
						exit(1);
					}
	
				/*connect socket using name specified by the command line*/
				server.sin_family = AF_INET;
	
				write(STDOUT_FILENO,WELCOME,strlen(WELCOME));//opening promopt
	
				write(STDOUT_FILENO,"type connect IP and Port\n",strlen("type connect IP and Port\n"));
	
				char openningBuff[100];
				
				char * token;
	
				int openningRead = read(STDIN_FILENO,openningBuff,100);
				
				if(openningBuff[0] == '\n')
						 token = "a";
				
				else token = strtok(openningBuff," \n");
	
					if(strcasecmp("connect",token) == 0)
					{
						int arguments = 0;
			
						char *argumentsArray[2];
			
						token = strtok(NULL," ");
			
						while(token != NULL)
						{
							if(arguments > 1)
								break;
				
							argumentsArray[arguments] = token;

							arguments++;
				
							token = strtok(NULL," "); 
						}
			
						if(arguments == 2)
			
							{	hp = gethostbyname(argumentsArray[0]);
								if( hp == 0)
									{
			
										exit(2);
									}
								bcopy(hp -> h_addr, &server.sin_addr, hp->h_length);
								server.sin_port = htons(atoi(argumentsArray[1]));
	
								if(connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0)	
									{
										perror("connect to stream socket");
										exit(0);
									}
								/*create threads*/	
								pthread_t thread1, thread2;
					
								/*pthread_attr_t myattr;
								pthread_attr_init(&myattr);
								pthread_attr_setdetachstate(&myattr , PTHREAD_CREATE_DETACHED);*/
	
								int iret1 = pthread_create(&thread1, NULL,readThread,(void *)&sock);
					
								struct writeThreadStruct writeT;
								writeT.socket = sock;
								writeT.thread = thread1;
		
								int iret2 = pthread_create(&thread2, NULL,writeThread,(void *)&writeT);
	
								pthread_join(thread1,NULL);
								pthread_join(thread2,NULL);
	
								close(sock);
							}
							else write(STDOUT_FILENO,"Invalid input\n",strlen("Invalid input\n"));			
					}
		
					else write(STDOUT_FILENO,"Invalid input\n",strlen("Invalid input\n"));
			}
}

void * readThread(void *ptr)
{
	int * sock = (int *)ptr;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
	
	while(1)
		{	
			char writeBuff[1000];
		
			int readFromScreen = read(STDIN_FILENO, writeBuff, 1000);
	
			if(readFromScreen == -1)
			{
				perror("error");
			}
			
			int writeCount = write(*sock, writeBuff, readFromScreen);
			
			if(writeCount == -1)
				perror("error");
			
		}	
}
void * writeThread(void *ptr)
{
	struct writeThreadStruct *writeT = (struct writeThreadStruct *)ptr;
		
	while(1)
	{
			char readBuff[1000];
			
			int readFromSocket  = read(writeT->socket, readBuff, 1000);
		
			if(readFromSocket == -1)
			{
				perror("error");
			}
			
			int writeCount = write(STDOUT_FILENO, readBuff, readFromSocket);
			
			readBuff[writeCount] = '\0';
			
			if(strcmp("Connection terminated\n",readBuff) == 0)
				{
					exit(EXIT_SUCCESS);
				}
			if(strcmp("Connection disconnected\n",readBuff) == 0)
				{
					pthread_cancel(writeT->thread);
					pthread_exit(NULL);
				}
			
			if(readFromSocket == -1)
				perror("error");
			
	}	
		
}
	
	
	
