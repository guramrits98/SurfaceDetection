// ObjectDetectionELCANO.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Objects.h"

//AVG_VALUES calibrated;

/*
Just a helper method for quicksort. Will not be used elsewhere. 
*/
int partition(int *a, int s, int e)
{
	int piviot = a[e];
	int pind = s;
	int i, t;
	for (i = s;i<e;i++)
	{
		if (a[i] <= piviot)
		{
			t = a[i];
			a[i] = a[pind];
			a[pind] = t;
			pind++;
		}
	}

	t = a[e];
	a[e] = a[pind];
	a[pind] = t;
	return pind;
}

/*
quicksort for an array of integers
*/
void quicksort(int *a, int s, int e)
{
	if (s<e)
	{
		int pind = partition(a, s, e);
		quicksort(a, s, pind - 1);
		quicksort(a, pind + 1, e);
	}
}

/*
@param arr[]: Array of integers whose median needs to be found. 
@param length: The length of the array passed. 
@return: Median value of an array. 
*/
int findMiddleValue(int arr[], int length)
{
	//int length = sizeof(arr);
	//cout << "The length is " << length << endl;
	//cout << "Length of array is " << sizeof(arr)/sizeof(arr[0]) << endl;
	//First copy this array
	int* newArr = new int[length];
	for (int i = 0; i < length; i++)
	{
		newArr[i] = arr[i];
	}
	quicksort(newArr, 0, length);
	int middleVal = 0;
	for (int i = 0; i < 5; i++)
	{
		middleVal += newArr[(length / 2) - 3 + i];
	}
	middleVal /= 5;
	return newArr[(length/2)];
}

/*
Takes in an image and returns a greyscale image with the obstacles marked in white. 
@param img:	The image in which the obstacles need to be found. This should be in HSV format.
@param numOfSectors: This is the number of sectors that the every row is divided into.
@return Mat: This will be a gryscale image. The obstacles will be marked by white, rest everything will be black. 
*/
Mat findObstacles(Mat img, int numOfSectors)
{
	Mat grey;
	grey = Mat:: zeros(img.rows, img.cols, CV_8UC1);

	//Split the image into its 3 channels.
	Mat channels[3];
	split(img, channels);

	//Use this array 2D array to store the values of the averages of each sector in the 3 channels.
	int** arr = new int*[3];
	for (int i = 0; i < 3; i++)
		arr[i] = new int[numOfSectors];

	//Iterate over the row of images.
	for (int row = 0; row < img.rows; row++)
	{
		//Use this array 2D array to store the values of the averages of each sector in the 3 channels.
		int** arr = new int*[3];
		for (int i = 0; i < 3; i++)
			arr[i] = new int[numOfSectors];

		int Avg00 = 0;
		int Avg01 = 0;
		int Avg02 = 0;


		//Iterate over every sector to find the average values of each sector and store them in arr.
		for (int sect = 0; sect < numOfSectors; sect++)
		{
			int Avg0 = 0;
			int Avg1 = 0;
			int Avg2 = 0;

			int AvgM0[SECTOR_SIZE];
			int AvgM1[SECTOR_SIZE];
			int AvgM2[SECTOR_SIZE];

			int pixelNo = SECTOR_SIZE * sect;
			//Go through every pixel value in that sector and find the average of the three channels. 
			for (int pix = 0; pix < SECTOR_SIZE; pix++)
			{
				//Save the pixel values of each sector into an array
				AvgM0[pix] = channels[0].at<uchar>(row, pixelNo + pix);
				AvgM1[pix] = channels[1].at<uchar>(row, pixelNo + pix);
				AvgM2[pix] = channels[2].at<uchar>(row, pixelNo + pix);

				Avg0 += channels[0].at<uchar>(row, pixelNo + pix);
				Avg1 += channels[1].at<uchar>(row, pixelNo + pix);
				Avg2 += channels[2].at<uchar>(row, pixelNo + pix);
			}

			//Find median on the sector.
			Avg00 = findMiddleValue(AvgM0, SECTOR_SIZE);
			Avg01 = findMiddleValue(AvgM1, SECTOR_SIZE);
			Avg02 = findMiddleValue(AvgM2, SECTOR_SIZE);

			Avg0 /= SECTOR_SIZE;
			Avg1 /= SECTOR_SIZE;
			Avg2 /= SECTOR_SIZE;

			//Save the median of that sector.
			arr[0][sect] = Avg00;
			arr[1][sect] = Avg01;
			arr[2][sect] = Avg02;
		}

		int Avg0 = findMiddleValue(arr[0], numOfSectors);
		int Avg1 = findMiddleValue(arr[1], numOfSectors);
		int Avg2 = findMiddleValue(arr[2], numOfSectors);

		for (int sect = 0; sect < numOfSectors; sect++)
		{
			arr[0][sect] -= Avg0;
			arr[1][sect] -= Avg1;
			arr[2][sect] -= Avg2;
		}

		bool prevSectorMarked = false;
		for (int sect = 0; sect < numOfSectors; sect++)
		{
			int brightness = 0;
			if (arr[0][sect] > MIN_DEVIATION || arr[0][sect] < -MIN_DEVIATION
				|| arr[1][sect] > MIN_DEVIATION || arr[1][sect] < -MIN_DEVIATION
				|| arr[2][sect] > MIN_DEVIATION || arr[2][sect] < -MIN_DEVIATION)
			{
				int sector = sect * SECTOR_SIZE;
				//peek into that sector
				for (int j = 0; j < SECTOR_SIZE; j++)
				{
					//If it is considered as an object in any one of the channels, mark it as an object.
					if (channels[0].at<uchar>(row, sector + j) - Avg0 > MIN_DEVIATION || channels[0].at<uchar>(row, sector + j) - Avg0 < -MIN_DEVIATION)
					{
						brightness += 70;
					}
					if (channels[1].at<uchar>(row, sector + j) - Avg1 > MIN_DEVIATION || channels[1].at<uchar>(row, sector + j) - Avg1 < -MIN_DEVIATION)
					{
						brightness += 70;
					}
					if (channels[2].at<uchar>(row, sector + j) - Avg2 > MIN_DEVIATION || channels[2].at<uchar>(row, sector + j) - Avg2 < -MIN_DEVIATION)
					{
						brightness += 70;
					}
					//If sector before it was marked, give it a boost.
					if (prevSectorMarked) {
						brightness += 25;
					}
					if (row != 0 && grey.at<uchar>(row - 1, sector + j) > 40)
					{
						brightness += 25;
					}
					if (brightness > BRIGHTNESS_THRESHOLD)
					{
						brightness = 255;
					}
					else
					{
						brightness = 0;
					}
					
					grey.at<uchar>(row, sector + j) = brightness;
					prevSectorMarked = true;
				}
			}
			else
			{
				prevSectorMarked = false;
			}
		}
		//deallocate the array
		for (int i = 0; i < 3; i++)
			delete[] arr[i];
		delete[] arr;
	}
	return grey;
}

