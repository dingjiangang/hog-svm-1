#include "stdafx.h"
#include "SVMClassifier.h"
#include "svmlight/svmlight.h"
#include "NMSFilter.h"

const string SVMClassifier::FEATURES_FILE = "genfiles/features.dat";
const string SVMClassifier::SVM_MODEL_FILE = "genfiles/svmlightmodel.dat";
const string SVMClassifier::DESCRIPTOR_VECTOR_FILE = "genfiles/descriptorvector.dat";
const string SVMClassifier::HOG_FILE = "genfiles/cvHOGClassifier.yaml";
const string SVMClassifier::TRAINING_PARAMS_FILE = "genfiles/trainingParams.txt";

SVMClassifier::SVMClassifier() : winStride_(Size(8, 8)), imageType_(CV_LOAD_IMAGE_COLOR) {}

SVMClassifier::~SVMClassifier() {}

int SVMClassifier::train(TrainParams& params) {
	this->hog_.winSize = params.winSize;

	string posSamplesDir = "pos\\";
	string negSamplesDir = "neg\\";

	vector<string> positiveTrainingImages;
	vector<string> negativeTrainingImages;
	vector<string> validExtensions;
	validExtensions.push_back("jpg");
	validExtensions.push_back("png");

	auto baseDir = params.trainingDir + "\\";

	utilities::getFilesInDirectory(baseDir + posSamplesDir, positiveTrainingImages, validExtensions);
	utilities::getFilesInDirectory(baseDir + negSamplesDir, negativeTrainingImages, validExtensions);
	unsigned long overallSamples = positiveTrainingImages.size() + negativeTrainingImages.size();

	if (overallSamples == 0) {
		printf("No training sample files found, nothing to do!\n");
		return EXIT_SUCCESS;
	}

	// Do not use the system locale
	setlocale(LC_ALL, "C");
	setlocale(LC_NUMERIC,"C");
	setlocale(LC_ALL, "POSIX");

	printf("Reading files, generating HOG features and save them to file '%s':\n", FEATURES_FILE.c_str());
	float percent;
	int pos = 0, neg = 0;
	fstream File;
	File.open(FEATURES_FILE.c_str(), ios::out);
	if (File.good() && File.is_open()) {
		File << "# Use this file to train, e.g. SVMlight by issuing $ svm_learn -i 1 -a weights.txt " << FEATURES_FILE.c_str() << endl;
		// Iterate over sample images
		for (unsigned long currentFile = 0; currentFile < overallSamples; ++currentFile) {
			utilities::storeCursor();
			vector<float> featureVector;
			// Get positive or negative sample image file path
			const string currentImageFile = (currentFile < positiveTrainingImages.size() ? positiveTrainingImages.at(currentFile) : negativeTrainingImages.at(currentFile - positiveTrainingImages.size()));
			// Output progress
			if ( (currentFile+1) % 10 == 0 || (currentFile+1) == overallSamples ) {
				percent = ((currentFile+1) * 100 / overallSamples);
				printf("%5lu (%3.0f%%):\tFile '%s'", (currentFile+1), percent, currentImageFile.c_str());
				fflush(stdout);
				utilities::resetCursor();
			}
			// Calculate feature vector from current image file
			int featureCount = this->calculateFeaturesFromInput(params, currentImageFile, featureVector);
			if (!featureVector.empty()) {
				/* Put positive or negative sample class to file, 
				* true=positive, false=negative, 
				* and convert positive class to +1 and negative class to -1 for SVMlight
				*/
				currentFile < positiveTrainingImages.size() ? pos+=featureCount : neg+=featureCount;

				File << ((currentFile < positiveTrainingImages.size()) ? "+1" : "-1");
				// Save feature vector components
				for (unsigned int feature = 0; feature < featureVector.size(); ++feature) {
					File << " " << (feature + 1) << ":" << featureVector.at(feature);
				}
				File << endl;
			}
		}
		printf("\n");
		File.flush();
		File.close();
	} else {
		printf("Error opening file '%s'!\n", FEATURES_FILE.c_str());
		return EXIT_FAILURE;
	}
	// Read in and train the calculated feature vectors
	printf("Calling %s\n", SVMlight::getInstance()->getSVMName());
	SVMlight::getInstance()->read_problem(const_cast<char*> (FEATURES_FILE.c_str()));
	SVMlight::getInstance()->train(); // Call the core svmlight training procedure
	printf("Training done, saving model file!\n");
	SVMlight::getInstance()->saveModelToFile(SVM_MODEL_FILE);

	printf("Generating representative single HOG feature vector using svmlight!\n");
	vector<float> descriptorVector;
	vector<unsigned int> descriptorVectorIndices;
	// Generate a single detecting feature vector (v1 | b) from the trained support vectors, for use e.g. with the HOG algorithm
	SVMlight::getInstance()->getSingleDetectingVector(descriptorVector, descriptorVectorIndices);
	// And save the precious to file system
	utilities::saveVectorToFile(descriptorVector, descriptorVectorIndices, DESCRIPTOR_VECTOR_FILE);

	// Detector detection tolerance threshold
	const double hitThreshold = SVMlight::getInstance()->getThreshold();
	// Set our custom detecting vector
	this->hog_.setSVMDetector(descriptorVector);
	this->hog_.save(HOG_FILE);

	printf("Positives: %i\nNegatives: %i\n", pos, neg);

	// Export training params
	ofstream f;
	f.open(this->TRAINING_PARAMS_FILE);
	f << "Training directory: " << params.trainingDir;
	f << "\nGrayscale: " << (this->imageType_ == CV_LOAD_IMAGE_GRAYSCALE);
	f << "\nMax training image size: " << params.maxTrainingImageSize;
	f << "\nTraining padding: " << params.trainingPadding;
	f << "\nWindow size: " << params.winSize;
	f << "\nSVMLight hit threshold: " << hitThreshold;
	f << "\nPositives/Negatives: " << pos << "/" << neg;
	f.close();

	return EXIT_SUCCESS;
}

