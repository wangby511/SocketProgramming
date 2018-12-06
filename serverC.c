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

#define Port "23708"
#define Host "localhost"
#define MaxLen 256

int main(int argc,char *argv[]){
	struct addrinfo hints,*s,*p;
	struct sockaddr_storage addr;
	int addr_len = sizeof(addr);
	memset(&hints,0,sizeof(hints));
	int socketC;
	int data_len,func_len;

	if(getaddrinfo(Host,Port,&hints,&s) != 0){
		return 1;
	}
	//reused codes

	for(p = s;p != NULL;p = p->ai_next){

		if(p->ai_socktype != SOCK_DGRAM){
			continue;
		}
		
		if((socketC = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1){
			continue;
		}

		if(bind(socketC,p->ai_addr,p->ai_addrlen) == -1){
			close(socketC);
			continue;
		}
		// printf("%d,%d,%d\n",p->ai_family,p->ai_socktype,p->ai_protocol);
		// printf("%d,%d,%d\n",AF_INET,SOCK_DGRAM,17);

		break;
	}
	
	if(p == NULL){
		printf("Server C: failed to bind socket\n");
		return 1;
	}

	printf ("The Server C is up and running using UDP port <%s>.\n",Port);

	while(1){
		// char function[MaxLen];
		// printf ("while(1): <%s>.\n",PortA);
		char data[MaxLen];
		memset(&data,0,sizeof(data));

		data_len = recvfrom(socketC,data,MaxLen,0,(struct sockaddr *)&addr, (socklen_t *)&addr_len);
		// printf ("The server received input <%d>",data_len);

		if(data_len == -1){
			perror ("recvfrom error in serverC in data.");
			exit(1);
		}

		char link_id[MaxLen];
	    char size[MaxLen];
	    char power[MaxLen];
	    memset(&link_id,0,sizeof(link_id));
	    memset(&size,0,sizeof(size));
	    memset(&power,0,sizeof(power));

	    char bandwidth[MaxLen];
	    char length[MaxLen];
	    char velocity[MaxLen];
	    memset(&bandwidth,0,sizeof(bandwidth));
	    memset(&length,0,sizeof(length));
	    memset(&velocity,0,sizeof(velocity));		
        int index = 0,f = 0,j = 0;
	    while(data[index] != '\0'){
	    	if(data[index] == '#'){
	    		f++;
	    		j = 0;
	    	}else{
	    		if(f == 0)link_id[j] = data[index];
	    		else if(f == 1)size[j] = data[index];
	    		else if(f == 2)power[j] = data[index];
	    		else if(f == 3)bandwidth[j] = data[index];
	    		else if(f == 4)length[j] = data[index];
	    		else if(f == 5)velocity[j] = data[index];
	    		j++;
	    	}
	    	index++;
	    }

	    printf("The Server C received link information of link <%s>,file size <%s>,",link_id,size);
		printf("and signal power <%s>\n",power);


	    //calculate Tt
        float f_size = atof(size);
	    float f_bandwidth = atof(bandwidth);

	    float Tt = (f_size/f_bandwidth)/1000.0;


        //calculate Tp
	    float f_length = atof(length);
	    float f_velocity = atof(velocity);

	    float Tp = (f_length/f_velocity)/10.0;
	    float T = Tt + Tp;


        //convert to char*
	    char c_Tt[MaxLen];
	    sprintf(c_Tt,"%.3f",Tt);
	    char c_Tp[MaxLen];
	    sprintf(c_Tp,"%.3f",Tp);
	    char c_T[MaxLen];
	    sprintf(c_T,"%.3f",T);

	    char results[MaxLen];
	    memset(&results,0,sizeof(results));
	    strcat(results,c_Tt);
	    strcat(results,"#");
	    strcat(results,c_Tp);
	    strcat(results,"#");
	    strcat(results,c_T);
	    strcat(results,"#");

	    printf("The Server C finished the calcluation for link <%s>\n",link_id);

	    int sendtoAWS;
        if((sendtoAWS = sendto(socketC,results,MaxLen,0,(struct sockaddr *)&addr,addr_len)) == -1){
        	perror ("send to AWS");
        	exit(1);
        }
        printf("The Server C finished sending the output to AWS\n");

	}

	return 0;
}

