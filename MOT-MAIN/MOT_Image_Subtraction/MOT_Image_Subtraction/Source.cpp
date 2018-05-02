#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include "opencv2/opencv.hpp"
#include "opencv2\video\background_segm.hpp"
#include <opencv/cv.h>
#include<iostream>
#include<vector>
#include "Ctracker.h"
using namespace std;
using namespace cv;
Scalar Colors[] = { Scalar(255,0,0),
Scalar(0,0,255),
Scalar(255,255,0),
Scalar(0,255,255),
Scalar(255,0,255),
Scalar(255,127,255),
Scalar(127,0,255),
Scalar(127,0,127) };
class Blob
{
public:
	int xmax, xmin, ymax, ymin, xcenter, ycenter;
	vector<Point> region;
	int size;
	int width, height;

	bool isInBlob(Point a)
	{
		if (a.x >= xmin - 1 && a.x <= xmax + 1
			&& a.y >= ymin - 1 && a.y <= ymax + 1)
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
		width = xmax - xmin;
		height = ymax - ymin;
		size = width * height;
	}

	Blob()
	{
		xmax = ymax = xcenter = ycenter = size = 0;
		xmin = ymin = INT_MAX;
		width = height = 0;
	}
	~Blob(){}
};

int main()
{

	CTracker tracker(0.2, 0.5, 60.0, 10, 30);
	vector<Point2f> centers;
	vector<Blob> b;
	//
	VideoCapture capVid("in.avi");
	//VideoCapture capVid("Wildlife.wmv");
	if (!capVid.isOpened())
		return 1;

	Mat imgFrame1;
	Mat imgFrame2;
	
	//
	capVid.read(imgFrame1); // current frame
	capVid.read(imgFrame2);
	//
	// MOG2 Background subtractor
	Ptr<BackgroundSubtractor> pMOG2;

	

	// Create MOG2 Background Subtractor object
	pMOG2 = createBackgroundSubtractorMOG2();
	//
	char EscKeyCheck = 0;
	int frameCount = 2;

	int aa = 1;
	while (capVid.isOpened() && EscKeyCheck != 27)
	{

		
		//
		Mat imgFrame1Clone = imgFrame1.clone();
		Mat imgFrame2Clone = imgFrame2.clone();
		
		//
		//
		Mat imgDifference, imgThresh;

		cvtColor(imgFrame1Clone, imgFrame1Clone, CV_BGR2GRAY);
		cvtColor(imgFrame2Clone, imgFrame2Clone, CV_BGR2GRAY);
		if (imgFrame1Clone.channels() != 1 || imgFrame2Clone.channels() != 1)
			return 1;


		int thresh = 25;
		absdiff(imgFrame1Clone, imgFrame2Clone, imgDifference);
		//dilation
		cv::Mat dilateElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), cv::Point(-1, -1));
		cv::dilate(imgDifference, imgDifference, dilateElement, cv::Point(-1, -1), 2);

		threshold(imgDifference, imgThresh, thresh, 255.0, CV_THRESH_BINARY);
		

		imshow("imgThresh", imgThresh);
		centers.clear();
		/// Detect edges using canny
		//Canny(imgThresh, imgThresh, thresh, thresh * 2, 3);
		//imshow("Canny", imgThresh);
		/*FInd contours*/
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
		cv::findContours(imgThresh, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
		
		/// Draw contours
		int blob_radius_thresh = 50;
		Mat drawing = Mat::zeros(imgThresh.size(), CV_8UC3);
		for (int i = 0; i< contours.size(); i++)
		{
			Scalar color = Colors[10%8];
			drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
			Point2f centroid;
			float r;
			cv::minEnclosingCircle(contours[i], centroid, r);
			/*if((int)r > blob_radius_thresh)
				centers.push_back(Point(centroid.x, centroid.y));*/
		}
		imshow("Contours", drawing);
		/*------------------------------------*/
		
		// Xử lý blob
		for (int i = 0; i < imgThresh.rows; i++)
		{
			for (int j = 0; j < imgThresh.cols; j++)
			{
				if (imgThresh.at<uchar>(i, j) == 255)
				{
					bool isIn = false;
					for (int k = 0; k < b.size(); k++)
					{
						if (b[k].isInBlob(Point(j, i)))
						{
							b[k].Add(Point(j, i));
							isIn = true;
							break;
						}
					}
					if (!isIn)
					{
						Blob n;
						n.Add(Point(j, i));
						b.push_back(n);
					}
				}
			}
		}
		
		
		for (int i = 0; i < b.size(); i++)
		{
			if (b[i].size < 1000 
				|| b[i].width > b[i].height)
				continue;
			Point center = Point(b[i].xcenter, b[i].ymax);
			centers.push_back(center);
			rectangle(imgFrame1, 
				Rect(b[i].xmin, b[i].ymin, b[i].xmax - b[i].xmin, b[i].ymax - b[i].ymin),
				Colors[10 % 8], 2);
		}
		
		if (centers.size()>0)
		{
			tracker.Update(centers);

			for (int i = 0; i<tracker.tracks.size(); i++)
			{
				if (tracker.tracks[i]->trace.size()>1)
				{
					for (int j = 0; j<tracker.tracks[i]->trace.size() - 1; j++)
					{
						Point p1 = Point(tracker.tracks[i]->trace[j].x, tracker.tracks[i]->trace[j].y);
						Point p2 = Point(tracker.tracks[i]->trace[j + 1].x, tracker.tracks[i]->trace[j + 1].y);
				
						line(imgFrame1,
							p1,
							p2, 
							Colors[tracker.tracks[i]->track_id % 8], 2, CV_AA);
					}
				}
			}
		}
		imshow("Input", imgFrame1);
		imgFrame1 = imgFrame2.clone();
		b.clear();
		waitKey(30);
		if ((capVid.get(CV_CAP_PROP_POS_FRAMES) + 1) < capVid.get(CV_CAP_PROP_FRAME_COUNT)) {
			capVid.read(imgFrame2);
		}
		else {
			std::cout << "end of video\n";
			break;
		}

		EscKeyCheck = cv::waitKey(1);
	}

	if (EscKeyCheck != 27) {               // if the user did not press esc (i.e. we reached the end of the video)
		cv::waitKey(0);                         // hold the windows open to allow the "end of video" message to show
	}

	return 0;
}
