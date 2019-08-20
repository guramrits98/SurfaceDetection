#include "stdafx.h"
#include "Objects.h"

/*
This program implements a simple convolutional neural net to classify the background
and not background. 
To use :
1. Call inputToTrain() #this will train the initial CNN model to detect the background. 
*/

int const CHANNELS = 3;
int filters[NUMBER_OF_FILTERS][KERNEL_SIZE][KERNEL_SIZE];
/*The image which is made after the first convolution will be saved here.
The first parameter of the array refers to the filter that was applied.
The second is the channel on which that filter was applied.*/
Mat conv1[CHANNELS][NUMBER_OF_FILTERS];
Mat pooled[CHANNELS][NUMBER_OF_FILTERS];
std::vector<std::vector<double > > pooledWeights[CHANNELS][NUMBER_OF_FILTERS];

/*
This will create all the required filters and will return the number of filters that were created
************************************LOOK AT HOW TO MAKE THIS MORE GENERALIZED******USE VECTORS IF POSSIBLE
*/
//Make sure to write down as many filters that have been specified as have been specified in the Objects.h file
int createFilters()
{
	//Create the horizontal, vertical and both the diagonal filters
	for (int i = 0; i < KERNEL_SIZE; i++)
	{
		for (int j = 0; j < KERNEL_SIZE; j++)
		{
			//For horizontal
			if (i % 2 == 1)
			{
				filters[0][i][j] = 1;
			}
			else
			{
				filters[0][i][j] = -1;
			}
			//A simple filter that keeps everything constant
			//filters[1][i][j] = 1;
			//For vertical
			if (j % 2 == 1)
			{
				filters[1][i][j] = 1;
				//filters[2][i][j] = 1;
			}
			else
			{
				filters[1][i][j] = -1;
				//filters[2][i][j] = -1;
			}
			//For diagonal starting at left top
			if ((i == j) || (i + j) == KERNEL_SIZE - 1)
			{
				filters[2][i][j] = 1;
			}
			else
			{
				filters[2][i][j] = -1;
			}
		}
	}
	return NUMBER_OF_FILTERS;
}

/*
Multiplies a Mat object with a filter. 
@param patch: This is the Mat object on which the filter needs to be applied. 
				The size of this should be KERNEL_SIZE * KERNEL_SIZE;
@param filter: square matrix that is to be used on the image. 
*/
int matrixMultiply(Mat patch, int filter[KERNEL_SIZE][KERNEL_SIZE])
{
	double average = 0;
	for (int i = 0; i < KERNEL_SIZE; i++)
	{
		for (int j = 0; j < KERNEL_SIZE; j++)
		{
			average += patch.at<uchar>(i, j) * filter[j][i];
		}
	}
	average /= (double)KERNEL_SIZE;
	if (average < 0)
	{
		average *= -1;
	}
	//if (average < 100)
	//{
	//	average = 0;
	//}
	return (int)average;
}

/*
@param channels: The channels of an image to which the filters need to be applied. 
Applies all the filters to the channels and saves them in conv1.
@return: returns 1 if it was executed as expected. 
*/
int applyFilters1(Mat channels[])
{
	//Image cropping parameters
	Rect roi;
	roi.width = KERNEL_SIZE;
	roi.height = KERNEL_SIZE;
	roi.x = 0;
	roi.y = 0; 

	//This applies all the filters to the channels of the image. 
	for (int i = 0; i < NUMBER_OF_FILTERS; i++)
	{
		Mat convoluted = Mat::zeros(channels[0].rows - ((KERNEL_SIZE / 2) + 1), channels[0].cols - ((KERNEL_SIZE / 2) + 1), CV_8UC3);
		split(convoluted, conv1[i]);
	}

	//Cropped image that will be sent to apply the filter. 
	Mat imgCropped[3];

	//Iterate the filter over the given rows of the image
	for (int i = 0; i < channels[0].rows - KERNEL_SIZE + 1; i++)
	{
		//Iterate the filter over teh given cols of the image
		for (int j = 0; j < channels[0].cols - KERNEL_SIZE + 1; j++)
		{
			//set cropping parameters. 
			roi.x = j;
			roi.y = i;
			//crop the channels
			for (int k = 0; k < CHANNELS; k++)
			{
				imgCropped[k] = channels[k](roi);
			}
			//construct filtered images using all the specified filters
			for (int k = 0; k < CHANNELS; k++)
			{
				for (int l = 0; l < NUMBER_OF_FILTERS; l++)
				{
					//cout << "it worked till here" << i << "with col" << j << endl;
					conv1[k][l].at<uchar>(i, j) = matrixMultiply(imgCropped[k], filters[l]);
				}
			}
		}
	}
	return 1;
}

