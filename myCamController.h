#ifndef  _MYCAMCONTROLLER_H_
#define _MYCAMCONTROLLER_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "mySvm.h"
#include "svm.h"
#include <opencv/cxcore.h>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv/cv.h>

using namespace cv;

class myCamController
{
public:
	void runTraining();
	int runTesting();
	void openCamera();//open video camera
	vector<float> gradientsForImage(Mat* mat);
	vector<float> gradientsForImage(Mat* mat, int label);
	void draw_box(Mat& mat, CvRect rect);
	myCamController();
	virtual ~myCamController();

private:
	MySvm* m_classifier;
	VideoCapture* m_cap;

	//width and height for window
	int m_nWinWidth;
	int m_nWinHeight;
	//width and height for train window
	int m_nRectW;
	int m_nRectH;
	CvRect m_rectBox;

	//label for class
	int m_nCurrentLabel;
	//flag indicates whether data being trained
	bool m_bTrained;
	//start getting images for training
	bool m_bGetTrainImage;
	//number of images per batch for training
	int m_nNumPerBatch;
	//current number of image captured
	int m_nNumCurrent;
	//number of classes of the gesture
	int m_nNumOfClasses;
	//map the label to the name of the class
	std::map<int, string> m_gestureDict;
	//set this value based on the HOGDescriptor parameter
	size_t m_nNumOfFeature;
	//store all the input images, under different gesture classes
	vector<vector<Mat*>> m_gestureImgVector;
	//vector that stores the estimate results
	vector<double> m_confidenceVector;

	HOGDescriptor* m_descriptor;
	//HOGDescriptor parameters
	Size m_winSize;
	Size m_blockSize;
	Size m_blockStride;
	Size m_cellSize;
	int m_nBins;
	Size m_winStride;
	Size m_padding;
};


#endif