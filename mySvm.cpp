#include "mySvm.h"

void MySvm::doubleToFloat(vector<float>& vecDest, vector<double>& vecSrc){
	vector<double>::iterator it = vecSrc.begin();
	while (it != vecSrc.end())
	{
		vecDest.push_back((float)(*it));
		it++;
	}
}

void MySvm::train(){
	//feature vector
	vector<vector<float>>  trainingVector;
	//label vector
	vector<int> trainingLabelVector;

	vector<Sample*>::iterator iter = m_vTrainingSamples.begin();

	while (iter != m_vTrainingSamples.end())
	{
		vector<float> temp;
		doubleToFloat(temp,(*iter)->getFeatureVector());
		trainingVector.push_back(temp);
		trainingLabelVector.push_back((*iter)->getLabel());
		iter++;
	}

	m_params->kernel_type = LINEAR;
	setSampleData(trainingLabelVector,trainingVector);

	m_params->gamma = (float)1/m_problem->l;
	m_params->probability = 1;

	m_model = svm_train(m_problem, m_params);
}

MySvm::MySvm(){
	m_problem = new svm_problem();
	m_params = new svm_parameter();

	//setting parameters for svm
	m_params->svm_type = C_SVC;
	m_params->kernel_type = LINEAR;
	m_params->degree = 3;
	m_params->gamma = 0;
	m_params->C = 100;
	m_params->coef0 = 0;
	m_params->nu = 0.5;
	m_params->eps = 0.001;
	m_params->p = 0.1;
	m_params->shrinking = 1;
	m_params->probability = 0;
	m_params->nr_weight = 0;
	m_params->weight_label = new int[0];
	m_params->weight = new double[0];

	m_problem = NULL;
	m_bModelLoaded = false;
}

MySvm::~MySvm(){
	delete  m_params->weight_label;
	delete m_params->weight;
	delete m_params;
	delete m_problem;
}

//save the current model to file
void MySvm::saveModel(std::string filename){
	svm_save_model(filename.c_str(),m_model);
}

//load model from existing file
void MySvm::loadModel(std::string filename, size_t numOfFeature){
	m_model = svm_load_model(filename.c_str());

	if (m_problem)
	{
		delete m_problem;
	}

	m_problem = new svm_problem();
}

double MySvm::predict(Sample *sample, vector<double>& confidence){
	vector<float> temp;
	doubleToFloat(temp,sample->getFeatureVector());
	return predict(temp,confidence);
}

//predict the result using input feature vector provided by HOGDescriptors
//the results will be stored in vector estimateVec
double MySvm::predict(vector<float>& srcVec, vector<double>& estimateVec){

	svm_node* sourceArray  = new svm_node[m_nNumOfFeatures + 1];
	for (int i=0;i<m_nNumOfFeatures;i++)
	{
		sourceArray[i].index = i+1;
		sourceArray[i].value = srcVec[i];
	}

	//mark the end of feature
	sourceArray[m_nNumOfFeatures].index = -1;

	int sizeEstimate = estimateVec.size();

	double* estimateArray = new double[sizeEstimate];
	for (int i=0;i<sizeEstimate;i++)
	{
		estimateArray[i] = 0;
		estimateArray[i] = estimateVec[i];
	}

	double value = svm_predict_probability(m_model, sourceArray, estimateArray);

	//put data back
	for (int i=0;i<sizeEstimate;i++)
	{
		estimateVec.at(i) = estimateArray[i];
	}

	//free memory here
	delete [] sourceArray;
	delete [] estimateArray;

	//return the label of the class with highest probability
	return value;
}

void MySvm::setSampleData(vector<int> labels, vector<vector<float>>& sampleVector){

	//reset the problem if needed
	if (m_problem)
	{
		delete m_problem;
	}

	m_problem = new svm_problem();

	//load the data into svm_problem
	m_nNumOfSamples = sampleVector.size();
	m_problem->x = new svm_node*[m_nNumOfSamples];

	for(size_t i=0;i<m_nNumOfSamples;i++){
		m_problem->x[i] = new svm_node[m_nNumOfFeatures + 1];
		//mark the end of the feature
		m_problem->x[i][m_nNumOfFeatures].index = -1;
	}

	m_problem->l = m_nNumOfSamples;
	m_problem->y = new double[m_nNumOfSamples];

	vector<vector<float>>::iterator sample_iter = sampleVector.begin() ;
	vector<int>::iterator label_iter = labels.begin();

	for (size_t i=0;i<m_nNumOfSamples;i++){
		for (size_t j=0;j<m_nNumOfFeatures;j++){
			m_problem->x[i][j].index = j+1;
			vector<float>::iterator it_double = (*(sample_iter + i)).begin();
			m_problem->x[i][j].value =  *(it_double + j);
			m_problem->y[i] = *(label_iter + i);
		}
	}
}