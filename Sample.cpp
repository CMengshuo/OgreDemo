#include "sample.h"


Sample::Sample(vector<float> vec ,int label){
	vector<float>::iterator iter = vec.begin();
	while (iter != vec.end())
	{
		m_vecFeature.push_back((double)(*iter));
		iter++;
	}

	m_nLabel = label;
}

Sample::Sample(vector<float> vec){
	vector<float>::iterator iter = vec.begin();
	while (iter != vec.end())
	{
		m_vecFeature.push_back((double)(*iter));
		iter++;
	}
	m_nLabel = -1;
}

vector<double> Sample::getFeatureVector(){
	return m_vecFeature;
}

void Sample::setLabel(int label){
	m_nLabel = label;
}

int Sample::getLabel(){
	return m_nLabel;
}