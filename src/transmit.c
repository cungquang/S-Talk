#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>

#include "transmit.h"
#include "list.h"

#define MSG_MAX_LEN 1024
#define MAX_N 100

static char terminate[] = "!\n";

/*--------------------------------Thread & Socket---------------------------------------*/

//Socket - Remote info:
static int* s_Descript;
static struct sockaddr_in* s_Remote;
static unsigned int* s_Remote_len;

//Struct thread:
static pthread_t keyboard_thread;	//receive thread
static pthread_t send_thread;		//screen thread
static pthread_t receive_thread;	//receive thread
static pthread_t screen_thread;		//screen thread

/*---------------------------------Sending Prep----------------------------------------*/

static int s_bytex;
static int s_size;

static char* k_buffer = NULL;		//keyboard buffer
static char* free_k = NULL;			//free r_buffer
static char* sen_buffer = NULL;		//send buffer for sending
static List* sen_List = NULL;		//list for sending

//condition var & mutex for sending:
static pthread_mutex_t get_Mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t get_Signal = PTHREAD_COND_INITIALIZER;

/*--------------------------------Receiving Prep---------------------------------------*/

static int r_bytex;
static int r_size;

static char* r_buffer = NULL;		//receive buffer
static char* free_r = NULL; 		//free r_buffer
static char* rec_buffer = NULL;		//receive buffer for screen
static List* rec_List = NULL;		//list for receiving

//condition var & mutex for receiving:
static pthread_mutex_t trans_Mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t trans_Signal = PTHREAD_COND_INITIALIZER;

/*-------------------------------Check Empty List--------------------------------------*/

static int ready_r = 1;		//check if receive list is ready
static int ready_s = 1;		//check if send list is ready

static pthread_mutex_t r_empty_Mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t r_empty_Signal = PTHREAD_COND_INITIALIZER;

static pthread_mutex_t s_empty_Mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_empty_Signal = PTHREAD_COND_INITIALIZER;

/*--------------------------------Keyboard Thread---------------------------------------*/

void* t_keyboard_message(void* unused){

	while(1){

		int j = 0;
		fflush(stdin);
		
		//allocate memory for buffer:
		k_buffer = malloc(sizeof(char)*MSG_MAX_LEN);
		memset(k_buffer, 0, MSG_MAX_LEN);

		s_bytex = (read(0,k_buffer,MSG_MAX_LEN))/sizeof(char);
		s_bytex = s_bytex < MAX_N ? s_bytex : MAX_N;


		//Terminate condition
		if(k_buffer[0] == '!' && k_buffer[1] == '\n'){
			sendto(*s_Descript, &terminate, sizeof(terminate)*2, 0,
			 (struct sockaddr*) s_Remote, *s_Remote_len);
			t_shutdown();
			return NULL;
		}

		//if the list is NOT EMPTY:
		if(ready_s == 0){
			pthread_mutex_lock(&s_empty_Mutex);
			{
				pthread_cond_wait(&s_empty_Signal, &s_empty_Mutex);
			}
			pthread_mutex_unlock(&s_empty_Mutex);	
		}
		//if the list is EMPTY:
		else{	
			pthread_mutex_lock(&get_Mutex);
			{
				while(j < s_bytex){
					List_add(sen_List, &k_buffer[j]);	
					j++;
				}
				ready_s = 0;
				pthread_cond_signal(&get_Signal);
			}
			pthread_mutex_unlock(&get_Mutex);
		}

	}

	return NULL;
}

/*----------------------------------Send Thread-----------------------------------------*/

