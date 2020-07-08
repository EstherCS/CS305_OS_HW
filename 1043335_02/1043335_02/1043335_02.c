#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */ 
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */
#include <time.h>
#include <sys/syscall.h> 

#define SWAP(x,y) {int t; t = x; x = y; y = t;} 
#define length 10000

int compare(const void *arg1, const void *arg2)
{
	return  (*(int *)arg1 - *(int *)arg2);
}

void quickSort(int data[], int left, int right);
void bubbleSort(int data[], int first, int last);
void mergeSort(int data[], int left, int right, int temp[], int c);
void load(int read[], char* name);
void store(int data[], int i);

pid_t gettid(void)
{
    return syscall(SYS_gettid);
}

typedef struct _S_GLOBAL
{
	int       Ready;
	int       sleepTime;
	int       Value[length];
	int       time[5][8];
	int       tid[5];
} S_GLOBAL, *P_GLOBAL;

typedef struct _S_THREAD
{
	int        ID;
	int        MyValue[length];
	P_GLOBAL   SG;
} S_THREAD, *P_THREAD;

typedef struct _S_ADV
{
	int        TID;
	int        CID;
	int        HalfData[length];
	P_THREAD   ST;
} S_ADV, *P_ADV;

void work_thread( P_THREAD ptr );
void advM(P_THREAD ptr);
void advM_work(P_ADV Mptr);
void advQ(P_THREAD ptr);
void advQ_work(P_ADV Mptr);

time_t timep; 
struct tm *p; 

