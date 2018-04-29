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

struct writeStructure{
	
char * writeStructureBuff;
int writeCount;
int socketNumber;

};
struct readStructure{
	
char * readStructureBuff;
int socketNumber;

};



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
	
	while(1)
	{
		char *writeBuff = (char *)malloc(100 * sizeof(char));
		struct writeStructure writeObject;
		int readFromScreen = read(STDIN_FILENO, writeBuff, 100);
		writeObject.writeStructureBuff = writeBuff;
		writeObject.writeCount = readFromScreen;
		writeObject.socketNumber = sock;
		int iret1 = pthread_create(&thread1, &myattr,readThread,(void *)&writeObject);
		
		struct readStructure readObject;
		char *readBuff = (char *)malloc(1000 * sizeof(char));
		readObject.readStructureBuff = readBuff;
		readObject.socketNumber = sock;
		int iret2 = pthread_create(&thread2, NULL,writeThread,(void *)&readObject);
		
		void * r1;
		pthread_join(thread2, &r1);
		
		int *readCount = (int *)r1;
		write(STDOUT_FILENO, readObject.readStructureBuff, *readCount);
		
		free(writeBuff);
		free(readBuff);
		free(readCount);
	}
	
	close(sock);

}

void * readThread(void *ptr)
{
	struct writeStructure * writeObject = (struct writeStructure *)ptr;
	
	int writeCount = write(writeObject->socketNumber, writeObject->writeStructureBuff, writeObject->writeCount);
	
	if(writeCount == -1)
		{
			perror("error");
		}
}
void * writeThread(void *ptr)
{
	struct readStructure * readObject = (struct readStructure *)ptr;
	
	struct readStructure readStruct = *readObject;
	
	int readCount = read(readStruct.socketNumber, readStruct.readStructureBuff, 1000);
	
	int * count = (int *)malloc(sizeof(4));
	
	*count = readCount;
	
	pthread_exit((void *)count);
}	
	
	
