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

#define awsPort "26708"
#define Host "localhost"
#define MaxLen 256
#define NUL "nomatch"


int main(int argc,char* argv[]){
	
	int socketMonitor;
	struct addrinfo hints,*serverinfo,*p;
	memset(&hints,0,sizeof(hints));

	int result = getaddrinfo(Host,awsPort,&hints,&serverinfo);
	if(result != 0){
		perror ("monitor: getaddrinfo");
		return 1;
	}
	//reused codes
	for(p = serverinfo;p!= NULL;p = p->ai_next){
		if(p->ai_socktype != 1){
			continue;
		}
		socketMonitor = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
		if(socketMonitor == -1){
			perror ("monitor: socket");
			continue;
		}

		if(connect(socketMonitor,p->ai_addr,p->ai_addrlen) == -1){
			perror ("monitor : connect");
			close(socketMonitor);
			continue;
		}
		break;
	}
	
	if(p == NULL){
		perror ("monitor : bind\n");
		return 2;
	}

	printf ("The monitor is up and running.\n");

	while(1){
		// char* send_data = "haha wolaile!";
		// int send_data_len = send(socketMonitor,send_data,sizeof(send_data),0);

		// printf("send_data_len =%d\n",send_data_len);

		char recv_data[MaxLen];
	    memset(&recv_data,0,sizeof(recv_data));
	    int recv_data_len = recv(socketMonitor,recv_data,MaxLen,0);

	    // printf("The data is %s\n",recv_data);

	    char link_id[MaxLen];
	    if(*recv_data == 'Q'){
	    	// printf("Q\n");
	    	
	    	// char link_id[MaxLen];
	        char filesize[MaxLen];
	        char power[MaxLen];
	        memset(&link_id,0,sizeof(link_id));
	        memset(&filesize,0,sizeof(filesize));
	        memset(&power,0,sizeof(power));

	        int index = 1,f = 0,j = 0;
	        while(recv_data[index] != '\0'){
	        	if(recv_data[index] == '#'){
	        		f++;
	        		j = 0;
	        	}else{
	        		if(f == 0)link_id[j] = recv_data[index];
	    	        else if(f == 1)filesize[j] = recv_data[index];
	    	        else power[j] = recv_data[index];
	    	        j++;
	            }
	            index++;
	        }
	        printf ("The monitor received input=<%s>,size=<%s>,and power=<%s> from the AWS\n",link_id,filesize,power);

	    }else{
	    	char c_Tt[MaxLen];
	    	char c_Tp[MaxLen];
	    	char c_T[MaxLen];
	    	memset(&c_Tt,0,sizeof(c_Tt));
	    	memset(&c_Tp,0,sizeof(c_Tp));
	        memset(&c_T,0,sizeof(c_T));

	        char *recv_data_1;
	        recv_data_1 = recv_data + 1;
	        if(strcmp(recv_data_1,NUL) == 0){
	        	printf("Found mo matches for link <%s>\n",link_id);
	        }else{
	        	int index = 1,f = 0,j = 0;
	            while(recv_data[index] != '\0'){
	                if(recv_data[index] == '#'){
	    	            f++;
	    	            j = 0;
	                }else{
	            	    if(f == 0)c_Tt[j] = recv_data[index];
	            	    if(f == 1)c_Tp[j] = recv_data[index];
	    	            if(f == 2)c_T[j] = recv_data[index];
	    	            j++;
	                }
	                index++;
	            }
	            printf ("The result for link <%s>:\n",link_id);
	            printf ("Tt = <%s>ms\n",c_Tt);
	            printf ("Tp = <%s>ms\n",c_Tp);
	            printf ("Delay = <%s>ms\n",c_T);
	        }
	    }
	}
	return 0;
}
