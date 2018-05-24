#pragma once
#include <queue>
#include <set>
#include <vector>
#include <iostream>
using namespace std;
static const int MAX = 100;
static const double maxC = 10001.f;
class Hungarian
{
private:
	double c[MAX][MAX];
	double Fx[MAX], Fy[MAX];
	int matchX[MAX], matchY[MAX], Trace[MAX];
	int m, n, k, start, finish;
	void Enter(vector<vector<double>>& DistMatrix, vector<int>& Assignment);
	void Init();
	double GetC(int i, int j);
	void FindAugmentingPath();
	void SubX_AddY();
	void Enlarge();
	void doHungarian();
	
public:
	Hungarian();
	~Hungarian();
	double Solve(vector<vector<double>>& DistMatrix, vector<int>& Assignment);
};

