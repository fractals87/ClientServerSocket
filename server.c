#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <arpa/inet.h>


/*
struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
};

*/
void DelLf(char* str);
int CheckInt(char str[]);
void ClearBuffer(char* buffer);
void l(char* str);

int main(int argc, char *argv[]) {

    int simpleSocket = 0;
    int simplePort = 11000;
    int returnStatus = 0;
    struct sockaddr_in simpleServer;
    char buffer[256] = "";
	char clientPort[5] = "";
	char MESSAGE[256] = "";
	char strLog[500];
	
	socklen_t len;
	struct sockaddr_storage addr;
		
	int nInteraction;		
	/*
    if (2 != argc) 
	{
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
	*/
	printf("Insert port or write 'def' for default param: ");
	scanf("%s", buffer);
	if(strcmp(buffer,"def")!=0)
	{
		scanf("%d", &simplePort);
	}

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

    /* retrieve the port number for listening */
    //simplePort = atoi(argv[1]);

    /* setup the address structure */
    /* use INADDR_ANY to bind to all local addresses  */
    memset(&simpleServer, '\0', sizeof(simpleServer)); 
    simpleServer.sin_family = AF_INET;
    simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
    simpleServer.sin_port = htons(simplePort);

    /*  bind to the address and port with our socket  */
    returnStatus = bind(simpleSocket,(struct sockaddr *)&simpleServer,sizeof(simpleServer));

    if (returnStatus == 0) 
	{
	    fprintf(stderr, "Bind completed!\n");
    }
    else 
	{
        fprintf(stderr, "Could not bind to address!\n");
		close(simpleSocket);
		exit(1);
    }

    /* lets listen on the socket for connections      */
    returnStatus = listen(simpleSocket, 5);

    if (returnStatus == -1) 
	{
        fprintf(stderr, "Cannot listen on socket!\n");
		close(simpleSocket);
        exit(1);
    }

    while (1)
    {
        struct sockaddr_in clientName = { 0 };
		int simpleChildSocket = 0;
		int clientNameLength = sizeof(clientName);

		/* wait here */

        simpleChildSocket = accept(simpleSocket,(struct sockaddr *)&clientName, &clientNameLength);

		if (simpleChildSocket == -1) {
			fprintf(stderr, "Cannot accept connections!\n");
			close(simpleSocket);
			exit(1);
		}


		char *clientIP = inet_ntoa(clientName.sin_addr);

		/* handle the new connection request  */
		/* write out our message to the client */
		/*ONLY WITH IPV4*/
		//inet_ntop(AF_INET, &clientName.sin_addr, clientIP, sizeof(clientIP));
		sprintf(clientPort, "%d", htons(clientName.sin_port));

		ClearBuffer(MESSAGE);
		strcpy(MESSAGE,"Reti 1 Project! Welcome\nYour IP: ");
		strcat(MESSAGE,clientIP);
		strcat(MESSAGE, " Port: ");
		strcat(MESSAGE,clientPort);
		
		write(simpleChildSocket, MESSAGE, strlen(MESSAGE));
		/* Get number of interacition   */
		ClearBuffer(MESSAGE);
		returnStatus = read(simpleChildSocket, buffer, sizeof(buffer) - 1);
		if (returnStatus <= 0) {
			fprintf(stderr, "Error read client message\n");
	        close(simpleChildSocket);
            continue;
		}
		nInteraction = CheckInt(buffer);
		if(nInteraction<=0){
			fprintf(stderr, "Error number of interactions\n");
	        close(simpleChildSocket);
            continue;			
		}
		ClearBuffer(MESSAGE);
		ClearBuffer(buffer);
		int i;
		//sprintf(strLog, "pippo %d", nInteraction);
		//l(strLog);
		for (i = 0; i < nInteraction; i++ )
		{
			//fprintf(stderr, "attendo");
			returnStatus = read(simpleChildSocket, buffer, sizeof(buffer) - 1);
			if ( returnStatus > 0 ) 
			{
				strcpy(MESSAGE,"ECHO: ");
				strcat(MESSAGE, buffer);
				sprintf(strLog, "ll %s ll", buffer);
				l(strLog);
				write(simpleChildSocket, MESSAGE, strlen(MESSAGE));
			} 
			else 
			{
				fprintf(stderr, "Return Status = %d \n", returnStatus);
			}

			
			ClearBuffer(MESSAGE);
			ClearBuffer(buffer);
		}
		returnStatus = read(simpleChildSocket, buffer, sizeof(buffer));	
		if ( returnStatus > 0 ) 
		{			
			DelLf(buffer);
			if(strcmp(buffer,"bye")==0){
				l("scrivo bye");
				ClearBuffer(buffer);
				strcpy(buffer,"ack\n");
				write(simpleChildSocket, buffer, strlen(buffer));
			}else{
				fprintf(stderr, "Missing bye");
			}
			close(simpleChildSocket);
		}else{
			fprintf(stderr, "Return Status = %d \n", returnStatus);
						
		}
    }

    close(simpleSocket);
    return 0;
}

void ClearBuffer(char* buffer){
	memset(buffer, 0, 256);
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

void l(char* str){
   FILE *fptr;
   fptr = fopen("log.txt","a");

   if(fptr == NULL)
   {
      printf("Error!");   
      exit(1);             
   }

   fprintf(fptr,"%s\n",str);
   fclose(fptr);

   return;
}