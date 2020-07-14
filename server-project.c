#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <arpa/inet.h>
#include <math.h>

#define WELCOME "RETI 1 PROJECT, YOUR IP: "
#define CORRECT 1
#define ERROR 0

void DelLf(char* str);
int CheckInt(char str[]);
void ClearBuffer(char* buffer);
int CheckMessage(char buffer[]);

typedef enum {
	OK,
    ERR
} outcome;

typedef enum {
	START,
    SYNTAX,
	DATA,
	STATS
} typeRes;

int main(int argc, char *argv[]) {

    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    struct sockaddr_in simpleServer;
    char buffer[512] = "";
	char MESSAGE[512] = ""; /*<Esito> <Tipo> <Contenuto>*/
	char *token;
	int arrData[255];
	
	socklen_t len;
	struct sockaddr_storage addr;
		
	int arrDataIndex = 0;
	int dataTmp = 0;
	int nElementTot = 0;
	int nElementTmp = 0;
	int nElementElab = 0;
	int indexToken = 0;
	int valid = 1;
	
    if (2 != argc) 
	{
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
	
    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1) 
	{
        fprintf(stderr, "Could not create a socket!\n");
        exit(1);
    }
    else 
	{
	    fprintf(stderr, "Socket created!!\n");
    }

    /* retrieve the port number for listening */
    simplePort = atoi(argv[1]);

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
	    fprintf(stderr, "Bind completed!!\n");
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
		
		arrDataIndex = 0;
		dataTmp = 0;
		nElementTot = 0;
		nElementTmp = 0;
		nElementElab = 0;
		indexToken = 0;
		valid = 1;		

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

		ClearBuffer(MESSAGE);
		strcpy(MESSAGE,"OK START ");
		strcat(MESSAGE, WELCOME);
		strcat(MESSAGE, clientIP);
		strcat(MESSAGE, "\n");
		//fprintf(stderr,"%s", MESSAGE);
		write(simpleChildSocket, MESSAGE, strlen(MESSAGE));

		ClearBuffer(MESSAGE);
		do
		{
			dataTmp = 0;			
			nElementTmp = 0;
			nElementElab = 0;
			indexToken=0;
			valid = 1;
			/* WAITING <Numero_dati> <dato1> <dato2> <datoN>*/
			returnStatus = read(simpleChildSocket, buffer, sizeof(buffer) - 1);
			if (returnStatus <= 0) {
				fprintf(stderr, "Error read client message\n");
				close(simpleChildSocket);
				exit(1);
			}
			if(CheckMessage(buffer)==ERROR){
				close(simpleChildSocket);
				exit(1);
			}
			
			//printf("MESSAGE RECIVE: %s", buffer);
			token = strtok(buffer, " ");
			
			while( token != NULL ) {
				//printf("token %s\n",token);
				if(indexToken==0){
					if(strcmp(token,"0\n")==0){
						dataTmp=0;
						//printf("TOKEN %s", token);						
					}else{				
						dataTmp = CheckInt(token);
						
						//First Element
						if(dataTmp<0){
							strcpy(MESSAGE,"ERR DATA First element should be positive integer\n");
							valid = 0;
							break;
						}
						nElementTmp = dataTmp;
					}
				}else{
					dataTmp = CheckInt(token);
					if(dataTmp<0){
						strcpy(MESSAGE,"ERR DATA Elements should be positive integer\n");
						valid = 0;
						break;						
					}
					arrData[arrDataIndex] = dataTmp;
					arrDataIndex +=1;
					nElementElab +=1;
				}
				indexToken +=1;				
				token = strtok(NULL, " ");				
		    }
			nElementTot += nElementTmp;
			
			//printf("ELAB: %d, FIRST ELE: %d\n",nElementElab,nElementTmp);
			
			//Check NElementElab
			if(valid==1){
				if(nElementTmp != nElementElab){
					strcpy(MESSAGE,"ERR DATA First element is not consistent with the elements sent\n");
					write(simpleChildSocket, MESSAGE, strlen(MESSAGE));			
					close(simpleChildSocket);
					break;					
				}else{
					if(dataTmp!=0){
						strcpy(MESSAGE,"OK DATA ");
						ClearBuffer(buffer);
						sprintf (buffer, "%d", nElementTmp);
						strcat(MESSAGE,buffer);
						strcat(MESSAGE,"\n");
						write(simpleChildSocket, MESSAGE, strlen(MESSAGE));												
					}
				}
			}else{
				write(simpleChildSocket, MESSAGE, strlen(MESSAGE));		
				close(simpleChildSocket);
				break;					
			}

			ClearBuffer(MESSAGE);
			ClearBuffer(buffer);
		}while(dataTmp != 0);

		if(nElementTot==1){
			strcpy(MESSAGE,"ERR STATS The popolation should be greater than 1\n");
			write(simpleChildSocket, MESSAGE, strlen(MESSAGE));	
		}else{
			//CALC AVG 
			float avg;
			int tot=0;
			int i;
			float var;
			for(i = 0; i < nElementTot; i++){
				tot=tot + arrData[i];
			}
			avg = (float)tot/(float)nElementTot;
			
			for(i = 0; i < nElementTot; i++){
				var += pow((float)arrData[i] - avg,2);
			}
			
			var = var / nElementTot;
			
			strcpy(MESSAGE,"OK STATS ");
			sprintf(buffer, "%d ", nElementTot);
			strcat(MESSAGE,buffer);
			ClearBuffer(buffer);
			
			sprintf(buffer, "%.2f ", avg);
			strcat(MESSAGE,buffer);
			ClearBuffer(buffer);
			
			sprintf(buffer, "%.2f\n", var);
			strcat(MESSAGE,buffer);
			ClearBuffer(buffer);

			//printf("MESSAGE STATS: %s\n", MESSAGE);
			write(simpleChildSocket, MESSAGE, strlen(MESSAGE));		
		}
    }

    close(simpleSocket);
    return 0;
}

void ClearBuffer(char* buffer){
	memset(buffer, 0, 512);
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

int CheckMessage(char buffer[]){
	if (strlen(buffer) > 512) {
        printf("ERROR Message too long: %lu char\n", strlen(buffer));
        return ERROR;
    }
	
    if (buffer[strlen(buffer)-1] != '\n') {
        printf("ERROR <LF> not found\n");
        return ERROR;
    }
}