int main()
{
	Mat frame;
	frame = imread("IMG_0468.jpg");
	cvtColor(frame, frame, CV_RGB2HSV);
	//This goes cols and rows
	Size size(frame.cols / 3, frame.rows / 3);
	Mat h[3];
	resize(frame, frame, size);
	split(frame, h);
	namedWindow("name", CV_WINDOW_AUTOSIZE);
	imshow("name", h[2]);
	medianBlur(frame, frame, 5);
	namedWindow("Initial Blurred", CV_WINDOW_AUTOSIZE);
	imshow("Initial Blurred", frame);
	cout << "started" << endl;
	Rect roi;
	roi.width = frame.cols;
	roi.height = frame.rows / 5;
	roi.x = 0;
	roi.y = 4 * frame.rows / 5;
	cout << "gonna crop" << endl;
	Mat testFrame = frame(roi);
	roi.width = IMAGE_SIZE_FOR_TRAINING;
	roi.height = IMAGE_SIZE_FOR_TRAINING;
	namedWindow("testFrame", CV_WINDOW_AUTOSIZE);
	imshow("testFrame", testFrame);
	inputToTrain(testFrame);
	Mat obstacles = Mat::zeros(frame.rows, frame.cols, CV_8UC1);
	cout << "The training is over, commencing the test on the image" << endl;
	for (int i = 0; i < frame.rows / IMAGE_SIZE_FOR_TRAINING - 1; i++)
	{
		for (int j = 0; j < frame.cols / IMAGE_SIZE_FOR_TRAINING - 1; j++)
		{
			roi.height = IMAGE_SIZE_FOR_TRAINING;
			roi.width = IMAGE_SIZE_FOR_TRAINING;
			roi.x = j * IMAGE_SIZE_FOR_TRAINING;
			roi.y = i * IMAGE_SIZE_FOR_TRAINING;
			//cout << "tried cropping at" << roi.x << "& " << roi.y << " but the frame size is " << frame.rows << " and " << frame.cols << endl;
			Mat cropped = frame(roi);
			//cout << "There was a problem in cropping" << endl;
			if (classify(cropped) > MIN_RESPONSE_TO_CNN)
			{
				for (int k = roi.y; k < roi.y + IMAGE_SIZE_FOR_TRAINING; k++)
				{
					for (int l = roi.x; l < roi.x + IMAGE_SIZE_FOR_TRAINING; l++)
					{
						obstacles.at<uchar>(k, l) = 255;
					}
				}
			}
		}
	}

	Mat st_elem = getStructuringElement(MORPH_RECT, Size(4, 4));
	dilate(obstacles, obstacles, st_elem);
	medianBlur(obstacles, obstacles, 5);
	//erode(obstacles, obstacles, st_elem);
	namedWindow("Obs", CV_WINDOW_AUTOSIZE);
	imshow("Obs", obstacles);
	/*cout << "The model was trained, now going to use the model" << endl;
	roi.width = frame.cols;
	roi.height = 7;
	roi.x = 0;
	roi.y = 0;
	namedWindow("Testing for negative feedback", CV_WINDOW_AUTOSIZE);
	imshow("Testing for negative feedback", frame(roi));
	roi.width = 7;
	roi.height = 7;
	while (1)
	{
		if (waitKey(30) == 27)
		{
			cout << "The following should give negative responses" << endl;
			break;
		}
	}
	for (int i = 0; i < frame.cols - IMAGE_SIZE_FOR_TRAINING - 8; i += 7)
	{
		roi.x = i;
		roi.y = 0;
		classify(frame(roi));
	}
	cout << "The top strip was analyzed" << endl;
	while (1)
	{
		if (waitKey(30) == 27)
		{
			cout << "The following should give positive responses" << endl;
			break;
		}
	}
	roi.height = 7;
	roi.width = frame.cols;
	roi.x = 0;
	roi.y = frame.rows - 8;
	namedWindow("Testing for positive feedback", CV_WINDOW_AUTOSIZE);
	imshow("Testing for positive feedback", frame(roi));
	roi.width = 7;
	roi.height = 7;
	for (int i = 0; i < frame.cols - IMAGE_SIZE_FOR_TRAINING - 8; i += 7)
	{
		roi.x = i;
		roi.y = frame.rows - 8;
		classify(frame(roi));
	}*/
	while (1)
	{
		if (waitKey(30) == 27)
		{
			cout << "Process Terminated" << endl;
			break;
		}
	}
	VideoCapture cap("Sample3.mp4");
	//Mat frame;
	//startCalibration(&cap);
	while (1) {
		if (frame.empty())
		{
			cout << "End of Video / Could not read anymore." << endl;
			return -1;
		}
		cout << "reading" << endl;
		//cout << "Image Loaded" << endl;
		//Scale down the image here. 
		//Size size(600, 400);
		Mat compressed = frame;
		//resize(frame, compressed, size);
		const int numOfSectors = (compressed.cols / SECTOR_SIZE);

		medianBlur(compressed, compressed, 5);
		cvtColor(compressed, compressed, CV_BGR2HSV);

		//Mat channel[3];
	//split(compressed, channel);
	//namedWindow("Actual Image", CV_WINDOW_AUTOSIZE);
	//namedWindow("Channel 1", CV_WINDOW_AUTOSIZE);
	//namedWindow("Channel 2", CV_WINDOW_AUTOSIZE);
	//namedWindow("Channel 3", CV_WINDOW_AUTOSIZE);
	//imshow("Actual Image", compressed);
	//imshow("Channel 1", channel[0]);
	//imshow("Channel 2", channel[1]);
	//imshow("Channel 3", channel[2]);

		//cout << "segrigation strarted" << endl;
		Mat st_elem = getStructuringElement(MORPH_RECT, Size(2, 2));
		compressed = findObstacles(compressed, numOfSectors);
		erode(compressed, compressed, st_elem);
		dilate(compressed, compressed, st_elem);
		erode(compressed, compressed, st_elem);
		//dilate(compressed, compressed, st_elem);
		////compressed = makeBoundingBoxes(compressed);
		namedWindow("RoadView", CV_WINDOW_AUTOSIZE);
		imshow("RoadView", compressed);
		if (waitKey(30) == 27) {
			cout << "Interrupted by user" << endl;
			break;
		}
		while (1)
		{
			if (waitKey(30) == 27)
			{
				cout << "Process Terminated" << endl;
				break;
			}
		}

		return 1;
	}
}