#include "classifier.h"


Classifer::Classifer(){
	m_nNumOfFeatures = 2;
	m_nNumOfSamples = 0;
}

void Classifer::addTraningSample(vector<float>& vecFeature, int label){
	addTraningSample(new Sample(vecFeature,label));
}

void Classifer::addTraningSample(Sample* sample){
	m_vTrainingSamples.push_back(sample);
	m_nNumOfSamples = m_vTrainingSamples.size();
}

void Classifer::clear(){
	std::vector<Sample*>::iterator iter = m_vTrainingSamples.begin();
	while(iter != m_vTrainingSamples.end())
	{
		delete *iter;
		iter++;
	}
	m_vTrainingSamples.clear();
}

void Classifer::setNumOfFeatures(size_t numOfFeatures){
	m_nNumOfFeatures = numOfFeatures;
}
