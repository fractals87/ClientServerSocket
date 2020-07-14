#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define CORRECT 1
#define ERROR 0

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

typedef struct {
	outcome state;
	typeRes type;
	char message[512];
	int nElementElab;
	float avg;
	float var;
} responseStr;

void DelLf(char* str);
int CheckInt(char* str);
void ClearBuffer(char* buffer);
int parse_response(responseStr* response, char buffer[]);
char* substr(char *src, int m, int n);

int main(int argc, char *argv[]) {

    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    char buffer[512] = "";
	char strTmp[512] = "";
    struct sockaddr_in simpleServer;
	
    responseStr response;
	
    if (3 != argc) {
        fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
        exit(1);
    }

    /* create a streaming socket      */
    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1){
        fprintf(stderr, "Could not create a socket!\n");
        exit(1);
    }

    simplePort = atoi(argv[2]);
    memset(&simpleServer, '\0', sizeof(simpleServer));
    simpleServer.sin_family = AF_INET;
    simpleServer.sin_addr.s_addr=inet_addr(argv[1]);	
    simpleServer.sin_port = htons(simplePort);

    /*  connect to the address and port with our socket  */
    returnStatus = connect(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));
    if (returnStatus < 0) {
        fprintf(stderr, "Could not connect to address!\n");
		close(simpleSocket);
		exit(1);
    }

    returnStatus = read(simpleSocket, buffer, sizeof(buffer));
    if (parse_response(&response, buffer)==ERROR) {
        printf("Format response not valid\n");
        close(simpleSocket);
        exit(1);
    }
	
	printf("Welcome message: %s\n", response.message);
	printf("This program asks the number of values ​​to be processed and the respective value.\n");
	printf("Avg and variance are returned\n");
	printf("Insert 0 when ask number of value for terminate the sequence.\n\n");	

	int nValues;
	
	do{
		ClearBuffer(buffer);		
		printf("Insert number of value to be processed (0 for terminate): ");		
		fgets(buffer, sizeof(buffer) - 1, stdin);
		DelLf(buffer);
		if(strcmp(buffer,"0")==0){
			nValues = 0;			
		}else{
			nValues = CheckInt(buffer);			
			if(nValues < 0){
				printf("Value should be positive integer\n");
				continue;
			}
		}
		int i=0;
		int valueTmp;
		int arrValue[nValues];
		arrValue[0] = nValues;
		for (i = 0; i < nValues; i++ )
		{
			valueTmp = 0;
			ClearBuffer(buffer);
			printf("Insert %d° number : ",i+1);
			fgets(buffer, sizeof(buffer) - 1, stdin);
			valueTmp = CheckInt(buffer);
			if(valueTmp<=0){
				printf("number not correct\n");
				i=i-1;
				continue;
			}
			arrValue[i+1] = valueTmp;
		}

		//COMPONE MESSAGE
		ClearBuffer(buffer);
		for(i=0;i<=nValues;i++){
			sprintf (strTmp, "%d", arrValue[i]);
			strcat(buffer,strTmp);
			if(i<nValues){
				strcat(buffer," ");
			}else{
				strcat(buffer,"\n");
			}
		}
		/* SIMULATE ERR SYNTAX */
		//ClearBuffer(buffer);
		//strcpy(buffer, "2 3 4 5\n");
		/* END SIMULATE */
		
		write(simpleSocket, buffer, strlen(buffer));

		if(nValues!=0){
			//WAITING RESULT CONFIRM RECIVE DATA
			ClearBuffer(buffer);		
			returnStatus = read(simpleSocket, buffer, sizeof(buffer));
			if (parse_response(&response, buffer)==ERROR) {
				printf("Format response not valid\n");
				close(simpleSocket);
				exit(1);
			}
			
			if(response.state==ERR){
				printf("ERROR %s\n",response.message);
				close(simpleSocket);
				exit(1);			
			}else{
				valueTmp = CheckInt(response.message);
				if(valueTmp == nValues){
					printf("Server has successfully received %s elements\n",response.message);			
				}else{
					printf("Server has reviced incoerent number of elements\n");													
				}
			}
		}

	}while(nValues != 0);
	
	ClearBuffer(buffer);
	returnStatus = read(simpleSocket, buffer, sizeof(buffer));
	if (parse_response(&response, buffer)==ERROR) {
		printf("Format response not valid\n");
		close(simpleSocket);
		exit(1);
	}
	if(response.state==ERR){
		printf("ERROR %s\n",response.message);
		close(simpleSocket);
		exit(1);			
	}else{
		printf("N° elements: %d\n", response.nElementElab);
		printf("AVG: %.2f\n", response.avg);		
		printf("Variance: %.2f\n", response.var);				
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
	memset(buffer, 0, 512);
}

int parse_response(responseStr* response, char buffer[]) {
	char bufferBk[512];
	stpcpy(bufferBk, buffer);
	if (strlen(buffer) > 512) {
        printf("ERROR Message too long: %lu char\n", strlen(buffer));
        return ERROR;
    }
	
    if (buffer[strlen(buffer)-1] != '\n') {
        printf("ERROR <LF> not found\n");
        return ERROR;
    }
	DelLf(buffer);
	ClearBuffer(response->message);
	
	int nToken = 0;
	char *token;	
	int intTmp = 0;
	int startMsg = 0;
    /* get the first token */
    token = strtok(buffer, " ");

	while( token != NULL ) {
		switch (nToken)
		{
			case 0:
				if (strcmp(token, "OK") == 0){
					response->state = OK;
					startMsg = 3;
				}else if (strcmp(token, "ERR") == 0){
					response->state = ERR;
					startMsg = 4;					
				}else{
					printf("Invalid state");
					return ERROR;					
				}
				break;
			case 1: 
				if (strcmp(token, "START") == 0){
					response->type = START;
					startMsg += 6;					
				}else if (strcmp(token, "SYNTAX") == 0){
					response->type = SYNTAX;
					startMsg += 7;					
				}else if (strcmp(token, "DATA") == 0){
					response->type = DATA;		
					startMsg += 5;
				}else if (strcmp(token, "STATS") == 0){
					response->type = STATS;						
					startMsg += 6;					
				}else{
					printf("Invalid type");
					return ERROR;					
				}
				break;
			case 2:
				if(response->type==STATS && response->state==OK){
					response->nElementElab = CheckInt(token);
				}else{
					strcpy(response->message, substr(bufferBk, startMsg, strlen(bufferBk)-1));	
					return CORRECT;					
				}
				break;
			case 3:
				if(response->type==STATS){
					response->avg = atof(token);
				}else{				
					printf("Too much tokens, '%s', number=%d, type=%d\n",token,nToken,response->type);
					return ERROR;
					break;				
				}
				break;				
			case 4:
				if(response->type==STATS){
					response->var = atof(token);
				}else{				
					printf("Too much tokens, '%s', number=%d, type=%d\n",token,nToken,response->type);
					return ERROR;
					break;				
				}				
				break;
			default: 
				printf("Too much tokens, '%s', number=%d\n",token,nToken);
				return ERROR;
				break;
		}
		nToken +=1;
		token = strtok(NULL, " ");
	}
    return CORRECT;
}

char* substr(char *src, int m, int n){
	int len = n - m;
	char *dest = (char*)malloc(sizeof(char) * (len + 1));
	strncpy(dest, (src + m), len);
	return dest;
}