int main(int argc, char** argv)
{
	printf("> prog2\n");
	
	pthread_t KidThread1, KidThread2, KidThread3, advMerge, advQuick;
    P_GLOBAL pG;
    P_THREAD pT[5];
    
    pG = malloc(sizeof(S_GLOBAL));
    pG->sleepTime = atoi(argv[2]);
    for(int i = 0; i < 5; i++)
    {
		pT[i] = malloc(sizeof( S_THREAD ));
		pT[i]->ID = i+1;
		pT[i]->SG = pG;
		pT[i]->SG->tid[i] = 0;
	}

    pthread_create (&KidThread1, NULL, (void *) &work_thread, pT[0]);
    while(pG->tid[0] == 0);
    printf("[Main thread]: work thread %d for bubblesort\n", pG->tid[0]);
    pthread_create (&KidThread2, NULL, (void *) &work_thread, pT[1]);
    while(pG->tid[1] == 0);
    printf("[Main thread]: work thread %d for quicksort\n", pG->tid[1]);
    pthread_create (&KidThread3, NULL, (void *) &work_thread, pT[2]);
    while(pG->tid[2] == 0);
    printf("[Main thread]: work thread %d for mergesort\n", pG->tid[2]);
    pthread_create (&advMerge, NULL, (void *) &advM, pT[3]);
    while(pG->tid[3] == 0);
    printf("[Main thread]: work thread %d for advMerge\n", pG->tid[3]);
    pthread_create (&advQuick, NULL, (void *) &advQ, pT[4]);
    while(pG->tid[4] == 0);
    printf("[Main thread]: work thread %d for advQuick\n", pG->tid[4]);
    sleep(1);
    pG->Ready = 16;
	sleep(2);
	printf("[Main thread]: read data \n");
	printf("[Main thread]: write data to the shared area  \n");
	load(pG->Value, argv[1]);
	printf("[Main thread]: tell work thread start to work \n");
	pG->Ready = 1;
		
    pthread_join(KidThread1, NULL);
    pthread_join(KidThread2, NULL);
    pthread_join(KidThread3, NULL);
    pthread_join(advMerge, NULL);
    pthread_join(advQuick, NULL);
    
    printf("[Main thread]: the execution time rank \n");
    qsort((void *)pG->time,5, sizeof(*pG->time), compare);	
    int check[6];
    for(int i = 0; i < 5; i++)
    {
		if(pG->time[i][1] == 'q')
		{
			check[0] = pG->time[i][0];
			check[1] = pG->time[i][7];
			check[2] = i;
		}
		else if(pG->time[i][1] == 'm')
		{
			check[3] = pG->time[i][0];
			check[4] = pG->time[i][7];
			check[5] = i;
		}		
	}
	if(check[0] == check[3])
	{
		if(check[1] >= check[4])
		{
			pG->time[check[2]][1] = 'm';
			pG->time[check[5]][1] = 'q';
		}
	}
    for(int i = 0; i < 5; i++)
	{
		if(pG->time[i][1] == 'b')
			printf("回傳時間為 : [%d.%d.%d %d:%d:%d], bubblesort: %d ms\n", pG->time[i][2], pG->time[i][3], pG->time[i][4], pG->time[i][5], pG->time[i][6], pG->time[i][7], pG->time[i][0]);
		else if(pG->time[i][1] == 'm')
			printf("回傳時間為 : [%d.%d.%d %d:%d:%d], mergesort: %d ms\n", pG->time[i][2], pG->time[i][3], pG->time[i][4], pG->time[i][5], pG->time[i][6], pG->time[i][7], pG->time[i][0]);
		else if(pG->time[i][1] == 'q')
			printf("回傳時間為 : [%d.%d.%d %d:%d:%d], quicksort: %d ms\n", pG->time[i][2], pG->time[i][3], pG->time[i][4], pG->time[i][5], pG->time[i][6], pG->time[i][7], pG->time[i][0]);
		else if(pG->time[i][1] == 'M')
			printf("回傳時間為 : [%d.%d.%d %d:%d:%d], advMergesort: %d ms\n", pG->time[i][2], pG->time[i][3], pG->time[i][4], pG->time[i][5], pG->time[i][6], pG->time[i][7], pG->time[i][0]);
		else if(pG->time[i][1] == 'Q')
			printf("回傳時間為 : [%d.%d.%d %d:%d:%d], advQuicksort: %d ms\n", pG->time[i][2], pG->time[i][3], pG->time[i][4], pG->time[i][5], pG->time[i][6], pG->time[i][7], pG->time[i][0]);
	}
    exit(0);
}
clock_t timeStartM, timeEndM;
int enterM = 0;
void advM(P_THREAD ptr)
{
	int Mtemp[length] = {};
	ptr->SG->tid[3] = gettid();
	pthread_t advm1, advm2;
	P_ADV M[2];
	for(int i = 0; i < 2; i++)
    {
		M[i] = malloc(sizeof( S_ADV ));
		M[i]->CID = i+1;
		M[i]->ST = ptr;
		M[i]->TID = 0;
	}
	pthread_create (&advm1, NULL, (void *) &advM_work, M[0]);
	while(M[0]->TID == 0);
	printf("[Tid = %d]: work thread  %d for child_advmerge_1\n",gettid(),M[0]->TID);
    sleep(0);
    pthread_create (&advm2, NULL, (void *) &advM_work, M[1]);
    while(M[1]->TID == 0);
	printf("[Tid = %d]: work thread  %d for child_advmerge_2\n",gettid(),M[1]->TID);
	while(ptr->SG->Ready != 16);
	sleep(0);
	while(ptr->SG->Ready != 1)
	{		
		time(&timep); 
		p = localtime(&timep); 
		printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] sleep %d second\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, ptr->SG->sleepTime);
		sleep(ptr->SG->sleepTime);
	}
	pthread_join(advm1, NULL);
	pthread_join(advm2, NULL);
	printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] parent_advmerge begins.\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	mergeSort(ptr->MyValue, 0, length - 1, Mtemp, 0);
	timeEndM = clock();
	printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] parent_advmerge ends.\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] advMergesort %d ms.\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, ((int)timeEndM - timeStartM) * 1000 / CLOCKS_PER_SEC);
	printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] advMergesort results written to file advmerge.txt .\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	ptr->SG->time[3][2] = (1900+p->tm_year);
	ptr->SG->time[3][3] = (1+p->tm_mon);
	ptr->SG->time[3][4] = p->tm_mday;
	ptr->SG->time[3][5] = p->tm_hour;
	ptr->SG->time[3][6] = p->tm_min;
	ptr->SG->time[3][7] = p->tm_sec;
	ptr->SG->time[3][0] = ((int)timeEndM - timeStartM) * 1000 / CLOCKS_PER_SEC;
	ptr->SG->time[3][1] = 'M';
	store(ptr->MyValue, ptr->ID);
}

