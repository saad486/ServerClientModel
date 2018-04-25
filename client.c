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
struct Node{
	int id;
	int active;
	char name[50];
};

void printProcessArray(); 

struct nodeProcessArray{

	int processCounter;
	
	struct Node processArray[30];

};

struct nodeProcessArray processListing;

void printProcessArray()
{
	char buff[1000];
	int count = 0;
	
	for(int i = 0; i<processListing.processCounter;i++)
		{
			count += sprintf(&buff[count],"Process ID = %d, Active = %d, Process Name = %s\n",processListing.processArray[i].id,processListing.processArray[i].active,processListing.processArray[i].name);
		}
	//buff[count] = '\0';
	
	count++;
	
	write(STDOUT_FILENO, buff, count);
}

int main(int argc, char * argv[])
{
	int sock;
	struct sockaddr_in server;
	struct hostent *hp;
	char buf[1024];
	
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
		
	
	while(1)
	{
		char * writeBuff = (char *)malloc(100 * sizeof(char));
		
		int readFromScreen = read(STDIN_FILENO, writeBuff,100);
		
		write(sock , writeBuff, readFromScreen);
		
		//writeBuff[readFromScreen - 1] = '\0';
		
		char * token = strtok(writeBuff,"  \n");
		
		char readBuff;
		
		int readFromSocket = 0;
		
		int count = 0;
		
		char readWriteBuff[100];
		
		bzero(readWriteBuff,100);
		
		int status;
	
		do{
				readFromSocket = read(sock,&readBuff,1);
			
				readWriteBuff[count] = readBuff;
	
				count++;
		
				ioctl(sock,FIONREAD, &status);
		
		}while(status != 0);
		
		if((strcasecmp("list",token) == 0) && (count == sizeof(struct nodeProcessArray)))
		{
			printf("size = %s\n",token);
			memcpy(&processListing, readWriteBuff, count);
			printProcessArray();	
				
		}

		else write(STDOUT_FILENO, readWriteBuff,count);
		
		free(writeBuff);
	
	}
	
	close(sock);

}
