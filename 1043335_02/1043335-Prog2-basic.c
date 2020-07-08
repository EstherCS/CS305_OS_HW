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
	int       time[3][2];
} S_GLOBAL, *P_GLOBAL;


typedef struct _S_THREAD
{
	int        ID;
	int        ID2;
	P_GLOBAL   SG;
} S_THREAD, *P_THREAD;

void work_thread( P_THREAD ptr );

int main(int argc, char** argv)
{
	printf("> prog2\n");
	
	pthread_t KidThread1, KidThread2, KidThread3, avdMerge;
    P_GLOBAL pG;
    P_THREAD pT[4];
    
    pG = malloc(sizeof(S_GLOBAL));
    pG->sleepTime = atoi(argv[2]);
    for(int i = 0; i < 4; i++)
    {
		pT[i] = malloc(sizeof( S_THREAD ));
		pT[i]->ID = i+1;
		pT[i]->SG = pG;
	}

    pthread_create (&KidThread1, NULL, (void *) &work_thread, pT[0]);
    sleep(0);
    pthread_create (&KidThread2, NULL, (void *) &work_thread, pT[1]);
    sleep(0);
    pthread_create (&KidThread3, NULL, (void *) &work_thread, pT[2]);
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
    
    printf("[Main thread]: the execution time rank \n");
    qsort((void *)pG->time,3, sizeof(*pG->time), compare);
    for(int i = 0; i < 3; i++)
	{
		if(pG->time[0][0] == pG->time[1][0])
		{
			pG->time[0][1] = 'm';
			pG->time[1][1] = 'q';
		}
		else if(pG->time[0][0] == pG->time[1][0] == pG->time[2][0])
		{
			pG->time[0][1] = 'b';
			pG->time[1][1] = 'm';
			pG->time[2][1] = 'q';
		}
		if(pG->time[i][1] == 'b')
			printf("bubblesort: %d ms\n", pG->time[i][0]);
		else if(pG->time[i][1] == 'm')
			printf("mergesort: %d ms\n", pG->time[i][0]);
		else if(pG->time[i][1] == 'q')
			printf("quicksort: %d ms\n", pG->time[i][0]);
	}
    exit(0);
}

void work_thread( P_THREAD ptr )
{
	int data[length] = {}, temp[length] = {};
	clock_t timeStart, timeEnd;
	time_t timep; 
	struct tm *p; 
	char *sort = "";
	if (ptr->ID == 1)
		sort = "bubblesort";
	else if (ptr->ID == 2)
		sort = "quicksort";
	else
		sort = "mergesort";		
	printf("[Main thread]: work thread %d for %s\n", gettid(), sort);
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
		data[i] = ptr->SG->Value[i];
	}
	time(&timep); 
	p = localtime(&timep); /*取得當地時間*/ 
	printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] %s begins\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, sort);
	timeStart = clock(); 
	if(ptr->ID == 1)
		bubbleSort(data, 0, length - 1);
	else if(ptr->ID == 2)
		quickSort(data, 0, length - 1);
	else
		mergeSort(data, 0, length - 1, temp, 1);
	timeEnd = clock(); 
	time(&timep); 
	p = localtime(&timep); /*取得當地時間*/ 
	printf("[Tid = %d]: [%d.%d.%d %d:%d:%d] %s ends\n", gettid(), (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, sort);
	printf("[Tid = %d]: %s %d ms.\n", gettid(), sort, ((int)timeEnd - timeStart) * 1000 / CLOCKS_PER_SEC);
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
	else
	{
		ptr->SG->time[2][0] = ((int)timeEnd - timeStart) * 1000 / CLOCKS_PER_SEC;
		ptr->SG->time[2][1] = 'm'; 
	}
	store(data, ptr->ID); 
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

