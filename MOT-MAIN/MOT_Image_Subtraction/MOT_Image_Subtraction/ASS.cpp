#include "ASS.h"



ASS::ASS()
{
	/*c = new double*[MAX];
	for (int i = 0; i < MAX; i++)
	{
		c[i] = new double[MAX];
	}*/
	/*Fx = new double[MAX];
	Fy = new double[MAX];

	matchX = new int[MAX];
	matchY = new int[MAX];
	Trace = new int[MAX];*/
}


ASS::~ASS()
{
	/*for (int i = 0; i < MAX; i++)
		delete[]c[i];
	delete[]c;*/
	/*delete[]Fx;
	delete[]Fy;
	delete[]matchX;
	delete[]matchY;
	delete[]Trace;*/
}

double ASS::Solve(vector<vector<double>>& DistMatrix, vector<int>& Assignment)
{
	Enter(DistMatrix, Assignment);
	Init();
	Hungary();
	//update assignment
	for (int x = 0; x < m; x++)
	{
		Assignment.push_back(matchX[x]);
	}
	for (int x = 0; x < m; x++)
	{
		for (int y = 0; y < n; y++)
		{

			DistMatrix[x][y] = c[x][y];
			if (c[x][y] == maxC)
				DistMatrix[x][y] = 0.f;
		}
	}
	float cost = 0.f;
	for (int x = 0; x < m; x++)
	{
		int y = matchX[x];
		if (c[x][y] < maxC)
		{
			cost += c[x][y];
		}
	}
	/*for (int i = 0; i < MAX; i++)
		delete[]c[i];
	delete[]c;
	delete[]Fx;
	delete[]Fy;
	delete[]matchX;
	delete[]matchY;
	delete[]Trace;*/
	return cost;
}

void ASS::Enter(vector<vector<double>>& DistMatrix, vector<int>& Assignment)
{
	int i, j;
	m = DistMatrix.size(); 
	n = DistMatrix[0].size(); 
	
	k = m > n ? m : n;//tìm số đỉnh nhiều nhất trong 2 bên X và Y
					  //gán tất cả các cạnh là giá trị maxC ( rất lớn)
	for (i = 0; i < k; i++)
	{
		for (j = 0; j < k; j++)
		{
			if (i >= m || j >= n)
			{
				c[i][j] = maxC;
				continue;
			}
			if (DistMatrix[i][j] == 0)
				c[i][j] = maxC;
			else
				c[i][j] = DistMatrix[i][j];
		}
	}
}

void ASS::Init()
{
	int i, j;
	//Khởi tạo ban đầu 2 bộ ghép đều rỗng (các giá trị được gán bằng -1 nghĩa là chưa có
	// đỉnh X gán với Y hoặc ngược lại)
	///memset(matchX, -1, sizeof(matchX));
	for (i = 0; i < MAX; i++)
	{
		matchX[i] = -1;
		matchY[i] = -1;
	}
	///memset(matchY, -1, sizeof(matchY));
	//tìm trọng số nhỏ nhất của các cạnh liên thuộc với X[i]
	for (i = 0; i < k; i++)
	{
		Fx[i] = maxC;
		for (j = 0; j < k; j++)
		{
			if (c[i][j] < Fx[i])
			{
				Fx[i] = c[i][j];
			}
		}
	}
	//tìm trọng số nhỏ nhất của các cạnh liên thuộc với Y[j]
	for (j = 0; j < k; j++)
	{
		Fy[j] = maxC;
		for (i = 0; i < k; i++)
		{
			if (c[i][j] - Fx[i] < Fy[j])
			{
				Fy[j] = c[i][j] - Fx[i];
			}
		}
	}
}

double ASS::GetC(int i, int j)
{
	return c[i][j] - Fx[i] - Fy[j];
}

void ASS::FindAugmentingPath()
{
	queue<int> q;
	int i, j, first, last;

	for (i = 0; i < MAX;i++)
	{
		Trace[i] = -1;
	}
	///memset(Trace, -1, sizeof(Trace));

	//chạy thuật toán BFS để tìm đường mở
	q.push(start);
	first = 0;
	last = 0;
	do
	{
		i = q.front();
		q.pop();
		//cout << "Dinh i la: " << i << endl;
		for (j = 0; j < k; j++)
		{
			//cout << "Get[" << i << "][" << j << "] = " << GetC(i, j) << endl;
			//cout << "Trace[" << j << "]" << Trace[j] << endl;
			if (Trace[j] == -1 && GetC(i, j) == 0.0f)
			{
				Trace[j] = i;
				if (matchY[j] == -1)
				{
					finish = j;
					return;
				}
				q.push(matchY[j]);
			}
		}
	} while (!q.empty());
}

void ASS::SubX_AddY()
{
	int i, j, t;
	double Delta;
	set<int> VisitedX, VisitedY;
	/*
	Để ý rằng:
	VisitedY = {y \ Trace[y] khác -1}
	VisitedX = {start} giao match(VisitedY) = {start} giao {matchY[y] Trace[y] khác -1}
	*/
	VisitedX.insert(start);
	for (j = 0; j < k; j++)
	{
		if (Trace[j] != -1)
		{
			VisitedX.insert(matchY[j]);
			VisitedY.insert(j);
		}
	}
	//Sau khi tìm được VisitedX và VisitedY, ta tìm delta là trọng số 
	// nhỏ nhất của cạnh nối từ VisitedX ra Y\VisitedY
	Delta = maxC;
	for (i = 0; i < k; i++)
	{
		if (VisitedX.find(i) != VisitedX.end())
		{
			for (j = 0; j < k; j++)
			{
				if ((VisitedY.find(j) == VisitedY.end()) && (GetC(i, j) < Delta))
					Delta = GetC(i, j);
			}
		}
	}
	//xoay trọng số cạnh
	for (t = 0; t < k; t++)
	{
		//trừ trọng số những cạnh liên thuộc với VisitedX đi Delta
		if (VisitedX.find(t) != VisitedX.end())
			Fx[t] = Fx[t] + Delta;
		//Cộng trọng số những cạnh liên thuộc với VisitedY lên Delta
		if (VisitedY.find(t) != VisitedY.end())
			Fy[t] = Fy[t] - Delta;
	}
}

void ASS::Enlarge()
{
	int x, next;
	do
	{
		x = Trace[finish];
		next = matchX[x];
		matchX[x] = finish;
		matchY[finish] = x;
		finish = next;
	} while (finish != -1);
}

void ASS::Hungary()
{
	int x, y;
	for (x = 0; x < k; x++)
	{
		//khởi tạo điểm bắt đầu và kết thúc của 1 đường mở
		//finish = -1 nghĩa là chưa tìm thấy đường mở
		start = x;
		finish = -1;
		do
		{
			FindAugmentingPath(); // tìm đường mở
			if (finish == -1) // nếu ko tìm được đường mở thì xoay các trọng số cạnh
				SubX_AddY();
		} while (finish == -1);
		Enlarge();//tăng cặp dựa trên đường mở tìm được
	}
}
