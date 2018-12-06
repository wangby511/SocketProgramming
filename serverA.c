#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define Port "21708"
#define Host "localhost"
#define MaxLen 256

char* searchInDataBase(char *link_id){
	FILE *f = NULL;
	char line[MaxLen];
	char *result = line;
	f = fopen("database_a.csv","r");
	if(f == NULL){
		exit(0);
	}
	while(!feof(f)){
		memset(&line,0,sizeof(line));
		fgets(line,MaxLen,f);
		int index = 0;
		char id[MaxLen];
		memset(&id,0,sizeof(id));
		while(line[index] != ',' && line[index] != '\0'){
			id[index] = line[index];
			index++;
		}
		if(strcmp(id,link_id) == 0){
			return result;

		}
	}
	return "0";
}
int main(int argc,char *argv[]){
	struct addrinfo hints,*s,*p;
	struct sockaddr_storage addr;
	int addr_len = sizeof(addr);
	memset(&hints,0,sizeof(hints));
	int socketA;
	int data_len,func_len;

	if(getaddrinfo(Host,Port,&hints,&s) != 0){
		return 1;
	}
	//reused codes

	for(p = s;p != NULL;p = p->ai_next){

		if(p->ai_socktype != SOCK_DGRAM){
			continue;
		}
		
		if((socketA = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1){
			continue;
		}

		if(bind(socketA,p->ai_addr,p->ai_addrlen) == -1){
			close(socketA);
			continue;
		}
		// printf("%d,%d,%d\n",p->ai_family,p->ai_socktype,p->ai_protocol);
		// printf("%d,%d,%d\n",AF_INET,SOCK_DGRAM,17);

		break;
	}
	
	if(p == NULL){
		printf("Server A: failed to bind socket\n");
		return 1;
	}

	printf ("The Server A is up and running using UDP port <%s>.\n",Port);

	while(1){
		// char function[MaxLen];
		// printf ("while(1): <%s>.\n",PortA);
		char data[MaxLen];
		memset(&data,'\0',sizeof(data));

		data_len = recvfrom(socketA,data,MaxLen,0,(struct sockaddr *)&addr, (socklen_t *)&addr_len);
		// printf ("The server received input <%d>",data_len);

		if(data_len == -1){
			perror ("recvfrom error in serverA in data.");
			exit(1);
		}


		// deal with the received string from client.c
		char link_id[MaxLen];
	    char size[MaxLen];
	    char power[MaxLen];
	    memset(&link_id,0,sizeof(link_id));
	    memset(&size,0,sizeof(size));
	    memset(&power,0,sizeof(power));
	    int index = 0;
	    int f = 0,j = 0;
	    while(data[index] != '\0'){
	    	if(data[index] == '#'){
	    		f++;
	    		j = 0;
	    	}else{
	    		if(f == 0)link_id[j] = data[index];
	    		else if(f == 1)size[j] = data[index];
	    		else power[j] = data[index];
	    		j++;
	    	}
	    	index++;
	    }
	    printf ("The Server A received input <%s>\n",link_id);

	    char *feedback = searchInDataBase(link_id);

	    char *match = "1";
	    if(strcmp(feedback,"0") == 0)match = "0";

	    printf ("The Server A has found <%s> matches\n",match);
	    // printf ("feedback = %d\n",sizeof(feedback));

	    int sendtoAWS;
        if((sendtoAWS = sendto(socketA,feedback,MaxLen,0,(struct sockaddr *)&addr,addr_len)) == -1){
        	perror ("send to AWS");
        	exit(1);
        }
        printf("The Server A finished sending the output to AWS\n");

	}

	return 0;
}
