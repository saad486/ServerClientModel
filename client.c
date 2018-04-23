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
				write(STDOUT_FILENO, readWriteBuff,count);
		
		free(writeBuff);
	
	}
	
	close(sock);

}