void advM_work(P_ADV Mptr)
{
	Mptr->TID = gettid();
	int Mtemp[length] = {};
	char *sort = "";
	if (Mptr->CID == 1)
		sort = "child_advmerge_1";
	else
		sort = "child_advmerge_2";
	while(Mptr->ST->SG->Ready != 16);
	sleep(0);
	while(Mptr->ST->SG->Ready != 1)
	{		
		time(&timep); 
		p = localtime(&timep); 
		printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] sleep %d second\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, Mptr->ST->SG->sleepTime);
		sleep(Mptr->ST->SG->sleepTime);
	}
	if(!enterM)
	{
		timeStartM = clock(); 
		enterM = 1;
	}
	if(Mptr->CID == 1)
	{
		for(int i = 0; i < length/2; i++)
		{
			Mptr->HalfData[i] = Mptr->ST->SG->Value[i];
		}
	}
	else
	{
		int d = 0;
		for(int i = length/2; i < length; i++)
		{
			Mptr->HalfData[d++] = Mptr->ST->SG->Value[i];
		}
	}
	time(&timep); 
	p = localtime(&timep);
	printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] %s begins\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, sort);
	mergeSort(Mptr->HalfData, 0, length / 2 - 1, Mtemp, 1);
	printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] %s ends\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, sort);
	if(Mptr->CID == 1)
	{
		for(int i = 0; i < length/2; i++)
		{
			Mptr->ST->MyValue[i] = Mptr->HalfData[i];
		}
	}
	else
	{
		int d = length/2;
		for(int i = 0; i < length/2; i++)
		{
			Mptr->ST->MyValue[d++] =  Mptr->HalfData[i];
		}
	}
}

clock_t timeStartQ, timeEndQ;
int enterQ = 0;
void advQ(P_THREAD ptr)
{
	ptr->SG->tid[4] = gettid();
	pthread_t advq1, advq2;
	P_ADV Q[2];
	for(int i = 0; i < 2; i++)
    {
		Q[i] = malloc(sizeof( S_ADV ));
		Q[i]->CID = i+1;
		Q[i]->ST = ptr;
		Q[i]->TID = 0;
	}
	pthread_create (&advq1, NULL, (void *) &advQ_work, Q[0]);
	while(Q[0]->TID == 0);
	printf("[Tid = %d]: work thread  %d for child_advquick_1\n",gettid(),Q[0]->TID);
    sleep(0);
    pthread_create (&advq2, NULL, (void *) &advQ_work, Q[1]);
    while(Q[1]->TID == 0);
	printf("[Tid = %d]: work thread  %d for child_advquick_2\n",gettid(),Q[1]->TID);
	while(ptr->SG->Ready != 16);
	sleep(0);
	while(ptr->SG->Ready != 1)
	{		
		time(&timep); 
		p = localtime(&timep);  
		printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] sleep %d second\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, ptr->SG->sleepTime);
		sleep(ptr->SG->sleepTime);
	}
	pthread_join(advq1, NULL);
	pthread_join(advq2, NULL);
	timeEndQ = clock();
	printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] advQuicksort %d ms.\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, ((int)timeEndQ - timeStartQ) * 1000 / CLOCKS_PER_SEC);
	printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] advQuicksort results written to file advquick.txt .\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	ptr->SG->time[4][2] = (1900+p->tm_year);
	ptr->SG->time[4][3] = (1+p->tm_mon);
	ptr->SG->time[4][4] = p->tm_mday;
	ptr->SG->time[4][5] = p->tm_hour;
	ptr->SG->time[4][6] = p->tm_min;
	ptr->SG->time[4][7] = p->tm_sec;
	ptr->SG->time[4][0] = ((int)timeEndQ - timeStartQ) * 1000 / CLOCKS_PER_SEC;
	ptr->SG->time[4][1] = 'Q';
	store(ptr->MyValue, ptr->ID);
}

