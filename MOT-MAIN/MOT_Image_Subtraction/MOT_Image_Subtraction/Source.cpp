#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include "opencv2/opencv.hpp"
#include "opencv2\video\background_segm.hpp"
#include <opencv/cv.h>
#include<iostream>
#include<vector>
#include "Ctracker.h"
#include "Blob.h"
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

void drawAndShowContours(cv::Size imageSize, std::vector<std::vector<cv::Point> > contours, std::string strImageName);
void drawAndShowContours(cv::Size imageSize, std::vector<std::vector<cv::Point> > contours, std::string strImageName)
{
	cv::Mat image(imageSize, CV_8UC3, Scalar(0, 0, 0));

	cv::drawContours(image, contours, -1, Scalar(255, 255, 255), -1);

	cv::imshow(strImageName, image);
}

// Lớp chứa tần số giá trị của các pixel
class PixelHistogram
{
public:
	Point coor;
	vector<int> histogram;
	uchar Id;

	PixelHistogram() { coor.x = coor.y = 0; vector<int> tmp(256, 0); histogram = tmp; }
	PixelHistogram(float x, float y) { coor.x = x; coor.y = y; vector<int> tmp(256, 0); histogram = tmp; }
	~PixelHistogram() {}
};

int main()
{
	// name of the video
	char *fn = "in.avi";
	CTracker tracker(0.2, 0.5, 60.0, 10, 30);
	//used to store points detected.
	vector<Point2f> centers;
	////store bounding boxes
	vector<Blob> b;
	
	char EscKeyCheck = 0;
	int frameCount = 1;
	/*CREATE BACKGROUND*/
	VideoCapture capVid1(fn);

	if (!capVid1.isOpened())
		return 1;
	Mat tmpFrame;
	capVid1.read(tmpFrame); // read a frame
	cvtColor(tmpFrame, tmpFrame, CV_BGR2GRAY); // convert to Gray
	vector<vector<PixelHistogram>> arr(tmpFrame.rows, vector<PixelHistogram>(tmpFrame.cols)); // pixel histogram of each frame
	for (int i = 0; i < tmpFrame.rows; i++)
	{
		for (int j = 0; j < tmpFrame.cols; j++)
		{
			PixelHistogram tmp(i, j);
			tmp.histogram[tmpFrame.at<uchar>(i, j)] += 1;
			arr[i][j] = tmp;
		}
	}

	frameCount = 0;
	// Lập bảng thống kê các giá trị mức xám tại các pixel trong video
	while (capVid1.isOpened())
	{
		frameCount++;
		// Đọc frame và lưu Frame
		if ((capVid1.get(CV_CAP_PROP_POS_FRAMES) + 1) < capVid1.get(CV_CAP_PROP_FRAME_COUNT) && frameCount < 300) {
			capVid1.read(tmpFrame);
		}
		else {
			std::cout << "end of video\n";
			break;
		}

		/*if (frameCount < 300)
		{*/
		cvtColor(tmpFrame, tmpFrame, CV_BGR2GRAY);
		// Duyệt hết frame và tính tần suất của các mức xám tại các pixel
		for (int i = 0; i < tmpFrame.rows; i++)
		{
			for (int j = 0; j < tmpFrame.cols; j++)
			{
				arr[i][j].histogram[tmpFrame.at<uchar>(i, j)] += 1;
			}
		}
		//}
	}

	// Ma trận ảnh result cho biết ảnh background sau khi thống kê
	Mat result = Mat::zeros(tmpFrame.size(), CV_8UC1);
	// Tạo ảnh result
	for (int i = 0; i < tmpFrame.rows; i++)
	{
		for (int j = 0; j < tmpFrame.cols; j++)
		{
			int max = 0;
			int value = 0;
			for (int k = 0; k < 256; k++)
			{
				int t = arr[i][j].histogram[k];
				if (max < t)
				{
					max = t;
					value = k;
				}
			}
			/*if (value == 0)
			{
			if (i > 0 && j > 0)
			value = result.at<uchar>(i - 1, j - 1);
			}*/
			result.at<uchar>(i, j) = value;
		}
	}
	capVid1.release();
	imshow("Background", result);
	waitKey(0);
	/*OBJECT DETECTION AND TRACKING*/
	VideoCapture capVid(fn);

	Mat imgFrame1;
	capVid.read(imgFrame1); // read a frame
	while (capVid.isOpened() && EscKeyCheck != 27)
	{
		Mat imgDifference, imgThresh;
		Mat imgFrame1Clone = imgFrame1.clone();
		cvtColor(imgFrame1Clone, imgFrame1Clone, CV_BGR2GRAY);

		int thresh = 50;
		absdiff(imgFrame1Clone, result.clone(), imgDifference);

		threshold(imgDifference, imgThresh, thresh, 255.0, CV_THRESH_BINARY);
		imshow("imgThresh", imgThresh); //show image thresholded
		
		
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
			if (b[i].size < 100 || (b[i].xmax - b[i].xmin < 10) || (b[i].ymax - b[i].ymin < 10))
			{
				b.erase(b.begin() + i, b.begin() + i + 1);
				i--;
				continue;
			}
		}
		for (int i = 0; i < b.size(); i++)
		{
			for (int j = i + 1; j < b.size(); j++)
			{
				// blob j contains blob i
				if (b[i].xmax <= b[j].xmax && b[i].xmin >= b[j].xmin
					&& b[i].ymax <= b[j].ymax && b[i].ymin >= b[j].ymin)
				{
					b.erase(b.begin() + i, b.begin() + i + 1);
					i--;
					break;
				}

				// blob i contains blob j
				if (b[j].xmax <= b[i].xmax && b[j].xmin >= b[i].xmin
					&& b[j].ymax <= b[i].ymax && b[j].ymin >= b[i].ymin)
				{
					b.erase(b.begin() + j, b.begin() + j + 1);
					break;
				}
			}
		}
		//
		centers.clear();// centers must be clear before new centers is pushed
		for (int i = 0; i < b.size(); i++)
		{
			if (b[i].size < 1000
				|| b[i].width > b[i].height)
				continue;
			rectangle(imgFrame1, Rect(b[i].xmin, b[i].ymin, b[i].xmax - b[i].xmin, b[i].ymax - b[i].ymin), Scalar(0, 255, 0), 2);
			Point center = Point(b[i].xcenter, b[i].ymax);
			centers.push_back(center);
		}

		/*for (int i = 0; i < trace_point.size(); i++)
		{
		rectangle(imgFrame1, Rect(trace_point[i].x, trace_point[i].y, 1, 1), Scalar(255, 0, 0), 2);
		}*/
		b.clear();
		imshow("Input", imgFrame1);
		/*------------------------------------*/
		
		
		
		if (centers.size()>0)
		{
			tracker.Update(centers);

			for (int i = 0; i<tracker.tracks.size(); i++)
			{
				if (tracker.tracks[i]->trace.size() > 1)
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
		b.clear();
		waitKey(30);
		if ((capVid.get(CV_CAP_PROP_POS_FRAMES) + 1) < capVid.get(CV_CAP_PROP_FRAME_COUNT)) {
			capVid.read(imgFrame1);
		}
		else {
			std::cout << "end of video\n";
			break;
		}

		EscKeyCheck = cv::waitKey(1);
	}

	if (EscKeyCheck != 27)
	{               // if the user did not press esc (i.e. we reached the end of the video)
		cv::waitKey(0);                         // hold the windows open to allow the "end of video" message to show
	}

	return 0;
}
