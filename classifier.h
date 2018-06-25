#ifndef _CLASSIFER_H_
#define  _CLASSIFER_H_

#include"sample.h"

class Classifer{

protected:
	vector<Sample*> m_vTrainingSamples;
	size_t m_nNumOfFeatures;
	size_t m_nNumOfSamples;

public:
	Classifer();
	void addTraningSample(vector<float>& vecFeature, int label);
	void addTraningSample(Sample* sample);
	void clear();
	void setNumOfFeatures(size_t numOfFeatures);
	
	virtual double predict(Sample* sample){return 0;}
	virtual void train(){}
};

#endif