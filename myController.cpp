#include "myCamController.h"

myCamController::myCamController(){
	m_nWinWidth = 640;
	m_nWinHeight= 480;
	m_nRectW = 320;
	m_nRectH = 320;
	m_nCurrentLabel = 0;

	m_bTrained = false;
	m_bGetTrainImage = false;
	m_nNumPerBatch = 100;
	m_nNumCurrent = 0;
	m_nNumOfClasses = 5;
	m_nNumOfFeature = 1764;

	m_gestureDict.insert(std::pair<int,string>(1, "FROWARD"));
	m_gestureDict.insert(std::pair<int,string>(2, "LEFT"));
	m_gestureDict.insert(std::pair<int,string>(3, "RIGHT"));
	m_gestureDict.insert(std::pair<int,string>(4, "BACKWARD"));
	m_gestureDict.insert(std::pair<int,string>(5, "STOP"));
	for (int i=0;i<m_nNumOfClasses;i++)
	{
		vector<Mat*> vec;
		m_gestureImgVector.push_back(vec);
	}

	m_rectBox.x = 0;
	m_rectBox.y = 0;
	m_rectBox.height = m_nRectH;
	m_rectBox.width = m_nRectW;

	for (int i=0;i<m_nNumOfClasses;i++)
	{
		m_confidenceVector.push_back(0.0);
	}

	//parameters for HOGDescriptor
	m_winSize = Size(64, 64);
	m_blockSize = Size(16, 16);
	m_blockStride = Size(8, 8);
	m_cellSize = Size(8, 8);
	m_nBins = 9;
	m_winStride = Size(8, 16);
	m_padding = Size(0, 0);

	m_descriptor =  new HOGDescriptor(m_winSize, m_blockSize, m_blockStride, m_cellSize, m_nBins);
	m_classifier = new MySvm();
	m_cap = new VideoCapture();
	m_classifier->setNumOfFeatures(m_nNumOfFeature);
}

myCamController::~myCamController(){
	delete m_descriptor;
	delete m_cap;
}

void myCamController::draw_box(Mat& mat, CvRect rect){
	cv::rectangle(mat,cv::Point(rect.x, rect.y),cv::Point(rect.x + rect.width, rect.y + rect.height), cv::Scalar(255,255,0));
}

vector<float> myCamController::gradientsForImage(Mat* mat){

	vector<float> descriptors;
	Mat rectCaptured(*mat,m_rectBox);

	//turn source mat into gray mat
	Mat grayMat;
	cv::cvtColor(rectCaptured,grayMat,COLOR_RGB2GRAY); 
	vector<cv::Point> locations;

	//resize the gray image to smaller one, reduce the number of features
	Mat resized_mat;
	cv::resize(grayMat,resized_mat,cv::Size(64,64));

	m_descriptor->compute(resized_mat,descriptors, m_winStride, m_padding, locations);

	return descriptors;
}

vector<float> myCamController::gradientsForImage(Mat* mat, int label){
	m_gestureImgVector[label].push_back(mat);
	return gradientsForImage(mat);
}

void myCamController::openCamera(){
	m_cap->open(0); // open the video camera no. 0

	m_cap->set(CV_CAP_PROP_FRAME_WIDTH, m_nWinHeight);
	m_cap->set(CV_CAP_PROP_FRAME_HEIGHT, m_nWinHeight);

	namedWindow("Main Window"); //default window
}

//run single cam stream input, could be called in other loops
int myCamController::runTesting(){

	Sample* temp = NULL;
	Mat frame;

	m_cap->read(frame); // read a new frame from video

	if (!m_cap->isOpened())  // if not success, exit program
	{
		cout << "Cannot open the video cam" << endl;
		return -1;
	}

	draw_box(frame,m_rectBox);

	double max_prob_label = 0;
	if (m_bTrained)
	{
		temp = new Sample(gradientsForImage(&frame));
		max_prob_label = m_classifier->predict(temp,m_confidenceVector);
	}

	int index = 0;
	std::vector<double>::iterator iter_predict = m_confidenceVector.begin();
	while (iter_predict != m_confidenceVector.end())
	{
		stringstream ss_class;
		ss_class<<*iter_predict;
		cv::putText(frame, m_gestureDict.find(index + 1)->second + " : "+ss_class.str(),cv::Point(m_nRectW + 50, index * 50 + 50), FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(255,255,255,0),2,8,false);

		if (max_prob_label == index)
		{
			cv::putText(frame, m_gestureDict.find(index + 1)->second + " : "+ss_class.str(),cv::Point(m_nRectW + 50, index * 50 + 50), FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(255,0,0,0),2,8,false);
		}

		index++;
		iter_predict++;
	}

	imshow("Main Window", frame); //show the frame in "MyVideo" window
	int key = waitKey(1);

	if (temp)
	{
		delete temp;
	}

	return max_prob_label;
}

