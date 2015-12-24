/* I/O Multiplexing Library */
#ifndef __SELECT_POOL
#define __SELECT_POOL

#include "csapp.h"

typedef struct 	/* Represents a pool of connected descriptors. */
{
	int maxfd;			/* Largest descriptor in read_set */
	fd_set read_set;	/* Set of all active descriptors */
	fd_set ready_set;	/* Subset of descriptors ready for reading */
	int nready;			/* Number of ready descriptors from select */
	int maxi;			/* Highwater index into client array */
	int clientfd[FD_SETSIZE];	/* Set of active descriptors */
	rio_t clientrio[FD_SETSIZE];/* Set of active read buffers */
} pool;

int byte_cnt; /* Counts total bytes received by server */	

void init_pool(int listenfd, pool *p);
void add_client(int connfd, pool *p);
void check_clients(pool *p);


#endif