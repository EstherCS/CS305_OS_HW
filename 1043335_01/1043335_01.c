#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>

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

typedef struct _S_CHILD
{
	int      Done;
	int      ResponseTime;
} S_CHILD, *P_CHILD;

typedef struct _S_MAIN
{
	char      Ready;
	int       Value[length];
	S_CHILD   A;
	S_CHILD   B;
	S_CHILD   C;
} S_MAIN, *P_MAIN;

int main(int argc, char** argv)
{
	printf("> prog1 data\n");

	clock_t timeStart, timeEnd;
	pid_t   self, child[5];
	self = getpid();                                      // 取得 parent 自己的 pid
	int data[length] = {}, temp[length] = {};
	const char *name = "1043335_shm";                     // file name
	const char *advmergeName = "advmerge_shm";
	const char *advquickName = "advquick_shm";
	const int SIZE = 40016;		                          // file size
	int shm_fd, shm_Merge, shm_Quick;		              // file descriptor, from shm_open()
	P_MAIN Shm_ptr, Shm_advmerge, Shm_advquick;

	/* create the shared memory segment as if it was a file */
	shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
	shm_Merge = shm_open(advmergeName, O_CREAT | O_RDWR, 0666);
	shm_Quick = shm_open(advquickName, O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1)
	{
		printf("shm_fd: Shared memory failed: %s\n", strerror(errno));
		exit(1);
	}
	if (shm_Merge == -1)
	{
		printf("shm_Merge: Shared memory failed: %s\n", strerror(errno));
		exit(1);
	}
	if (shm_Quick == -1)
	{
		printf("shm_Quick: Shared memory failed: %s\n", strerror(errno));
		exit(1);
	}

	ftruncate(shm_fd, SIZE);                              // configure the size of the shared memory segment
	ftruncate(shm_Merge, SIZE);
	ftruncate(shm_Quick, SIZE);

	/* map the shared memory segment to the address space of the process */
	Shm_ptr = (P_MAIN)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	Shm_advmerge = (P_MAIN)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_Merge, 0);
	Shm_advquick = (P_MAIN)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_Quick, 0);
	if (Shm_ptr == MAP_FAILED)
	{
		printf("Shm_ptr: Map failed: %s\n", strerror(errno));
		exit(1);
	}
	if (Shm_advmerge == MAP_FAILED)
	{
		printf("Shm_advmerge: advmerge Map failed: %s\n", strerror(errno));
		exit(1);
	}
	if (Shm_advquick == MAP_FAILED)
	{
		printf("Shm_advquick: advmerge Map failed: %s\n", strerror(errno));
		exit(1);
	}
	memset(Shm_ptr, 0, sizeof(S_MAIN));
	memset(Shm_advmerge, 0, sizeof(S_MAIN));
	memset(Shm_advquick, 0, sizeof(S_MAIN));

	child[0] = fork();                                    // fork 出第一個 child[0]
	int x = 0;                                            // x 為 child 的編號
	if (child[0] != 0)
	{
		printf("[%d parent]: fork %d for bubblesort.\n", self, child[0]);
		x = 1;                                            // 在 parent 裡才能輸出訊息到視窗
		child[1] = fork();                                // 在 parent 裡才會 fork 出第二個 child[1]
	}
	if (child[0] != 0 && child[1] != 0)
	{
		printf("[%d parent]: fork %d for quicksort.\n", self, child[1]);
		x = 2;
		child[2] = fork();                                // 在 parent 裡才會 fork 出第三個 child[2]
	}
	if (child[0] != 0 && child[1] != 0 && child[2] != 0)
	{
		printf("[%d parent]: fork %d for mergesort.\n", self, child[2]);
		x = 3;
		child[3] = fork();                                // 在 parent 裡才會 fork 出第四個 child[3] ( 做 advmerge)
	}
	if (child[0] != 0 && child[1] != 0 && child[2] != 0 && child[3] != 0)
	{
		printf("[%d parent]: fork %d for advmerge.\n", self, child[3]);
		x = 4;
		child[4] = fork();                                // 在 parent 裡才會 fork 出第五個 child[4] ( 做 advquick)
	}
	if (child[0] != 0 && child[1] != 0 && child[2] != 0 && child[3] != 0 && child[4] != 0)
	{
		printf("[%d parent]: fork %d for advquick.\n\n", self, child[4]);
	}

	if (child[x] < 0)
	{
		fprintf(stderr, "Fork Failed");                   // error occurred
		exit(-1);
	}
	else if (x < 3 && child[x] == 0)                      /// chlid[0]~child[2]會進入
	{
		while (Shm_ptr->Ready != 'T');                    // 如果 parent 還沒將 F 改成 T，就代表還不能做排序

		for (int i = 0; i < length; i++)
		{
			data[i] = Shm_ptr->Value[i];                  // 把數字放入 data 這個陣列
		}
		char *sort = "";
		if (x == 0)
			sort = "bubblesort";
		else if (x == 1)
			sort = "quicksort";
		else
			sort = "mergesort";

		printf("[%d child]: %s begin.\n", getpid(), sort);
		sleep(1);
		timeStart = clock();                              // 開始計時
		if (x == 0)
			bubbleSort(data, 0, length - 1);
		else if (x == 1)
			quickSort(data, 0, length - 1);
		else
			mergeSort(data, 0, length - 1, temp, 1);
		timeEnd = clock();                                // 結束計時

		printf("[%d child]: %s end.\n", getpid(), sort);
		printf("[%d child]: %s %d ms.\n", getpid(), sort, ((int)timeEnd - timeStart) * 1000 / CLOCKS_PER_SEC);

		if (x == 0)
		{
			Shm_ptr->A.ResponseTime = ((int)timeEnd - timeStart) * 1000 / CLOCKS_PER_SEC;
			Shm_ptr->A.Done = 1;
		}
		else if (x == 1)
		{
			Shm_ptr->B.ResponseTime = ((int)timeEnd - timeStart) * 1000 / CLOCKS_PER_SEC;
			Shm_ptr->B.Done = 1;
		}
		else
		{
			Shm_ptr->C.ResponseTime = ((int)timeEnd - timeStart) * 1000 / CLOCKS_PER_SEC;
			Shm_ptr->C.Done = 1;
		}

		printf("[%d child]: %s results written to file %s.txt.\n", getpid(), sort, sort);
		store(data, x);                                   // 將結果存到 txt 檔中
		exit(0);                                          // child process 在這會結束不會繼續往下
	}
	else if (x == 3 && child[x] == 0)                     /// 此為 advmerge ( child[3] )
	{
		int Pidmerge = getpid();
		while (Shm_ptr->Ready != 'T');                    // 如果 parent 還沒將 F 改成 T，就代表還不能做排序
		sleep(1);
		timeStart = clock();                              // 開始計時
		int Mchild[2] = {}, y = 0;                        // y 為 Mchild 的編號
		Mchild[0] = fork();                               // fork 出第一個 child[0]		
		if (Mchild[0] != 0)
		{
			printf("[%d Mparent]: fork %d for child_advmerge_1.\n", Pidmerge, Mchild[0]);
			y = 1;                                        // 在 parent 裡才能輸出訊息到檔案
			Mchild[1] = fork();                           // 在 parent 裡才會 fork 出第二個 Mchild[1]
		}
		if (Mchild[0] != 0 && Mchild[1] != 0)
		{
			printf("[%d Mparent]: fork %d for child_advmerge_2.\n", Pidmerge, Mchild[1]);
		}

		if (Mchild[y] < 0)
		{
			fprintf(stderr, "Fork Failed");               // error occurred
			exit(-1);
		}
		else if (Mchild[y] == 0)                          /// grandchild 會進來 ( child[3] 的兩個 child )
		{
			while (Shm_advmerge->Ready != 'T');
			if (y == 0)
			{
				for (int i = 0; i < length / 2; i++)
				{
					data[i] = Shm_ptr->Value[i];         // 把數字放入 data 這個陣列
				}
			}
			else
			{
				int d = 0;
				for (int i = length / 2; i < length; i++)
				{
					data[d++] = Shm_ptr->Value[i];        // 把數字放入 data 這個陣列
				}
			}
			char *sort = "";
			if (y == 0)
				sort = "child_advmerge_1";
			else
				sort = "child_advmerge_2";
			printf("[%d Mchild]: %s begin.\n", getpid(), sort);
			sleep(1);
			mergeSort(data, 0, length / 2 - 1, temp, 1);
			printf("[%d Mchild]: %s end.\n", getpid(), sort);
			if (y == 0)
			{
				Shm_advmerge->B.Done = 1;
			}
			else
			{
				Shm_advmerge->C.Done = 1;
			}
			if (y == 0)
			{
				for (int i = 0; i < length / 2; i++)
				{
					Shm_advmerge->Value[i] = data[i];     // 把數字放 shared memory
				}
			}
			else
			{
				int d = length / 2;
				for (int i = 0; i < length / 2; i++)
				{
					Shm_advmerge->Value[d++] = data[i];   // 把數字放 shared memory
				}
			}
			exit(0);                                      // child process 在這會結束不會繼續往下			
		}
		else                                              /// child[3]會進來 
		{
			Shm_advmerge->Ready = 'T';
			wait(NULL);
			wait(NULL);
			while (Shm_advmerge->B.Done != 1 || Shm_advmerge->C.Done != 1);
			sleep(2);                                     // parent 睡 5 秒，避免排序還沒排完
			for (int i = 0; i < length; i++)
			{
				data[i] = Shm_advmerge->Value[i];         // 把數字放入 data 這個陣列
			}
			printf("[%d Mparent]: parent_advmerge begin.\n", Pidmerge);
			sleep(1);

			mergeSort(data, 0, length - 1, temp, 0);
			timeEnd = clock();                            // 結束計時
			printf("[%d Mparent]: parent_advmerge end.\n", Pidmerge);
			printf("[%d Mparent]: parent_advmerge %d ms.\n", Pidmerge, ((int)timeEnd - timeStart) * 1000 / CLOCKS_PER_SEC);
			printf("***[%d Mparent]: patent_advmerge %d ns.\n", Pidmerge, ((int)timeEnd - timeStart) * 100000 / CLOCKS_PER_SEC);
			Shm_advmerge->A.ResponseTime = ((int)timeEnd - timeStart) * 1000 / CLOCKS_PER_SEC;
			Shm_advmerge->B.ResponseTime = ((int)timeEnd - timeStart) * 100000 / CLOCKS_PER_SEC;
			Shm_advmerge->A.Done = 1;
			printf("[%d Mparent]: parent_advmerge results written to file advmerge.txt.\n", Pidmerge);
			store(data, 3);                               // 將結果存到 txt 檔中			
			exit(0);                                      // child process 在這會結束不會繼續往下						
		}
	}
	else if (x == 4 && child[x] == 0)                     /// 此為 advquick ( child[4] )
	{
		int Pidquick = getpid();
		while (Shm_ptr->Ready != 'T');                    // 如果 parent 還沒將 F 改成 T，就代表還不能做排序
		sleep(1);
		timeStart = clock();                              // 開始計時
		for (int i = 0; i < length; i++)
		{
			data[i] = Shm_ptr->Value[i];                  // 把數字放入 data 這個陣列
		}
		int m = data[(0 + length - 1) / 2];
		int a = -1, b = length;
		while (1)
		{
			while (data[++a] < m);                        // 往右邊找 
			while (data[--b] > m);                        // 往左邊找 
			if (a >= b)
				break;
			SWAP(data[a], data[b]);
		}
		int Qchild[2] = {}, z = 0;                        // z 為 Mchild 的編號
		Qchild[0] = fork();                               // fork 出第一個 child[0]		
		if (Qchild[0] != 0)
		{
			printf("[%d Qparent]: fork %d for child_advquick_1.\n", Pidquick, Qchild[0]);
			z = 1;                                        // 在 parent 裡才能輸出訊息到檔案
			Qchild[1] = fork();                           // 在 parent 裡才會 fork 出第二個 child[1]
		}
		if (Qchild[0] != 0 && Qchild[1] != 0)
		{
			printf("[%d Qparent]: fork %d for child_advquick_2.\n", Pidquick, Qchild[1]);
		}

		if (Qchild[z] < 0)
		{
			fprintf(stderr, "Fork Failed");               // error occurred
			exit(-1);
		}
		else if (Qchild[z] == 0)                          /// child[4] 的兩個 child 會進來
		{
			while (Shm_advquick->Ready != 'T');
			int end = 0;
			if (z == 0)
			{
				for (int i = 0; i < a; i++)
				{
					temp[end++] = data[i];                // 把數字放入 data 這個陣列
				}
			}
			else
			{
				for (int i = b + 1; i < length; i++)
				{
					temp[end++] = data[i];                // 把數字放入 data 這個陣列
				}
			}

			char *sort = "";
			if (z == 0)
				sort = "child-advquick_1";
			else
				sort = "child-advquick_2";
			printf("[%d Qchild]: %s begin.\n", getpid(), sort);
			sleep(1);
			quickSort(temp, 0, end - 1);
			printf("[%d Qchild]: %s end.\n", getpid(), sort);
			if (z == 0)
			{
				Shm_advquick->B.Done = 1;
			}
			else
			{
				Shm_advquick->C.Done = 1;
			}
			if (z == 0)
			{
				for (int i = 0; i < a; i++)
				{
					Shm_advquick->Value[i] = temp[i];     // 把數字放 shared memory
				}
			}
			else
			{
				int d = b + 1;
				for (int i = 0; i < end; i++)
				{
					Shm_advquick->Value[d++] = temp[i];   // 把數字放 shared memory
				}
			}
			exit(0);                                      // child process 在這會結束不會繼續往下			
		}
		else                                              /// child[4] 會進來
		{
			Shm_advquick->Ready = 'T';
			wait(NULL);
			wait(NULL);
			while (Shm_advquick->B.Done != 1 || Shm_advquick->C.Done != 1);
			timeEnd = clock();                            // 結束計時
			sleep(2);                                     // parent 睡 5 秒，避免排序還沒排完
			for (int i = 0; i < length; i++)
			{
				data[i] = Shm_advquick->Value[i];         // 把數字放入 data 這個陣列
			}
			printf("[%d Qparent]: parent_advquick %d ms.\n", Pidquick, ((int)timeEnd - timeStart) * 1000 / CLOCKS_PER_SEC);
			printf("***[%d Qparent]: parent-advquick %d ns.\n", Pidquick, ((int)timeEnd - timeStart) * 100000 / CLOCKS_PER_SEC);
			Shm_advquick->A.ResponseTime = ((int)timeEnd - timeStart) * 1000 / CLOCKS_PER_SEC;
			Shm_advquick->B.ResponseTime = ((int)timeEnd - timeStart) * 100000 / CLOCKS_PER_SEC;
			Shm_advquick->A.Done = 1;
			printf("[%d Qparent]: parent_advquick results written to file advquick.txt.\n", Pidquick);
			store(data, 4);                               // 將結果存到 txt 檔中		
			exit(0);                                      // child process 在這會結束不會繼續往下						
		}
	}
	else                                                  /// 所有 child 的 parent 會進來
	{
		load(Shm_ptr->Value, argv[1]);                    // 讀取檔案中要排序的數字

		printf("[%d parent]: read data and write to shared memory.\n", self);
		printf("[%d parent]: tell childs to read data.\n\n", self);
		Shm_ptr->Ready = 'T';                             // 更改 0 為 T，通知 child 們開始排序

		for (int i = 0; i < 5; i++)
			waitpid(child[i], NULL, WNOHANG);

		while (Shm_ptr->A.Done != 1 || Shm_ptr->B.Done != 1 || Shm_ptr->C.Done != 1);
		while (Shm_advmerge->A.Done != 1 || Shm_advmerge->B.Done != 1 || Shm_advmerge->C.Done != 1);
		while (Shm_advquick->A.Done != 1 || Shm_advquick->B.Done != 1 || Shm_advquick->C.Done != 1);

		printf("[%d parent]: the execution time rank .\n", self);

		int Sort[5];
		Sort[0] = Shm_ptr->A.ResponseTime;
		Sort[1] = Shm_ptr->B.ResponseTime;
		Sort[2] = Shm_ptr->C.ResponseTime;
		Sort[3] = Shm_advmerge->A.ResponseTime;
		Sort[4] = Shm_advquick->A.ResponseTime;
		qsort((void *)Sort, 5, sizeof(int), compare);

		int ch[4] = {};
		for (int i = 0; i < 5; i++)
		{
			if (ch[0] == 0 && Sort[i] == Shm_advquick->A.ResponseTime)
			{
				printf("advquick: %d ms, ( %d ns ).\n", Shm_advquick->A.ResponseTime, Shm_advquick->B.ResponseTime);
				ch[0] = 1;
			}
			else if (ch[1] == 0 && Sort[i] == Shm_advmerge->A.ResponseTime)
			{
				printf("advmerge: %d ms, ( %d ns ).\n", Shm_advmerge->A.ResponseTime, Shm_advmerge->B.ResponseTime);
				ch[1] = 1;
			}
			else if (ch[2] == 0 && Sort[i] == Shm_ptr->A.ResponseTime)
			{
				printf("bubblesort: %d ms.\n", Shm_ptr->A.ResponseTime);
				ch[2] = 1;
			}
			else if (ch[3] == 0 && Sort[i] == Shm_ptr->B.ResponseTime)
			{
				printf("quicksort: %d ms.\n", Shm_ptr->B.ResponseTime);
				ch[3] = 1;
			}
			else
				printf("mergesort: %d ms.\n", Shm_ptr->C.ResponseTime);
		}
		printf("> \n");
	}

	/* remove the mapped memory segment from the address space of the process */
	if (munmap(Shm_ptr, SIZE) == -1)
	{
		printf("Shm_ptr: Unmap failed: %s\n", strerror(errno));
		exit(1);
	}
	/* close the shared memory segment as if it was a file */
	if (close(shm_fd) == -1)
	{
		printf("shm_fd: Close failed: %s\n", strerror(errno));
		exit(1);
	}
	/* remove the shared memory segment from the file system */
	if (shm_unlink(name) == -1)
	{
		printf("1043335_shm: Error removing %s: %s\n", name, strerror(errno));
		exit(1);
	}

	/* remove the mapped memory segment from the address space of the process */
	if (munmap(Shm_advmerge, SIZE) == -1)
	{
		printf("Shm_advmerge: Unmap failed: %s\n", strerror(errno));
		exit(1);
	}
	/* close the shared memory segment as if it was a file */
	if (close(shm_Merge) == -1)
	{
		printf("shm_Merge: Close failed: %s\n", strerror(errno));
		exit(1);
	}
	/* remove the shared memory segment from the file system */
	if (shm_unlink(advmergeName) == -1)
	{
		printf("advmerge_shm: Error removing %s: %s\n", name, strerror(errno));
		exit(1);
	}

	/* remove the mapped memory segment from the address space of the process */
	if (munmap(Shm_advquick, SIZE) == -1)
	{
		printf("Shm_advquick: Unmap failed: %s\n", strerror(errno));
		exit(1);
	}
	/* close the shared memory segment as if it was a file */
	if (close(shm_Quick) == -1)
	{
		printf("shm_Quick: Close failed: %s\n", strerror(errno));
		exit(1);
	}
	/* remove the shared memory segment from the file system */
	if (shm_unlink(advquickName) == -1)
	{
		printf("adquick_shm: Error removing %s: %s\n", name, strerror(errno));
		exit(1);
	}
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
	if (i == 0)
		pFile = fopen("bubbleSort.txt", "w");   // 讀取檔案
	else if (i == 1)
		pFile = fopen("quickSort.txt", "w");
	else if (i == 2)
		pFile = fopen("mergeSort.txt", "w");
	else if (i == 3)
		pFile = fopen("advmerge.txt", "w");
	else if (i == 4)
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
