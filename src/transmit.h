#ifndef _TRANSMIT_H_
#define _TRANSMIT_H_

#include "list.h"

void t_transmit_init(List* s_List, List* r_List, int* socketfd, struct sockaddr_in* Remote, unsigned int* Remote_len);

void t_shutdown();

void t_free_memo();

void free_routine(void* pItem);

#endif 