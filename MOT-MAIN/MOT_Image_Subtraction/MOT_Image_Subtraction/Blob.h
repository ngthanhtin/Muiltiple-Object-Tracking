#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include<iostream>
#include<vector>

using namespace std;
using namespace cv;
// Lớp chứa các blob
class Blob
{
public:
	int xmax, xmin, ymax, ymin, xcenter, ycenter;
	vector<Point> region;
	int width, height;
	int size;

	bool isInBlob(Point a)
	{
		if (a.x >= xmin - 2 && a.x <= xmax + 2
			&& a.y >= ymin - 2 && a.y <= ymax + 2)
			return true;
		return false;
	}

	void Add(Point a)
	{
		region.push_back(a);
		xmax = max(a.x, xmax);
		xmin = min(a.x, xmin);
		ymin = min(a.y, ymin);
		ymax = max(a.y, ymax);

		xcenter = (xmax + xmin) / 2;
		ycenter = (ymax + ymin) / 2;
		size = (xmax - xmin) * (ymax - ymin);
		
		width = xmax - xmin;
		height = ymax - ymin;
	}

	Blob()
	{
		xmax = ymax = xcenter = ycenter = size = 0;
		width = height = 0;
		xmin = ymin = INT_MAX;
	}
	~Blob() {}
};
