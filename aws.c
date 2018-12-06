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

#define PortA "21708"
#define PortB "22708"
#define PortC "23708"
#define PortUDPClient "24708"
#define PortClient "25708"
#define PortMonitor "26708"
#define MaxLen 256
#define Host "localhost"
#define BACKLOG 20
#define NUL "nomatch"

struct addrinfo *pA,*pB,*pC; 
int UDPConnection(char X){
	struct addrinfo hints,*s,*p;
	memset(&hints,0,sizeof(hints));
	int socketX;

	char *PortX;

    if(X == 'A')PortX = PortA;
    else if(X == 'B')PortX = PortB;
    else PortX = PortC;

    hints.ai_socktype = SOCK_DGRAM;

	if(getaddrinfo(Host,PortX,&hints,&s) != 0){
		return 1;
	}

	for(p = s;p != NULL;p = p->ai_next){
		// printf("%d,%d,%d\n",p->ai_family,p->ai_socktype,p->ai_protocol);
		//reused codes
		if(p->ai_socktype != SOCK_DGRAM){
			continue;
		}
		
		if((socketX = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1){
			continue;
		}

		break;
	}
	

	if(p == NULL){
		printf("Server %c: failed to bind socket\n",X);
		return 1;
	}

	if(X == 'A')pA = p;
    else if(X == 'B')pB = p;
    else pC = p;

	return socketX;
}
int setUpTCP(char* PortNo){

	struct addrinfo hints,*s,*p;
	memset(&hints,0,sizeof(hints));
	struct sockaddr_in addr;
	int addr_len = sizeof(struct sockaddr_in);
	int socketClient;
	int reuseAddr = 1;

	int result = getaddrinfo(Host,PortNo,&hints,&s);
	if(result != 0){
		return 1;
	}
	//reused codes

	for(p = s;p != NULL;p = p->ai_next){

		socketClient = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
		if(socketClient == -1){
			perror ("aws socket:");
			continue;
		}

		if(setsockopt(socketClient,SOL_SOCKET,SO_REUSEADDR,(const char *)&reuseAddr,sizeof(int)) == -1){
			perror ("setsockopt");
			continue;
		}

		if(bind(socketClient,p->ai_addr,p->ai_addrlen) == -1){
			close(socketClient);
			perror("bind");
			continue;
		}
		// printf("%d : ",socketClient);
		// printf("%d,%d,%d\n",p->ai_family,p->ai_socktype,p->ai_protocol);
		break;
	}
	
	if(p == NULL){
		perror ("AWS: failed to bind socket\n");
		return 1;
	}
	freeaddrinfo(s);
	if(listen(socketClient,BACKLOG) == -1){
		perror ("listen");
		exit(1);
	}
	return socketClient;

}

int main(){
	struct sockaddr_in addr;
	int addr_len = sizeof(struct sockaddr_in);

	int socketClient = setUpTCP(PortClient);
	int socketMonitor = setUpTCP(PortMonitor);

	char data[MaxLen];

	printf ("The AWS is up and running\n");

	int socketServerA = UDPConnection('A');
    int socketServerB = UDPConnection('B');
    int socketServerC = UDPConnection('C');

    int accept_from_monitor = accept(socketMonitor,(struct sockaddr *)&addr,&addr_len);
	// printf("accept_from_monitor = %d\n",accept_from_monitor);

	if(accept_from_monitor == -1){
		perror ("error of accept monitor in AWS");
		exit(1);
	}

	while(1){

		int accept_from_client = accept(socketClient,(struct sockaddr *)&addr,&addr_len);
		// printf("accept_from_client =%d\n",accept_from_client);

		if(accept_from_client == -1){
			perror ("error of accept client in AWS");
			exit(1);
		}

		memset(&data,0,sizeof(data));

		int data_len = recv(accept_from_client,data,sizeof(data),0);

		if(data_len == -1){
			perror ("error of recv in AWS");
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


	    // printf("link_id = %s\n",link_id);
	    // printf("size = %s\n",size);
	    // printf("power = %s\n",power);
	    printf("The AWS received link ID=<%s>,size=<%s>,and power=<%s> ",link_id,size,power);
	    printf("from the client using TCP over port <%s>\n",PortClient);



        //send data back to monitor
        //-------------------------
        char data_to_mointor[MaxLen];
	    memset(&data_to_mointor,0,sizeof(data_to_mointor));
	    strcat(data_to_mointor,"Q");
	    strcat(data_to_mointor,data);

	    int send_data_len_monitor = send(accept_from_monitor,data_to_mointor,MaxLen,0);
	    if(send_data_len_monitor == -1){
	    	perror ("send back to monitor");
	        exit(1);
	    }
	    printf("The AWS sent link ID=<%s>,size=<%s>,and power=<%s> ",link_id,size,power);
	    printf("to the monitor using TCP over port <%s>\n",PortMonitor);
        

        //send to server A
        //----------------
        int sendtoA,sendtoB,sendtoC;
        if((sendtoA = sendto(socketServerA,data,sizeof(data),0,pA->ai_addr,pA->ai_addrlen)) == -1){
        	perror ("send to A");
        	exit(1);
        }
        printf("The AWS sent link ID=<%s> to Backend-Server A ",link_id);
	    printf("using UDP over port <%s>\n",PortUDPClient);
        

        //send to server B
        //----------------
        if((sendtoB = sendto(socketServerB,data,sizeof(data),0,pB->ai_addr,pB->ai_addrlen)) == -1){
        	perror ("send to B");
        	exit(1);
        }
        printf("The AWS sent link ID=<%s> to Backend-Server B ",link_id);
	    printf("using UDP over port <%s>\n",PortUDPClient);


        //receive from server A
        //---------------------
	    char datafromA[MaxLen],datafromB[MaxLen];
		memset(&datafromA,0,sizeof(datafromA));
		memset(&datafromB,0,sizeof(datafromB));

		if((recvfrom(socketServerA,datafromA,sizeof(datafromA),0,(struct sockaddr *)&addr, &addr_len)) == -1){
			perror ("recvfrom A");
			exit(1);
		}
 
		if(strcmp(datafromA,"0") == 0){
			printf ("The AWS received <0> matches from Backend-Server <A> ");
		}else{
		    printf ("The AWS received <1> matches from Backend-Server <A> ");
		}
		printf("using UDP over port <%s>\n",PortUDPClient);



        //receive from server B
        //---------------------
		if((recvfrom(socketServerB,datafromB,sizeof(datafromB),0,(struct sockaddr *)&addr, &addr_len)) == -1){
			perror ("recvfrom B");
			exit(1);
		}

		if(strcmp(datafromB,"0") == 0){
			printf ("The AWS received <0> matches from Backend-Server <B> ");
		}else{
		    printf ("The AWS received <1> matches from Backend-Server <B> ");
		}
		printf("using UDP over port <%s>\n",PortUDPClient);



		//send to server C or not
		//-----------------------
		if(strcmp(datafromA,"0") == 0 && strcmp(datafromB,"0") == 0){

			// char *send_data = "nomatch";
			char data_to_mointor[MaxLen];
	        memset(&data_to_mointor,0,sizeof(data_to_mointor));
	        strcat(data_to_mointor,"R");
	        strcat(data_to_mointor,NUL);

			int send_data_len1 = send(accept_from_client,NUL,MaxLen,0);
			int send_data_len2 = send(accept_from_monitor,data_to_mointor,MaxLen,0);

	        if(send_data_len1 == -1 || send_data_len2 == -1){
	        	perror ("send");
	        	exit(1);
	        }
			printf("The AWS sent No Match to the monitor and the client using TCP over ports<%s> and <%s>,respectively\n",PortClient,PortMonitor);
		
		}else{

			char search_result_data[MaxLen];
	        memset(&search_result_data,0,sizeof(search_result_data));

	        if(strcmp(datafromA,"0") == 0){
	        	strcpy(search_result_data,datafromB);
	        }else{
	        	strcpy(search_result_data,datafromA);
	        }

	        char bandwidth[MaxLen];
	        char length[MaxLen];
	        char velocity[MaxLen];

	        memset(&bandwidth,0,sizeof(bandwidth));
	        memset(&length,0,sizeof(length));
	        memset(&velocity,0,sizeof(velocity));

	        int index = 0,f = 0,j = 0;
	        while(search_result_data[index] != '\0'){
	        	if(search_result_data[index] == ','){
	        		f++;
	    		    j = 0;
	    	    }else{
	    		    if(f == 1)bandwidth[j] = search_result_data[index];
	    		    else if(f == 2)length[j] = search_result_data[index];
	    		    else if(f == 3)velocity[j] = search_result_data[index];
	    		    j++;
	    	    }
	    	    index++;
	        }

	        // printf("bandwidth = %s\n",bandwidth);
	        // printf("length = %s\n",length);
	        // printf("velocity = %s\n",velocity);

	        char send_data_C[MaxLen];
	        memset(&send_data_C,0,sizeof(send_data_C));

	        strcat(send_data_C,link_id);
	        strcat(send_data_C,"#");
	        strcat(send_data_C,size);
	        strcat(send_data_C,"#");
	        strcat(send_data_C,power);
	        strcat(send_data_C,"#");
	        strcat(send_data_C,bandwidth);
	        strcat(send_data_C,"#");
	        strcat(send_data_C,length);
	        strcat(send_data_C,"#");
	        strcat(send_data_C,velocity);
	        strcat(send_data_C,"#");



	        //send to server C
	        //----------------
            if((sendtoC = sendto(socketServerC,send_data_C,MaxLen,0,pC->ai_addr,pC->ai_addrlen)) == -1){
        	    perror ("send to C");
        	    exit(1);
            }
            printf("The AWS sent link ID=<%s>,size=<%s>,power=<%s> to Backend-Server C ",link_id,size,power);
	        printf("using UDP over port <%s>\n",PortUDPClient);

	        char resultsFromC[MaxLen];
	        memset(&resultsFromC,0,sizeof(resultsFromC));



	        //receive from server C
	        //---------------------
		    if((recvfrom(socketServerC,resultsFromC,MaxLen,0,(struct sockaddr *)&addr, &addr_len)) == -1){
			    perror ("recvfrom C");
			    exit(1);
		    }

            printf ("The AWS received outputs from Backend-Server C");
		    printf("using UDP over port <%s>\n",PortUDPClient);

	        // printf("resultsFromC =%s\n",resultsFromC);


	        char c_T[MaxLen];
	        memset(&c_T,0,sizeof(c_T));

	        index = 0,f = 0,j = 0;
	        while(resultsFromC[index] != '\0'){
	            if(resultsFromC[index] == '#'){
	    	        f++;
	    	        j = 0;
	            }else{
	    	        if(f == 2)c_T[j] = resultsFromC[index];
	    	        j++;
	            }
	            index++;
	        }


	        //send to client and monitor
	        //--------------------------
	        char data_to_mointor[MaxLen];
	        memset(&data_to_mointor,0,sizeof(data_to_mointor));

	        strcat(data_to_mointor,"R");
	        strcat(data_to_mointor,resultsFromC);

	        int send_data_len1 = send(accept_from_client,c_T,MaxLen,0);
	        int send_data_len2 = send(accept_from_monitor,data_to_mointor,MaxLen,0);

	        if(send_data_len1 == -1 || send_data_len2 == -1){
	        	perror ("send");
	        	exit(1);
	        }
	        printf("The AWS sent delay=<%s>ms to the client using TCP over port<%s>\n",c_T,PortClient);
	        printf("The AWS sent detailed results to the monitor using TCP over port<%s>\n",PortMonitor);

		}
		printf ("\n");

		 //send to server C
     //    if((sendtoC = sendto(socketServerC,data,sizeof(data),0,pC->ai_addr,pC->ai_addrlen)) == -1){
     //    	perror ("send to C");
     //    	exit(1);
     //    }
     //    printf("The AWS sent link ID=<%s>,size=<%s>,power=<%s> to Backend-Server C ",link_id,size,power);
	    // printf("using UDP over port <%s>\n",PortUDPClient);

	    //receive from server C
		// if((recvfrom(socketServerC,datafromC,sizeof(datafromC),0,(struct sockaddr *)&addr, &addr_len)) == -1){
		// 	perror ("recvfrom C");
		// 	exit(1);
		// }
		// printf ("The AWS received outputs from Backend-Server C  ",datafromB);
		// printf("using UDP over port <%s>\n",PortUDPClient);

		// char send_data[MaxLen];
	 //    memset(&send_data,0,sizeof(send_data));
	 //    strcat(send_data,datafromA);
	 //    strcat(send_data,"#");
	 //    strcat(send_data,datafromB);
	 //    strcat(send_data,"#");

	    // if(strcmp(datafromA,"1") == 0 && strcmp(datafromB,"1") == 0){
	    // 	printf("%s\n", );

	//     printf("send_data =%s\n",send_data);

	//     int send_data_len = send(accept_from_client,send_data,sizeof(send_data),0);

	//     // if(send_data_len == -1){
	//     // 	printf("error send_data, %s\n",send_data);
	// 	   //  perror ("send");
	// 	   //  exit(1);
	//     // }
	//     printf("The AWS sent delay=<%s>ms to the client using TCP over port<%s>\n",send_data,PortClient);
	// // printf("send_data_len = %d \n",send_data_len);




	}



	return 0;
}
