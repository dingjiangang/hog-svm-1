#pragma once

#include <tuple>
#include <ios>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include "Utilities.h"

using namespace std;
using namespace cv;

class SVMClassifier
{
public:
	struct TrainParams{
		string trainingDir;
		Size trainingPadding;
		int maxTrainingImageSize;
		Size winSize;
	};

	struct DetectParams{
		string imagePath;
		double hitThreshold;
		bool applyNms;
		float nmsPaddingWidth;
		float nmsPaddingHeight;
		int maxWidth;
		bool outputImage;
		double scale;
	};

	SVMClassifier();
	~SVMClassifier();

	void load();
	int train(TrainParams& params);
	vector<Rect>* detect(DetectParams& params);

	int getImageType() const { return this->imageType_; }
	void setImageType(int type) { this->imageType_ = type; }

private:
	int calculateFeaturesFromInput(TrainParams& params, const string& imageFilename, vector<float>& featureVector);
	vector<Rect> hogDetect(DetectParams& params, Mat& imageData);

	HOGDescriptor hog_;
	Size winStride_;
	int imageType_;

	static const string FEATURES_FILE;
	static const string SVM_MODEL_FILE;
	static const string DESCRIPTOR_VECTOR_FILE;
	static const string HOG_FILE;
	static const string TRAINING_PARAMS_FILE;
};