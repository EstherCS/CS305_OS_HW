#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <iomanip>
using namespace std;

void load(int &F, vector<int> &R, char* name);
int Optimal(int Fsize, vector<int> reference);
int FIFO(int Fsize, vector<int> reference);
int LRU(int Fsize, vector<int> reference);
int LFU(int Fsize, vector<int> reference);
int Clock(int Fsize, vector<int> reference);

int main(int argc, char** argv)
{
	int Fsize, PageFault;
	vector <int> reference;
	load(Fsize, reference, argv[1]);

	cout << "Optimal ========== " << endl;
	PageFault = Optimal(Fsize, reference);
	cout << "Optimal Total Page Fault:    " << PageFault << endl << endl;

	cout << "FIFO ========== " << endl;
	PageFault = FIFO(Fsize, reference);
	cout << "FIFO Total Page Fault:       " << PageFault << endl << endl;

	cout << "LRU ========== " << endl;
	PageFault = LRU(Fsize, reference);
	cout << "LRU Total Page Fault:        " << PageFault << endl << endl;

	cout << "LFU ========== " << endl;
	PageFault = LFU(Fsize, reference);
	cout << "LFU Total Page Fault:        " << PageFault << endl << endl;

	cout << "Clock ========== " << endl;
	PageFault = Clock(Fsize, reference);
	cout << "Clock Total Page Fault:       " << PageFault << endl << endl;
}

void load(int &F, vector<int> &R, char* name)
{
	ifstream infile(name, ios::in);

	if (infile.is_open())                                // 判斷檔案是否正常開啟
	{
		string reference;
		infile >> reference;
		F = reference[2] - '0';                          // 取得 F
		infile >> reference;
		int num = reference[3] - '0';
		if (reference.size() - 3 > 1)
		{
			num = 10 * (reference[3] - '0') + (reference[4] - '0');
		}
		R.push_back(num);                                // 取得第一個數字
		while (infile >> reference)
		{
			num = reference[0] - '0';
			if (reference.size() - 1 > 0)
			{
				if (reference[1] != '"')                 // 取得剩餘的數字
					num = 10 * (reference[0] - '0') + (reference[1] - '0');
			}
			R.push_back(num);
		}
	}
	else
	{
		cout << "Open Error" << endl;
		exit(1);
	}
	infile.close();
}

int Optimal(int Fsize, vector<int> reference)
{
	vector <int> frame;
	vector <int> first(Fsize, -1);
	int fault = Fsize;
	int point = 0, Fcount = 0;
	for (; Fcount < Fsize; point++, Fcount++)               // frame 數有幾個就放幾個進去
	{
		int output = 0;
		if (find(frame.begin(), frame.end(), reference[point]) == frame.end())
		{
			first[Fcount] = point;
			frame.push_back(reference[point]);
		}
		else
		{
			output = 1;
			Fcount--;
		}
		cout << setw(2) << reference[point] << " : ";
		for (int i = 0; i < Fsize; i++)
		{
			if (i < frame.size())
				cout << setw(4) << frame[i] << " ";
			else
				cout << setw(5) << " X ";
		}
		if (!output)
			cout << setw(16) << "page fault " << endl;
		else
			cout << endl;
	}
	int fifo = 0;
	for (; point < reference.size(); point++)
	{
		int Mfirst = 10000;
		cout << setw(2) << reference[point] << " : ";
		vector <bool> check(Fsize, 0);
		if (find(frame.begin(), frame.end(), reference[point]) == frame.end())
		{
			int replace = -1, count = 0;
			for (int i = point + 1; i < reference.size(); i++)
			{
				for (int j = 0; j < frame.size(); j++)
				{
					if (reference[i] == frame[j])
					{
						if (check[j] != 1)
						{
							check[j] = 1;                  // 找未來有幾個存在
							if (count == Fsize - 1)
								replace = j;               // 若是未來最後一個出現，代表要替換掉
							count++;
						}
					}
				}
			}
			if (count != Fsize)                            // 未來找不到的數字這邊做處理
			{
				for (int i = 0; i < Fsize; i++)
				{
					if (check[i] == 0)
					{
						if (first[i] < Mfirst)
						{
							Mfirst = first[i];             // 未來沒有的數字中，最早進來的被替換掉
							replace = i;
						}
					}

				}
			}
			first[replace] = point;
			frame[replace] = reference[point];
			for (int i = 0; i < frame.size(); i++)
			{
				cout << setw(4) << frame[i] << " ";
			}
			cout << setw(16) << "page fault ";
			fault++;
		}
		else                                                // 數字已存在 frame 中
		{
			for (int i = 0; i < frame.size(); i++)
			{
				cout << setw(4) << frame[i] << " ";
			}
		}
		cout << endl;
	}
	return fault;
}