void advQ_work(P_ADV Qptr)
{
	Qptr->TID = gettid();
	int Qtemp[length] = {};
	char *sort = "";
	if (Qptr->CID == 1)
		sort = "child_advquick_1";
	else
		sort = "child_advquick_2";
	while(Qptr->ST->SG->Ready != 16);
	sleep(0);
	while(Qptr->ST->SG->Ready != 1)
	{		
		time(&timep); 
		p = localtime(&timep); 
		printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] sleep %d second\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, Qptr->ST->SG->sleepTime);
		sleep(Qptr->ST->SG->sleepTime);
	}
	if(!enterQ)
	{
		timeStartQ = clock(); 
		enterQ = 1;
	}
	for (int i = 0; i < length; i++)
	{
		Qtemp[i] = Qptr->ST->SG->Value[i];
	}
	int m = Qtemp[(0 + length - 1) / 2];
	int a = -1, b = length;
	while (1)
	{
		while (Qtemp[++a] < m);                        // 往右邊找 
		while (Qtemp[--b] > m);                        // 往左邊找 
		if (a >= b)
			break;
		SWAP(Qtemp[a], Qtemp[b]);
	}
	int end = 0;
	if(Qptr->CID == 1)
	{
		for(int i = 0; i < a; i++, end++)
		{
			Qptr->HalfData[end] = Qtemp[i];
		}
	}
	else
	{
		for(int i = b+1; i < length; i++, end++)
		{
			Qptr->HalfData[end] = Qtemp[i];
		}
	}
	time(&timep); 
	p = localtime(&timep); 
	printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] %s begins\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, sort);
	quickSort(Qptr->HalfData, 0, end - 1);
	printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] %s ends\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, sort);
	if(Qptr->CID == 1)
	{
		for (int i = 0; i < a; i++)
		{
			Qptr->ST->MyValue[i] = Qptr->HalfData[i];     // 把數字放 shared memory			
		}
	}
	else
	{
		int d = b + 1;
		for(int i = 0; i < end; d++, i++)
		{
			Qptr->ST->MyValue[d] =  Qptr->HalfData[i];
		}
	}
}

void work_thread( P_THREAD ptr )
{
	ptr->SG->tid[ptr->ID - 1] = gettid();
	clock_t timeStart, timeEnd;
	int temp[length] = {};
	char *sort = "";
	if (ptr->ID == 1)
		sort = "bubblesort";
	else if (ptr->ID == 2)
		sort = "quicksort";
	else 
		sort = "mergesort";	
	//printf("[Main thread]: work thread %d for %s\n", gettid(), sort);
	while(ptr->SG->Ready != 16);
	sleep(0);
	while(ptr->SG->Ready != 1)
	{		
		time(&timep); 
		p = localtime(&timep); /*取得當地時間*/ 
		printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] sleep %d second\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, ptr->SG->sleepTime);
		sleep(ptr->SG->sleepTime);
	}
	for(int i = 0; i < length; i++)
	{
		ptr->MyValue[i] = ptr->SG->Value[i];
	}
	time(&timep); 
	p = localtime(&timep); /*取得當地時間*/ 
	printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] %s begins\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, sort);
	timeStart = clock(); 
	if(ptr->ID == 1)
		bubbleSort(ptr->MyValue, 0, length - 1);
	else if(ptr->ID == 2)
		quickSort(ptr->MyValue, 0, length - 1);
	else 
		mergeSort(ptr->MyValue, 0, length - 1, temp, 1);
	timeEnd = clock(); 
	time(&timep); 
	p = localtime(&timep); /*取得當地時間*/ 
	printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] %s ends\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, sort);
	printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] %s %d ms.\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, sort, ((int)timeEnd - timeStart) * 1000 / CLOCKS_PER_SEC);
	printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] %s results written to file %s.txt .\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, sort, sort);
	ptr->SG->time[ptr->ID-1][2] = (1900+p->tm_year);
	ptr->SG->time[ptr->ID-1][3] = (1+p->tm_mon);
	ptr->SG->time[ptr->ID-1][4] = p->tm_mday;
	ptr->SG->time[ptr->ID-1][5] = p->tm_hour;
	ptr->SG->time[ptr->ID-1][6] = p->tm_min;
	ptr->SG->time[ptr->ID-1][7] = p->tm_sec;
	if(ptr->ID == 1)
	{
		ptr->SG->time[0][0] = ((int)timeEnd - timeStart) * 1000 / CLOCKS_PER_SEC;
		ptr->SG->time[0][1] = 'b';
	}
	else if(ptr->ID == 2)
	{
		ptr->SG->time[1][0] = ((int)timeEnd - timeStart) * 1000 / CLOCKS_PER_SEC;
		ptr->SG->time[1][1] = 'q';
	}
	else if (ptr->ID == 3)
	{
		ptr->SG->time[2][0] = ((int)timeEnd - timeStart) * 1000 / CLOCKS_PER_SEC;
		ptr->SG->time[2][1] = 'm'; 
	}
	store(ptr->MyValue, ptr->ID); 
	pthread_exit(0); 
}