int SVMClassifier::calculateFeaturesFromInput(TrainParams& params, const string& imageFilename, vector<float>& featureVector) {
	Mat imageData = imread(imageFilename, this->imageType_);
	if (imageData.empty()) {
		featureVector.clear();
		printf("Error: HOG image '%s' is empty, features calculation skipped!\n", imageFilename.c_str());
		return 0;
	}
	int i = 0;
	int width = imageData.cols;
	int height = imageData.rows;

	// Resize if necessary
	if (width >= params.maxTrainingImageSize || height >= params.maxTrainingImageSize) {
		Size size = max(width, height) == width ? Size(params.maxTrainingImageSize, height*params.maxTrainingImageSize/width) : Size(width*params.maxTrainingImageSize/height, params.maxTrainingImageSize);
		resize(imageData, imageData, size);
	}

	const int N = this->hog_.winSize.width;

	if (imageData.rows < N || imageData.cols < N) {
		return 0;
	}

	// Split image into smaller segments
	for (int r = 0; r < imageData.rows; r += N) {
		for (int c = 0; c < imageData.cols; c += N) {
			cv::Range rowRange(r, min(r + N, imageData.rows));
			cv::Range colRange(c, min(c + N, imageData.cols));

			if (rowRange.end - rowRange.start != this->hog_.winSize.width) {
				rowRange = cv::Range(imageData.rows - N, imageData.rows);
			}

			if (colRange.end - colRange.start != this->hog_.winSize.height) {
				colRange = cv::Range(imageData.cols - N, imageData.cols);
			}

			cv::Mat tile = imageData(rowRange, colRange);

			auto extLocation = imageFilename.find_last_of(".");
			string ext = imageFilename.substr(extLocation);

			i++;
			vector<Point> locations;
			this->hog_.compute(tile, featureVector, this->winStride_, params.trainingPadding, locations);
		}
	}

	imageData.release();

	return i;
}

void SVMClassifier::load() {
	this->hog_.load(HOG_FILE);
}

vector<Rect>* SVMClassifier::detect(DetectParams& params) {
	Mat testImage = imread(params.imagePath, this->imageType_);

	if (testImage.empty()) {
		printf("Error: HOG image '%s' is empty!\n", params.imagePath);
		return 0;
	}

	auto originalWidth = testImage.cols;
	auto originalHeight = testImage.rows;

	if (testImage.empty()) {
		return nullptr;
	}

	int width = params.maxWidth > 0 ? params.maxWidth : testImage.cols;
	int height = width == testImage.cols ? testImage.rows : testImage.rows*width/testImage.cols;

	resize(testImage, testImage, cv::Size(width, height));

	auto rects = this->hogDetect(params, testImage);

	if (params.outputImage) {
		utilities::drawRectangles(rects, testImage);

		size_t lastindex = params.imagePath.find_last_of(".");
		string rawname = params.imagePath.substr(0, lastindex);
		string extension = params.imagePath.substr(lastindex);

		imwrite(rawname + "-tested" + extension, testImage);
	}

	// Convert back to original input image dimensions
	auto outputRects = new vector<Rect>();
	for (int i = 0; i < rects.size(); i++)
	{
		auto r = Rect(rects[i]);

		r.width *= (float)originalWidth/(float)testImage.cols;
		r.height *= (float)originalHeight/(float)testImage.rows;
		r.x *= (float)originalWidth/(float)testImage.cols;
		r.y *= (float)originalHeight/(float)testImage.rows;

		outputRects->push_back(r);
	}

	return outputRects;
}

vector<Rect> SVMClassifier::hogDetect(DetectParams& params, Mat& imageData) {
	vector<Rect> found;
	Size padding(8,8);
	float imagePaddingThreshold = 0.08f;

	this->hog_.detectMultiScale(imageData, found, params.hitThreshold, this->winStride_, padding, params.scale);

	vector<Rect>::iterator it = found.begin();

	// Enlarge rectangles
	while(it != found.end()) {
		it->x -= it->width*params.nmsPaddingWidth/2.0f;
		it->width += it->width*params.nmsPaddingWidth;
		it->y -= it->height*params.nmsPaddingHeight/2.0f;
		it->height += it->height*params.nmsPaddingHeight;

		// Remove if the rectangle overlaps with the outside of the image
		if (it->x < imageData.cols*imagePaddingThreshold || it->y < imageData.rows*imagePaddingThreshold
			|| it->x + it->width > imageData.cols*(1.0f - imagePaddingThreshold) || it->y + it->height > imageData.rows*(1.0f - imagePaddingThreshold)) {
				it = found.erase(it);
		}

		else ++it;
	}

	if (!params.applyNms) {
		return found;
	}

	// Apply NMS filtering
	vector<Rect> resRects;
	nms::apply(found, resRects, 0.01f);

	return resRects;
}