#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */

typedef enum {false=0, true=1} bool;
enum {battery, aircraft, propeller};
char name[3][10] = {"battery", "aircraft", "propeller"};

int compare(const void *arg1, const void *arg2)
{
	return  (*(int *)arg2 - *(int *)arg1);
}

typedef struct _S_Table
{
	int        quantity;
	int 	   count;
	int		   generate[3][2];
	bool       dispatch;
	bool       component[3];
	pthread_mutex_t lock;
} S_Table, *P_Table;

typedef struct _S_Producer
{
	int        number;
	P_Table    ST;
} S_Producer, *P_Producer;

void producer( P_Producer ptr );
void dispatcher( P_Table ptr );

int main(int argc, char** argv)
{
	srand(time(NULL));
	printf("> prog3\n");
	
	pthread_t proThread1, proThread2, proThread3, disThread;
    P_Table pT;
    P_Producer pPRO[3];
    
    pT = malloc(sizeof( S_Table ));
	memset(pT, 0, sizeof(S_Table));
    for(int i = 0; i < 3; i++)
    {
		pPRO[i] = malloc(sizeof( S_Producer ));
		pPRO[i]->number = i;
		pPRO[i]->ST = pT;
	}
	pT->generate[0][1] = 'b';
	pT->generate[1][1] = 'a';
	pT->generate[2][1] = 'p';

	pthread_mutex_init(&pT->lock, NULL);
    pthread_create (&proThread1, NULL, (void *) &producer, pPRO[0]);
    pthread_create (&proThread2, NULL, (void *) &producer, pPRO[1]);
    pthread_create (&proThread3, NULL, (void *) &producer, pPRO[2]);
    pthread_create (&disThread, NULL, (void *) &dispatcher, pT);
		
    pthread_join(proThread1, NULL);
    pthread_join(proThread2, NULL);
    pthread_join(proThread3, NULL);
    pthread_join(disThread, NULL); 
    qsort((void *)pT->generate,3, sizeof(*pT->generate), compare);
    printf("\n"); 
    for(int i = 0; i < 3; i++)
    {
		if(pT->generate[i][1] == 'b')
			printf("Producer (battery): %d\n", pT->generate[i][0]);
		else if(pT->generate[i][1] == 'a')
			printf("Producer (aircraft): %d\n", pT->generate[i][0]);
		else if(pT->generate[i][1] == 'p')
			printf("Producer (propeller): %d\n", pT->generate[i][0]);
	}
	
}

void producer( P_Producer ptr )
{
	while(!ptr->ST->dispatch);
	while(ptr->ST->quantity != 50)
	{
		while(!ptr->ST->dispatch && ptr->ST->quantity != 50);
		pthread_mutex_lock(&ptr->ST->lock);
		bool check = 1;
		if(ptr->ST->count >= 2)
		{
			if(ptr->number == battery && ptr->ST->component[1] && ptr->ST->component[2])
			{
				ptr->ST->component[1] = ptr->ST->component[2] = 0;	
			}
			else if(ptr->number == aircraft && ptr->ST->component[0] && ptr->ST->component[2])
			{
				ptr->ST->component[0] = ptr->ST->component[2] = 0;
			}
			else if(ptr->number == propeller && ptr->ST->component[0] && ptr->ST->component[1])
			{
				ptr->ST->component[0] = ptr->ST->component[1] = 0;
			}
			else
				check = 0;
				
			if(check)
			{
				printf("Producer (%s): OK, %d drone(s) \n", name[ptr->number], ++ptr->ST->quantity);
				ptr->ST->generate[ptr->number][0]++;
				ptr->ST->count -= 2;
				ptr->ST->dispatch = 0;
			}			
		}
		pthread_mutex_unlock(&ptr->ST->lock);
	}
}

void dispatcher( P_Table ptr )
{
	while(true)
	{
		if(ptr->count == 2)
			ptr->dispatch = 1;
		while(ptr->dispatch && ptr->quantity != 50);
		if(ptr->quantity == 50)
			break;
		pthread_mutex_lock(&ptr->lock);
		int random = rand() % 3;
		while(ptr->component[random] == 1)
			random = rand() % 3;
		ptr->component[random] = 1;
		printf("Dispatcher : %s\n", name[random]);
		ptr->count++;
		pthread_mutex_unlock(&ptr->lock);
	}
}
