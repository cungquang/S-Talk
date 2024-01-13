#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "transmit.h"
#include "list.h"

/*--------------------------------------Socket------------------------------------------*/

struct sockaddr_in* s_Remote;
unsigned int Remote_len;
int Local_sock;
int status;

/*---------------------------------------Other------------------------------------------*/

struct addrinfo hints, *res, *p;
char buf[INET_ADDRSTRLEN];

/*------------------------------------Main Func-----------------------------------------*/

int main(int argc, char* argv[]){

	/*----------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------*/
	/*-----------------------------------Local Machine----------------------------------*/
	/*----------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------*/

	if(argc < 3){
		printf("ERROR, Not Enought Information \n");
		return 1;
	}


	/*-------------------------From Bjee's guide-------------------------*/

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	if((status = getaddrinfo(NULL,argv[1], &hints, &res)) != 0){
		printf("ERROR, unable to get address info\n");
		return 2;
	}

	for(p = res; p != NULL; p = p->ai_next){
		//Create the socket:
		if( (Local_sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1){
			printf("ERROR, socket\n");
			continue;
		}

		//bind the socket for UDP & check if binding is failure:
		if(bind(Local_sock, res->ai_addr, res->ai_addrlen) == -1){
			printf("ERROR, bind\n");
			continue;
		}

		break;
	}

	if(p == NULL){
		printf("ERROR, fail to bind the socket\n");
		return 2;
	}
	
	/*----------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------*/
	/*----------------------------------Remote Machine----------------------------------*/
	/*----------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------*/

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = 0;

	if((status = getaddrinfo(argv[2],argv[3], &hints, &res)) != 0){
		printf("ERROR, unable to get address info\n");
		return 2;
	}

	for(p = res; p != NULL; p = p->ai_next){
		void* addr;

		if(p->ai_family == AF_INET){

			s_Remote = (struct sockaddr_in *)p->ai_addr;
			Remote_len = p->ai_addrlen;
			addr = &(s_Remote->sin_addr);
		}

		inet_ntop(p->ai_family, addr, buf, sizeof buf);
    	printf("IP address: %s\n", buf);
		break;
	}

	if(p == NULL){
		printf("ERROR, socket fail");
	}

	/*-------------------------From Bjee's guide-------------------------*/


	/*----------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------*/
	/*---------------------------------Implementation-----------------------------------*/
	/*----------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------*/

	List* s_List = List_create();	//List for sending data
	List* r_List = List_create();	//List for receiving data
	
	printf("Enter '!' to end s-talk\n");

	//Execute s-talk:
	t_transmit_init(s_List, r_List, &Local_sock, s_Remote, &Remote_len);

	//free list:
	if(s_List != NULL){
		s_List = NULL;
		r_List = NULL;
	}
	//free the res pointer
	freeaddrinfo(res);
	//close the socket		
	close(Local_sock);

	return 0;
}