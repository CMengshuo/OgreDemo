#ifndef _MYSVM_H_
#define  _MYSVM_H_

#include "classifier.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <imgproc.hpp>
#include "svm.h"

#include <string>

using namespace std;

class MySvm: public Classifer{
protected:
	svm_parameter* m_params;
	svm_model* m_model;
	svm_problem* m_problem;

	//if a model has been loaded, used this mark to free svmproblem memory
	bool m_bModelLoaded;
	
public:
	MySvm();
	virtual ~MySvm();
	void doubleToFloat(vector<float>& vecDest, vector<double>& vecSrc);

	void setKernalParameters(int svmType, int kernelTye, int degree, float gamma, int c);
	void loadModel(std::string filename, size_t numOfFeature);
	void saveModel(std::string filename);

	//for prediction using a confidence vector
	double predict(vector<float>& srcVec, vector<double>& estimateVec);
	double predict(Sample *sample, vector<double>& confidence);

	void setSampleData(vector<int> labels, vector<vector<float>>& sampleVector);
	void train();
};

#endif