void* t_send_message(void* unused){
	while(1){
		int e = 0;

		//Wait for signal to enter the list:
		pthread_mutex_lock(&get_Mutex);
		{
			pthread_cond_wait(&get_Signal, &get_Mutex);
			free_k = k_buffer;
			s_size = List_count(sen_List);		
			sen_buffer = malloc(sizeof(char)*s_size);

			Node* c = sen_List->pFirstNode;
			while(c != NULL){
				sen_buffer[e] = *(char*)c->pItem;
				c = c->pNext;
				e++;
			}
			
			sen_buffer[e] = '\0';
		}
		pthread_mutex_unlock(&get_Mutex);

		sendto(*s_Descript, sen_buffer, sizeof(sen_buffer)*s_size, 0,
			(struct sockaddr*) s_Remote, *s_Remote_len);

		//Empty the list before release:
		pthread_mutex_lock(&get_Mutex);
		{
			while(List_count(sen_List) > 0){
				List_trim(sen_List);
			}
			ready_s = 1;
			pthread_cond_signal(&get_Signal);
		}
		pthread_mutex_unlock(&get_Mutex);

		//Clear the s_buffer & k_buffer:
		if(sen_buffer != NULL){
			memset(sen_buffer, 0, s_size);
			free(sen_buffer);
			sen_buffer = NULL;	
		}
		
		if(free_k != NULL){
			memset(free_k, 0, s_size);
			free(free_k);
			free_k = NULL;	
		}
		
	}

	return NULL;
}

/*--------------------------------Receive Thread----------------------------------------*/

void* t_receive_message(void* unused){
	//Busy wait to collect the message from sender
	while(1){

		int j = 0;
		fflush(stdin);

		//Create a new receive buffer:
		r_buffer = malloc(sizeof(char)*MSG_MAX_LEN);
		if(r_buffer == NULL){
			return NULL;
		}
		memset(r_buffer, 0, MSG_MAX_LEN);

		//Receive the data from remote machine:
		r_bytex = recvfrom(*s_Descript, r_buffer, MSG_MAX_LEN, 0,
			(struct sockaddr*) s_Remote, s_Remote_len);

		//Terminate condition
		if(r_buffer[0] == '!' && r_buffer[1] == '\n'){
			t_shutdown();
			return NULL;
		}
		
		//Set the last element to 0:
		r_bytex = r_bytex < MSG_MAX_LEN? r_bytex : MSG_MAX_LEN;
		int t_index = r_bytex/sizeof(r_buffer);

		if(t_index < MAX_N){
			r_buffer[t_index] = 0;
		}else{
			t_index = MAX_N;
			r_buffer[t_index-1] = '\n';
			r_buffer[t_index] = 0;
		}

		//if the list is NOT EMPTY:
		if(ready_r == 0){
			pthread_mutex_lock(&r_empty_Mutex);
			{
				pthread_cond_wait(&r_empty_Signal, &r_empty_Mutex);
			}
			pthread_mutex_unlock(&r_empty_Mutex);	
		}
		//if the list is EMPTY:
		else{
			pthread_mutex_lock(&trans_Mutex);
			{	
				while(j < t_index){
					List_add(rec_List, &r_buffer[j]);
					j++;
				}
				
				ready_r = 0;
				pthread_cond_signal(&trans_Signal);
			}
			pthread_mutex_unlock(&trans_Mutex);	
		}
		
	}

	return NULL;
}


/*--------------------------------Screen Thread---------------------------------------*/

void* t_screen_message(void* unused){
	while(1){

		int j=0;	

		//Wait for the signal to accss the list:
		pthread_mutex_unlock(&trans_Mutex);
		{	
			pthread_cond_wait(&trans_Signal, &trans_Mutex);
			free_r = r_buffer;
			r_size = List_count(rec_List);			
			rec_buffer = malloc(sizeof(char)*r_size);
			
			//transfer dat to the buffer:
			Node* c = rec_List->pFirstNode;
			List_first(rec_List);
			while(c != NULL){
				rec_buffer[j] = *(char*)c->pItem;
				c = c->pNext;
				j++;
			}
		}
		pthread_mutex_unlock(&trans_Mutex);

		fflush(stdout);
		write(1, rec_buffer, r_size);

		//Clear the list before release:
		pthread_mutex_lock(&r_empty_Mutex);
		{
			while(List_count(rec_List) > 0){
				List_trim(rec_List);
			}
			ready_r = 1;

			pthread_cond_signal(&r_empty_Signal);
		}
		pthread_mutex_unlock(&r_empty_Mutex);

		//Clear the s_buffer:
		if(rec_buffer != NULL){
			memset(rec_buffer, 0, r_size);
			free(rec_buffer);
			rec_buffer = NULL;
		}

		//Clear the buffer for the next message
		if(free_r != NULL){
			memset(free_r, 0, MSG_MAX_LEN);
			free(free_r);
			free_r = NULL;
		}
	}
	return NULL;
}

