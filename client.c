#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void DelLf(char* str);
int CheckInt(char str[]);
void ClearBuffer(char* buffer);
void l(char* str);

int main(int argc, char *argv[]) {

    int simpleSocket = 0;
    int simplePort = 11000;
	char simpleIP[] = "127.0.0.1";
    int returnStatus = 0;
    char buffer[256] = "";
    struct sockaddr_in simpleServer;
	
	int nInteraction;
	char strLog[50];
	//char send_buffer[256] = "";
	/*
    if (3 != argc) {

        fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
        exit(1);

    }
	*/
	printf("Insert ip or write 'def' for default param: ");
	fgets(buffer, sizeof(buffer) - 1, stdin);
	DelLf(buffer);
	if(strcmp(buffer,"def")!=0)
	{
		printf("Insert port: ");
		fgets(buffer, sizeof(buffer) - 1, stdin);
		simplePort=CheckInt(buffer);
		if(simplePort==-1){
			fprintf(stderr, "Port not correct!\n");			
			close(simpleSocket);
			return 0;
		}
	}

    /* create a streaming socket      */
    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1) 
	{
        fprintf(stderr, "Could not create a socket!\n");
        exit(1);
    }
    else 
	{
	    fprintf(stderr, "Socket created!\n");
    }

    /* retrieve the port number for connecting */
    //simplePort = atoi(argv[2]);

    /* setup the address structure */
    /* use the IP address sent as an argument for the server address  */
    //bzero(&simpleServer, sizeof(simpleServer)); 
    memset(&simpleServer, '\0', sizeof(simpleServer));
    simpleServer.sin_family = AF_INET;
    //inet_addr(argv[2], &simpleServer.sin_addr.s_addr);
	simpleServer.sin_addr.s_addr=inet_addr(simpleIP);
    simpleServer.sin_port = htons(simplePort);

    /*  connect to the address and port with our socket  */
    returnStatus = connect(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

    if (returnStatus == 0) 
	{
	    fprintf(stderr, "Connect successful!\n");
    }
    else 
	{
        fprintf(stderr, "Could not connect to address!\n");
		close(simpleSocket);
		exit(1);
    }

    /* get the message from the server   */
    returnStatus = read(simpleSocket, buffer, sizeof(buffer));

    if ( returnStatus > 0 ) 
	{
        printf("%d: %s\n", returnStatus, buffer);
    } 
	else 
	{
        fprintf(stderr, "Return Status = %d \n", returnStatus);
    }
	ClearBuffer(buffer);
	
	printf("Insert number of interactions: ");
	fgets(buffer, sizeof(buffer) - 1, stdin);
	nInteraction = CheckInt(buffer);
	if(nInteraction <= 0)
	{
		fprintf(stderr, "Number of interactions not correct!\n");			
		close(simpleSocket);
		return 0;
	}
	sprintf (buffer,"%d",nInteraction);
	
	write(simpleSocket, buffer, strlen(buffer));
	ClearBuffer(buffer);

	int i;
	for (i = 0; i < nInteraction; i++ )
	{
		//l("printf");	
		printf("Phrase %d: ",i);
		fgets(buffer, sizeof(buffer) - 1, stdin);
		write(simpleSocket, buffer, strlen(buffer));

		ClearBuffer(buffer);
		returnStatus = read(simpleSocket, buffer, sizeof(buffer));
		l(buffer);
		if ( returnStatus > 0 ) 
		{
			printf("%d: %s", returnStatus, buffer);
		} 
		else 
		{
			fprintf(stderr, "Return Status = %d \n", returnStatus);
		}
		ClearBuffer(buffer);
	}
	//l("CLIENT: FINE FOR, inizio read");	
	strcpy(buffer,"bye\n");
	write(simpleSocket, buffer, strlen(buffer));
	
	ClearBuffer(buffer);
	returnStatus = read(simpleSocket, buffer, sizeof(buffer));
	if ( returnStatus > 0 ) 
	{
		DelLf(buffer);
		l(buffer);
		if(strcmp(buffer,"ack")==0){
			printf("Program terminate correctly");
		}else{
			printf("ACK not recived");
		}
	} 
	else 
	{
		fprintf(stderr, "Return Status = %d \n", returnStatus);
	}
	
    close(simpleSocket);
    return 0;
}

int CheckInt(char str[]) {
    DelLf(str);
	int number = atoi(str);
	if(number == 0)
		return -1;
    return number;
}

void DelLf(char* str) {
    if (str[strlen(str) - 1] == '\n')
        str[strlen(str) - 1] = '\0';
}

void ClearBuffer(char* buffer){
	memset(buffer, 0, 256);
}

void l(char* str){
   FILE *fptr;
   fptr = fopen("log_client.txt","a");

   if(fptr == NULL)
   {
      printf("Error!");   
      exit(1);             
   }

   fprintf(fptr,"%s\n",str);
   fclose(fptr);

   return;
}