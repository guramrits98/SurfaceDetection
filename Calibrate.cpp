#include "stdafx.h"
#include "Objects.h"

//Mat subFromFirst[FRAMES_FOR_CALIBRATION];
//Mat iterativeSubs[FRAMES_FOR_CALIBRATION];
//Mat originals[FRAMES_FOR_CALIBRATION];

Mat subtract(Mat frame1, Mat frame2)
{
	return frame1 - frame2;
}

Mat canny(Mat src)
{
	Mat src_gray;
	Mat dst, detected_edges;

	int edgeThresh = 1;
	int lowThreshold = 50;
	int const max_lowThreshold = 100;
	int ratio = 3;
	int kernel_size = 3;
	char* window_name = "Edge Map";

	/// Convert the image to grayscale
	cvtColor(src, src_gray, CV_BGR2GRAY);

	/// Reduce noise with a kernel 3x3
	blur(src_gray, detected_edges, Size(5, 5));

	/// Canny detector
	Canny(detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size);

	return detected_edges;
}

/*
Looks at a predefined number of rows in the beginning of the image. 
It is assumed that this area being scanned has no obstacles. It is also assumed that
there is just one kind of surface in that area (For eg. just grass or just concrete)
*/
void generateTemplate(Mat frame) {
	int middle = frame.cols / 2;
	int outerBounds = middle / 10;
	int rowsToScan = frame.rows / 10;
	//Iterate over the ROI. It is the first 10% of rows in the image. 
	//5% of the cols have also been trimmed from teh side. 
	for (int i = outerBounds; i < middle; i++)
	{
		
	}
}

/*
Find the background in the image and save it. 
*/
extern int startCalibration(VideoCapture *cap) {
	/*
	Mat frame1;
	cap->read(frame1);
	Mat frame3 = frame1;
	for (int i = 0; i < FRAMES_FOR_CALIBRATION; i++) 
	{
		Mat frame2;
		for (int j = 0; j < FRAMES_TO_SKIP; j++)
		{
			cap->read(frame2);
		}
		originals[i] = frame2;
		subFromFirst[i] = subtract(frame1, frame2);
		iterativeSubs[i] = subtract(frame3, frame2);
		frame3 = frame2;
	}
	Mat frameFinal = subFromFirst[0];
	for (int i = 0; i < FRAMES_FOR_CALIBRATION - 1; i++)
	{
		frameFinal += subFromFirst[i + 1];
	}
	frameFinal /= FRAMES_FOR_CALIBRATION;
	for (int i = 0; i < FRAMES_FOR_CALIBRATION; i++)
	{
		namedWindow("subs", CV_WINDOW_AUTOSIZE);
		namedWindow("subs1", CV_WINDOW_AUTOSIZE);
		imshow("subs", frameFinal);
		imshow("subs1", subFromFirst[i]);
		while (1)
		{
			if (waitKey(30) == 27) break;
		}
	}
	return 1;
	*/
	Mat frame;
	cap->read(frame);
	blur(frame, frame, Size(5, 5));
	generateTemplate(frame);
	return 1;
}