/*-----------------------------Shutdown Implementation----------------------------------*/

void t_shutdown(){
	pthread_cancel(keyboard_thread);
	pthread_cancel(send_thread);
	pthread_cancel(receive_thread);
	pthread_cancel(screen_thread);
	return;
}

void t_free_memo(){
	//clear allocating memory:
	if(r_buffer != NULL){
		memset(r_buffer, 0, MSG_MAX_LEN);
		free(r_buffer);
		r_buffer = NULL;
	}

	if(k_buffer != NULL){
		memset(k_buffer, 0, MSG_MAX_LEN);
		free(k_buffer);
		k_buffer = NULL;
	}
	if(free_r != NULL){
		memset(free_r, 0, MSG_MAX_LEN);
		free(free_r);
		free_r = NULL;
	}

	if(free_k != NULL){
		memset(free_k, 0, MSG_MAX_LEN);
		free(free_k);
		free_k = NULL;
	}

	if(sen_buffer != NULL){
		memset(sen_buffer, 0, s_size);
		free(sen_buffer);
		sen_buffer = NULL;
	}

	if(rec_buffer != NULL){
		memset(rec_buffer, 0, s_size);
		free(rec_buffer);
		rec_buffer = NULL;
	}

	if(List_count(sen_List) > 0){
		List_free(sen_List,&free_routine);
	}

	if(List_count(rec_List) > 0){
		List_free(rec_List,&free_routine);
	}

	pthread_mutex_destroy(&trans_Mutex);
	pthread_mutex_destroy(&get_Mutex);
	pthread_mutex_destroy(&r_empty_Mutex);
	pthread_mutex_destroy(&s_empty_Mutex);

	pthread_cond_destroy(&trans_Signal);
	pthread_cond_destroy(&get_Signal);
	pthread_cond_destroy(&r_empty_Signal);
	pthread_cond_destroy(&s_empty_Signal);
	
	memset(&k_buffer, 0, MSG_MAX_LEN);
	return;
}

void free_routine(void* pItem){
	List* f_List = (List*) pItem; 
	while(List_count(f_List) > 0){
		List_trim(f_List);
	}
	return;
}

/*-------------------------------Thread Management--------------------------------------*/

void t_transmit_init(List* s_List, List* r_List, int* socketfd, struct sockaddr_in* Remote, unsigned int* Remote_len){
	sen_List = s_List;			//pointer to sending List
	rec_List = r_List;			//pointer to receiving List
	s_Descript = socketfd;		//pointer to integer
	s_Remote = Remote;			//pointer to struct
	s_Remote_len = Remote_len;	//pointer to unsigned integer

	//Create thread:
	pthread_create(&keyboard_thread, NULL, t_keyboard_message, NULL);
	pthread_create(&send_thread, NULL, t_send_message, NULL);
	pthread_create(&receive_thread, NULL, t_receive_message, NULL);
	pthread_create(&screen_thread, NULL, t_screen_message, NULL);

	//Wait for execute:
	pthread_join(keyboard_thread, NULL);
	pthread_join(send_thread, NULL);
	pthread_join(receive_thread, NULL);
	pthread_join(screen_thread, NULL);

	t_free_memo();
	return;
}