int FIFO(int Fsize, vector<int> reference)
{
	vector <int> frame;
	int fault = Fsize;
	int point = 0, Fcount = 0;
	for (; Fcount < Fsize; point++, Fcount++)               // frame 數有幾個就放幾個進去
	{
		int output = 0;
		if (find(frame.begin(), frame.end(), reference[point]) == frame.end())
		{
			frame.push_back(reference[point]);
		}
		else
		{
			output = 1;
			Fcount--;
		}
		cout << setw(2) << reference[point] << " : ";
		for (int i = 0; i < Fsize; i++)
		{
			if (i < frame.size())
				cout << setw(4) << frame[i] << " ";
			else
				cout << setw(5) << " X ";
		}
		if (!output)
			cout << setw(16) << "page fault " << endl;
		else
			cout << endl;
	}
	int fifo = 0;
	for (; point < reference.size(); point++)
	{
		if (fifo > Fsize - 1)
			fifo = 0;
		cout << setw(2) << reference[point] << " : ";
		if (find(frame.begin(), frame.end(), reference[point]) == frame.end())
		{
			frame[fifo++] = reference[point];              // 先進來的先替換掉
			for (int i = 0; i < frame.size(); i++)
			{
				cout << setw(4) << frame[i] << " ";
			}
			cout << setw(16) << "page fault ";
			fault++;
		}
		else                                                // 數字已存在 frame 中
		{
			for (int i = 0; i < frame.size(); i++)
			{
				cout << setw(4) << frame[i] << " ";
			}
		}
		cout << endl;
	}
	return fault;
}

int LRU(int Fsize, vector<int> reference)
{
	vector <int> frame;
	int fault = Fsize;
	int point = 0, Fcount = 0;
	for (; Fcount < Fsize; point++, Fcount++)               // frame 數有幾個就放幾個進去
	{
		int output = 0;
		if (find(frame.begin(), frame.end(), reference[point]) == frame.end())
		{
			frame.push_back(reference[point]);
		}
		else
		{
			output = 1;
			Fcount--;
		}
		cout << setw(2) << reference[point] << " : ";
		for (int i = 0; i < Fsize; i++)
		{
			if (i < frame.size())
				cout << setw(4) << frame[i] << " ";
			else
				cout << setw(5) << " X ";
		}
		if (!output)
			cout << setw(16) << "page fault " << endl;
		else
			cout << endl;
	}
	for (; point < reference.size(); point++)
	{
		cout << setw(2) << reference[point] << " : ";
		vector <bool> check(Fsize, 0);
		if (find(frame.begin(), frame.end(), reference[point]) == frame.end())
		{
			int replace = -1, count = 0;
			for (int i = point - 1; i >= 0 && count != Fsize; i--)
			{
				for (int j = 0; j < frame.size() && count != Fsize; j++)
				{
					if (reference[i] == frame[j])
					{
						if (check[j] != 1)
						{
							check[j] = 1;
							if (count == Fsize - 1)
								replace = j;               // 從過去找，最久遠的被替換掉
							count++;
						}
					}
				}
			}
			frame[replace] = reference[point];
			for (int i = 0; i < frame.size(); i++)
			{
				cout << setw(4) << frame[i] << " ";
			}
			cout << setw(16) << "page fault ";
			fault++;
		}
		else                                                // 數字已存在 frame 中
		{
			for (int i = 0; i < frame.size(); i++)
			{
				cout << setw(4) << frame[i] << " ";
			}
		}
		cout << endl;
	}
	return fault;
}