void quickSort(int data[], int left, int right)
{
	if (left < right)
	{
		int m = data[(left + right) / 2];
		int i = left - 1;
		int j = right + 1;

		while (1)
		{
			while (data[++i] < m);      // 往右邊找 
			while (data[--j] > m);      // 往左邊找 
			if (i >= j)
				break;
			SWAP(data[i], data[j]);
		}
		quickSort(data, left, i - 1);   // 左邊遞迴 
		quickSort(data, j + 1, right);  // 右邊遞迴 
	}
}

void bubbleSort(int data[], int left, int right)
{
	if (right > 0)
	{
		for (int i = left; i < right; i++)
		if (data[i] > data[i + 1])
			SWAP(data[i], data[i + 1]);
		bubbleSort(data, left, right - 1);
	}
}

void mergeSort(int data[], int left, int right, int temp[], int c)
{
	int mid = (left + right) / 2;
	if (c == 1)
	{
		if (left == right)
		{
			return;
		}
		else
		{
			mergeSort(data, left, mid, temp, 1);
			mergeSort(data, mid + 1, right, temp, 1);
		}
	}
	/* 將已排序好的兩個陣列做合併排序 */
	for (int i = left; i <= right; i++)         // 先copy再放回去
		temp[i] = data[i];
	int top = left, Ltop = left, Rtop = mid + 1;// 合併
	while (top <= right)
	{
		if (temp[Ltop] < temp[Rtop] && Ltop <= mid || Rtop > right)
		{
			data[top++] = temp[Ltop++];
			continue;
		}
		else if (temp[Ltop] >= temp[Rtop] && Rtop <= right)
		{
			data[top++] = temp[Rtop++];
			continue;
		}
		top++;
	}
}

void load(int read[], char* name)
{
	FILE *pFile;
	pFile = fopen(name, "rt");                  // 讀取檔案
	if (NULL == pFile)                          // 判斷檔案是否正常開啟
	{
		printf("Open failure\n");
		system("pause");
		exit(1);
	}
	else
	{
		for (int i = 0; i < length; i++)
		{
			fscanf(pFile, "%d", &read[i]);
		}
	}
	fclose(pFile);                              // 關閉檔案
}

void store(int data[], int i)
{
	FILE *pFile;
	if (i == 1)
		pFile = fopen("bubbleSort.txt", "w");   // 讀取檔案
	else if (i == 2)
		pFile = fopen("quickSort.txt", "w");
	else if (i == 3)
		pFile = fopen("mergeSort.txt", "w");
	else if (i == 4)
		pFile = fopen("advmerge.txt", "w");
	else if (i == 5)
		pFile = fopen("advquick.txt", "w");
	if (NULL == pFile)                          // 判斷檔案是否正常開啟
	{
		printf("Open failure\n");
		system("pause");
		exit(1);
	}
	else
	{
		for (int i = 0; i < length; i++)
		{
			fprintf(pFile, "%d ", data[i]);
		}
	}
	fclose(pFile);                               // 關閉檔案
}