/*
This will pool the pixels of the specified image.
@param createVector: Pass in 1 if it is the first time training the program, else pass in 0.
@return returns 1 if it executed correctly. 
*/
int poolFilteredImages(int createVector)
{
	for (int i = 0; i < CHANNELS; i++)
	{
		for (int j = 0; j < NUMBER_OF_FILTERS; j++)
		{

			Mat img = conv1[i][j];
			if (createVector)
			{
				vector<double> imgCols(img.cols / SIZE_FOR_POOL);
				vector<vector<double> > imgRows((img.rows / SIZE_FOR_POOL) * (img.cols / SIZE_FOR_POOL));
				for (int k = 0; k < img.rows / SIZE_FOR_POOL; k++)
				{
					imgRows.push_back(imgCols);
					for (int l = 0; l < img.cols / SIZE_FOR_POOL; l++)
					{
						imgRows[k].push_back(0);
					}
				}
				pooledWeights[i][j] = imgRows;
			}
			Mat pooledImg = Mat::zeros(img.rows / SIZE_FOR_POOL, img.cols / SIZE_FOR_POOL, CV_8UC1);
			for (int k = 0; k < img.rows / SIZE_FOR_POOL; k++)
			{
				for (int l = 0; l < img.cols / SIZE_FOR_POOL; l++)
				{
					//Need to fix this part.................Do it later.
					int col = l * SIZE_FOR_POOL;
					int row = k * SIZE_FOR_POOL;
					int max1 = std::max((int)img.at<uchar>(row, col), (int)img.at<uchar>(row + 1, col + 1));
					int max2 = std::max((int)img.at<uchar>(row + 1, col), (int)img.at<uchar>(row, col + 1));
					max1 = std::max(max1, max2);
					pooledImg.at<uchar>(k, l) = max1;
				}
			}
			pooled[i][j] = pooledImg;
		}
	}
	return 1;
}

int findWeights() 
{
	for (int i = 0; i < CHANNELS; i++)
	{
		for (int j = 0; j < NUMBER_OF_FILTERS; j++)
		{
			for (int k = 0; k < (int)pooled[i][j].rows; k++)
			{
				for (int l = 0; l < (int)pooled[i][j].cols; l++)
				{
					pooledWeights[i][j].at(k).at(l) += (int)pooled[i][j].at<uchar>(k, l);
				}
			}
		}
	}
	return 1;
}

double applyWeights()
{
	double finalVote = 0;
	int highestPossibleVote = 0;
	for (int i = 0; i < CHANNELS; i++)
	{
		for (int j = 0; j < NUMBER_OF_FILTERS; j++)
		{
			for (int k = 0; k < (int)pooled[i][j].rows; k++)
			{
				for (int l = 0; l < (int)pooled[i][j].cols; l++)
				{
					double voteToAdd = 0;
					voteToAdd += (double)(pooled[i][j].at<uchar>(k, l)) - (double)pooledWeights[i][j][k][l];
					voteToAdd /= (double)255;
					if (voteToAdd < 0)
					{
						voteToAdd *= -1;
					}
					voteToAdd = 1 - voteToAdd;
					finalVote += voteToAdd;
					highestPossibleVote += 1;
					//cout << "Multiplying " << (double)pooledWeights[i][j].at(k).at(l) << "with " << (double)pooled[i][j].at<uchar>(k, l) << endl;
				}
			}
		}
	}
	//cout << "The final vote is " << finalVote << " and the maximum possible weight is " << highestPossibleVote << endl;
	double prob = finalVote / (double)highestPossibleVote;
	//cout << "The final probability is " << prob << endl;
	return prob;
}

/*
Send in an image size that it was trained with. In this case it should be a 7 * 7
*/
extern double classify(Mat input)
{
	Mat channels[CHANNELS];
	split(input, channels);
	applyFilters1(channels);
	poolFilteredImages(0);
	return applyWeights();
}

/*
@param input: 3 channel image
This image should consist only of the background. 
*/
extern int inputToTrain(Mat input)
{
	cout << "The number of rows is " << input.rows << endl;
	cout << "The number of cols is " << input.cols << endl;
	Mat channels[CHANNELS];
	//This will create all the required filters and save them in an array called filters. 
	createFilters();
	Rect roi;
	roi.height = IMAGE_SIZE_FOR_TRAINING;
	roi.width = IMAGE_SIZE_FOR_TRAINING;
	int numOfImages = 0;
	//channels now has all greyscale images. 
	for (int i = 0; i < input.rows - IMAGE_SIZE_FOR_TRAINING + 1; i++)
	{
		for (int j = 0; j < input.cols - IMAGE_SIZE_FOR_TRAINING + 1; j++)
		{
			roi.x = j;	//Refers to the cols.
			roi.y = i;	//Refers to the rows. 
			Mat cropped = input(roi);
			split(cropped, channels);
			applyFilters1(channels);
			if (i == 0 && j == 0)
			{
				poolFilteredImages(1);
			}
			else
			{
				poolFilteredImages(0);
			}
			findWeights();
			numOfImages += 1;
		}
	}
	cout << "I am done with the first part" << endl;
	for (int i = 0; i < CHANNELS; i++)
	{
		for (int j = 0; j < NUMBER_OF_FILTERS; j++)
		{
			for (int k = 0; k < pooled[i][j].rows; k++)
			{
				for (int l = 0; l < pooled[i][j].cols; l++)
				{
					//This will now save the average value of that pixel
					pooledWeights[i][j].at(k).at(l) /= (double)numOfImages;
					cout << "The found average is " << pooledWeights[i][j][k][l] << endl;
				}
			}
		}
	}
	//namedWindow("one", CV_WINDOW_AUTOSIZE);
	//namedWindow("two", CV_WINDOW_AUTOSIZE);
	//namedWindow("three", CV_WINDOW_AUTOSIZE);
	//cout << "Looking at different image with same filters" << endl;
	//imshow("one", conv1[0][0]);
	//imshow("two", conv1[1][0]);
	//imshow("three", conv1[2][0]);
	return 1;
}