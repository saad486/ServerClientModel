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

void * readThread(void *ptr);

void * writeThread(void *ptr);

int main(int argc, char * argv[])
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
	hp = gethostbyname(argv[1]);
	if( hp == 0)
		{
			
			exit(2);
		}
	bcopy(hp -> h_addr, &server.sin_addr, hp->h_length);
	server.sin_port = htons(atoi(argv[2]));
	
	if(connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0)	
		{
			perror("connect to stream socket");
			exit(0);
		}
	/*create threads*/	
	pthread_t thread1, thread2;
	pthread_attr_t myattr;
	pthread_attr_init(&myattr);
	pthread_attr_setdetachstate(&myattr , PTHREAD_CREATE_DETACHED);
	
	int iret1 = pthread_create(&thread1, NULL,readThread,(void *)&sock);
		
	int iret2 = pthread_create(&thread2, NULL,writeThread,(void *)&sock);
	
	pthread_join(thread1,NULL);
	pthread_join(thread2,NULL);
	
	
	close(sock);

}

void * readThread(void *ptr)
{
	int * sock = (int *)ptr;
	
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
	int * sock = (int *)ptr;
		
	while(1)
	{
			char readBuff[1000];
			
			int readFromSocket  = read(*sock, readBuff, 1000);
		
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
			
			if(readFromSocket == -1)
				perror("error");
			
	}	
		
}
	
	
	
