#ifndef _OBJECTS_H
#define _OBJECTS_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

int startCalibration(VideoCapture *cap);
int inputToTrain(Mat input);
double classify(Mat input);

//Number of frames to iterate over for proper calibration. 
#define FRAMES_FOR_CALIBRATION 10
#define FRAMES_TO_SKIP 5
#define FRAMES_TO_SAVE 10

#define SECTOR_SIZE 2
#define MIN_DEVIATION 50
#define BRIGHTNESS_THRESHOLD 100

#define KERNEL_SIZE 3
#define NUMBER_OF_FILTERS 3
#define SIZE_FOR_POOL 2
#define MIN_RESPONSE_TO_CNN 0.90
#define IMAGE_SIZE_FOR_TRAINING 7
#endif