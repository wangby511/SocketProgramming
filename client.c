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

#define awsPort "25708"
#define Host "localhost"
#define MaxLen 256
#define NUL "nomatch"


int main(int argc,char* argv[]){
	char link_id[50];
	char size[50];
	char power[50];

	if(argc < 4){
		printf ("invalid input\n");
		exit(1);
	}

	strcpy(link_id,argv[1]);
	strcpy(size,argv[2]);
	strcpy(power,argv[3]);
	
	int socketClient,data_len;
	struct addrinfo hints,*s,*p;
	memset(&hints,0,sizeof(hints));

	int result = getaddrinfo(Host,awsPort,&hints,&s);
	if(result != 0){
		perror ("client: getaddrinfo");
		return 1;
	}

    //reused codes
	for(p = s;p!= NULL;p = p->ai_next){
		if(p->ai_socktype != 1){
			continue;
		}
		socketClient = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
		if(socketClient == -1){
			perror ("client: socket");
			continue;
		}

		if(connect(socketClient,p->ai_addr,p->ai_addrlen) == -1){
			perror ("client : connect");
			close(socketClient);
			continue;
		}
		break;
	}
	
	if(p == NULL){
		perror ("client : bind\n");
		return 2;
	}

	printf ("The Client is up and running.\n");

	char send_data[MaxLen];
	memset(&send_data,0,sizeof(send_data));
	strcat(send_data,link_id);
	strcat(send_data,"#");
	strcat(send_data,size);
	strcat(send_data,"#");
	strcat(send_data,power);

	int send_data_len = send(socketClient,send_data,sizeof(send_data),0);

	if(send_data_len == -1){
		perror ("send");
		exit(1);
	}
	printf("The client sent link ID=<%s>,size=<%s>,and power=<%s> to AWS\n",link_id,size,power);


	// printf ("argc = %d\n",argc);
	char recv_data[MaxLen];
	memset(&recv_data,0,sizeof(recv_data));
	int recv_data_len = recv(socketClient,recv_data,MaxLen,0);

	if(strcmp(recv_data,NUL) == 0){
		printf("Found no matches for link <%s>.\n",link_id);
		return 0;
	}
	printf("The delay for link <%s> is <%s>ms\n",link_id,recv_data);

	// char c_Tt[MaxLen];
	// char c_Tp[MaxLen];
	// char c_T[MaxLen];
	// memset(&c_Tt,0,sizeof(c_Tt));
	// memset(&c_Tp,0,sizeof(c_Tp));
	// memset(&c_T,0,sizeof(c_T));

	// int index = 0,f = 0,j = 0;
	// while(recv_data[index] != '\0'){
	//     if(recv_data[index] == '#'){
	//     	f++;
	//     	j = 0;
	//     }else{
	//     	if(f == 0)c_Tt[j] = recv_data[index];
	//     	else if(f == 1)c_Tp[j] = recv_data[index];
	//     	else c_T[j] = recv_data[index];
	//     	j++;
	//     }
	//     index++;
	// }
	// printf ("Tt = <%s>ms\n",c_Tt);
	// printf ("Tp = <%s>ms\n",c_Tp);
	// printf ("Delay = <%s>ms\n",c_T);
	return 0;


	// printf("%s,%s,%s.\n",link_id,size,power);
	// printf("END OF CODE CLIENT!\n");
}