int LFU(int Fsize, vector<int> reference)
{
	vector <int> frame;
	vector <int> first(Fsize, -1);
	vector <int> count(Fsize, 1);
	int fault = Fsize;
	int point = 0, Fcount = 0, Min;
	for (; Fcount < Fsize; point++, Fcount++)               // frame 數有幾個就放幾個進去
	{
		int output = 0;
		if (find(frame.begin(), frame.end(), reference[point]) == frame.end())
		{
			frame.push_back(reference[point]);
		}
		else
		{
			output = 1;
			Fcount--;
			count[find(frame.begin(), frame.end(), reference[point]) - frame.begin()]++;
		}
		cout << setw(2) << reference[point] << " : ";
		for (int i = 0; i < Fsize; i++)
		{
			if (i < frame.size())
				cout << setw(4) << frame[i] << " ";
			else
				cout << setw(5) << " X ";
		}
		if (!output)
			cout << setw(16) << "page fault " << endl;
		else
			cout << endl;
	}
	int fifo = 0;
	for (; point < reference.size(); point++)
	{
		cout << setw(2) << reference[point] << " : ";
		if (find(frame.begin(), frame.end(), reference[point]) == frame.end())
		{
			int replace = -1, Mfirst = 10000;
			Min = *min_element(count.begin(), count.end());
			for (int i = 0; i < Fsize; i++)
			{
				if (count[i] == Min)
				{
					if (first[i] < Mfirst)
					{
						Mfirst = first[i];
						replace = i;                       // 找到被使用最少次的替換掉 
					}
				}
			}
			first[replace] = point;
			count[replace] = 1;
			frame[replace] = reference[point];
			for (int i = 0; i < frame.size(); i++)
			{
				cout << setw(4) << frame[i] << " ";
			}
			cout << setw(16) << "page fault ";
			fault++;
		}
		else                                                // 數字已存在 frame 中
		{
			for (int i = 0; i < Fsize; i++)
			{
				if (reference[point] == frame[i])
					count[i]++;
			}
			for (int i = 0; i < frame.size(); i++)
			{
				cout << setw(4) << frame[i] << " ";
			}
		}
		cout << endl;
	}
	return fault;
}

int Clock(int Fsize, vector<int> reference)
{
	vector <int> frame;
	vector <int> Rbit(Fsize, 1);
	int fault = Fsize;
	int point = 0, Fcount = 0;
	for (; Fcount < Fsize; point++, Fcount++)               // frame 數有幾個就放幾個進去
	{
		int output = 0;
		if (find(frame.begin(), frame.end(), reference[point]) == frame.end())
		{
			frame.push_back(reference[point]);
		}
		else
		{
			output = 1;
			Fcount--;
		}
		cout << setw(2) << reference[point] << " : ";
		for (int i = 0; i < Fsize; i++)
		{
			if (i < frame.size())
				cout << setw(4) << frame[i] << " ";
			else
				cout << setw(5) << " X ";
		}
		if (!output)
			cout << setw(16) << "page fault " << endl;
		else
			cout << endl;
	}
	int fifo = 0;
	for (; point < reference.size(); point++)
	{
		if (fifo > Fsize - 1)
			fifo = 0;
		cout << setw(2) << reference[point] << " : ";
		if (find(frame.begin(), frame.end(), reference[point]) == frame.end())
		{
			bool out = 0;
			while (!out)
			{
				if (fifo > Fsize - 1)
					fifo = 0;
				if (Rbit[fifo] == 0)                       // 根據 reference bit 去給第二次機會 
				{
					frame[fifo] = reference[point];        // 沒機會的被替換掉
					Rbit[fifo++] = 1;
					for (int i = 0; i < frame.size(); i++)
					{
						cout << setw(4) << frame[i] << " ";
					}
					cout << setw(16) << "page fault ";
					fault++;
					out = 1;
				}
				else
				{
					Rbit[fifo] = 0;
					fifo++;
				}
			}
		}
		else                                                // 數字已存在 frame 中
		{
			Rbit[find(frame.begin(), frame.end(), reference[point]) - frame.begin()] = 1;
			for (int i = 0; i < frame.size(); i++)
			{
				cout << setw(4) << frame[i] << " ";
			}
		}
		cout << endl;
	}
	return fault;
}
