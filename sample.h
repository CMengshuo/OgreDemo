#ifndef _SAMPLE_H_
#define  _SAMPLE_H_

#include <vector>
using namespace std;

class Sample{
public:
	Sample(vector<float> vec ,int label);
	Sample(vector<float> vec);
	
	void setLabel(int label);
	vector<double> getFeatureVector();
	int getLabel();

protected:
	vector<double> m_vecFeature;
	int m_nLabel;
};

#endif