//run cam stream in loop
void myCamController::runTraining(){

	openCamera();

	if (!m_cap->isOpened())  // if not success, exit program
	{
		cout << "Cannot open the video cam" << endl;
		return;
	}

	namedWindow("Main Window"); //default window
	//for temporary sample
	Sample* temp = NULL;
	bool flag = true;

	while (flag)
	{
		Mat frame;
		
		m_cap->read(frame); // read a new frame from video

		if (frame.empty())
			continue;

		draw_box(frame,m_rectBox);

		stringstream ss_label;
		ss_label<<m_nCurrentLabel + 1;
		cv::putText(frame, "Current class: " + ss_label.str(),cv::Point(m_nRectW + 50,50), FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(255,255,255,0),2,8,false);

		int index = 0;
		std::vector<double>::iterator iter_predict = m_confidenceVector.begin();
		while (iter_predict != m_confidenceVector.end())
		{
			stringstream ss_class;
			ss_class<<*iter_predict;
			cv::putText(frame, m_gestureDict.find(index + 1)->second + " : "+ss_class.str(),cv::Point(m_nRectW + 50, index * 30 + 150), FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(255,255,255,0),2,8,false);
			index++;
			iter_predict++;
		}

		cv::putText(frame,"key 1,2,3,4,5 for switching classes",cv::Point(50, m_nWinHeight  - 20), FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(255,255,255,0),2,8,false);
		cv::putText(frame,"key y for adding samples to current class",cv::Point(50, m_nWinHeight  - 50), FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(255,255,255,0),2,8,false);
		cv::putText(frame,"key t for training",cv::Point(50, m_nWinHeight  - 80), FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(255,255,255,0),2,8,false);
		cv::putText(frame,"key ESC for exit and begin testing",cv::Point(50, m_nWinHeight  - 110), FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(255,255,255,0),2,8,false);

		imshow("Main Window", frame); //show the frame in "MyVideo" window
		int key = waitKey(1);

		if (m_bGetTrainImage && !frame.empty())
		{
			m_nNumCurrent++;

			m_classifier->addTraningSample(new Sample(gradientsForImage( &frame, m_nCurrentLabel ), m_nCurrentLabel) );
			cout<<"adding new Sample "<<m_nNumCurrent<<" ......"<<endl;

			//output text
			stringstream ss_count;

			ss_count<<m_nNumCurrent;
			cv::putText(frame, "adding new Sample " + ss_count.str(),cv::Point(m_nRectW + 50,100), FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(255,255,255,0),2,8,false);
			imshow("Main Window", frame); 
			waitKey(1);

			if (m_nNumCurrent >= m_nNumPerBatch)
			{
				m_nNumCurrent = 0;
				m_bGetTrainImage = false;
			}

			//ignoring other commands
			continue;
		}

		switch (key)
		{
		case 49:
		case 50:
		case 51:
		case 52:
		case 53:
		case 54:
			//"1","2","3","4","5" for entering different class
			m_nCurrentLabel = key - 49;
			cout << "Go to "<<m_gestureDict.find(m_nCurrentLabel + 1)->second<<" Class"<< endl;
			break;
		case 27:
			//"ESC" for exiting program
			cout << "Exit the program" << endl;
			flag = false;
			break; 
		case 121:
			//"y" for starting adding images for training
			m_bGetTrainImage = true;
			break;
		case 116:
			//"t" for training sample images
			cout<<"Training......"<<endl;

			cv::putText(frame, "Training......",cv::Point(m_nRectW + 50,100), FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(255,255,255,0),2,8,false);
			imshow("Main Window", frame); 
			waitKey(1);

			m_classifier->train();
			m_bTrained = true;
			break;
		case 112:
			//'p' for predict single frame
			if (m_bTrained)
			{
				temp = new Sample(gradientsForImage(&frame));
				m_classifier->predict(temp,m_confidenceVector);
				std::vector<double>::iterator iter = m_confidenceVector.begin();
				cout<<"Prediction...."<<endl;

				while (iter != m_confidenceVector.end())
				{
					cout<<*iter<<endl;
					iter++;
				}

				delete temp;
			}
			break;
		default:
			break;
		}
	}
}