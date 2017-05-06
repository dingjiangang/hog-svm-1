#include "stdafx.h"
#include <iostream>
#include "SVMClassifier.h"

/**
* Training:
*
* HOG train <dataset> [-ws <window_size> -ss <split_size> -g]
** dataset: directory containing a 'pos' and a 'neg' folder with corresponding training samples
** ws: sliding window size
** ss: image split size
** g: grayscale training
*
*
* Detection:
* 
* HOG run <image> [-t <threshold> -s <scale> -w <width> -o -nms -p <padding> -pw <padding_width> -ph <padding_height> -g]
** image: path to image to compute
** t: detection threshold
** s: image pyramidal scale
** o: output image with detections (bool)
** w: max image width
** nms: apply nms (bool)
** p: nms padding
** pw: horiztonal nms padding
** ph: vertical nms padding
** g: grayscale detection (the training must have been done in grayscale also)
**/
int main(int argc, char** argv) {
	SVMClassifier::DetectParams detectParams;
	detectParams.applyNms = false;
	detectParams.outputImage = false;
	detectParams.hitThreshold = 1.0;
	detectParams.nmsPaddingHeight = 0.f;
	detectParams.nmsPaddingWidth = 0.f;
	detectParams.maxWidth = 800;
	detectParams.scale = 1.005;

	SVMClassifier::TrainParams trainingParams;
	trainingParams.maxTrainingImageSize = 256;
	trainingParams.trainingPadding = Size(0, 0);
	trainingParams.winSize = Size(128, 128);

	SVMClassifier classifier;

	if (std::string(argv[1]) == "run") {
		for (int i = 3; i < argc; ++i) {
			if (string(argv[i]) == "--threshold" || string(argv[i]) == "-t") {
				string::size_type sz;
				detectParams.hitThreshold = std::stod(argv[i + 1], &sz);
			}
			if (string(argv[i]) == "--scale" || string(argv[i]) == "-s") {
				string::size_type sz;
				detectParams.scale = std::stod(argv[i + 1], &sz);
			}
			if (string(argv[i]) == "--output" || string(argv[i]) == "-o") {
				detectParams.outputImage = true;
			}
			if (string(argv[i]) == "--width" || string(argv[i]) == "-w") {
				detectParams.maxWidth = atoi(argv[i + 1]);
			}
			if (string(argv[i]) == "--padding" || string(argv[i]) == "-p") {
				detectParams.nmsPaddingHeight = detectParams.nmsPaddingWidth = atof(argv[i + 1]);
			}
			if (string(argv[i]) == "--padding_width" || string(argv[i]) == "-pw") {
				detectParams.nmsPaddingWidth = atof(argv[i + 1]);
			}
			if (string(argv[i]) == "--padding_height" || string(argv[i]) == "-ph") {
				detectParams.nmsPaddingHeight = atof(argv[i + 1]);
			}
			if (string(argv[i]) == "--nms" || string(argv[i]) == "-nms") {
				detectParams.applyNms = true;
			}
			if (string(argv[i]) == "--gray" || string(argv[i]) == "-g") {
				classifier.setImageType(CV_LOAD_IMAGE_GRAYSCALE);
			}
		}

		detectParams.imagePath = std::string(argv[2]);

		std::cout << "Processing image...";

		classifier.load();
		auto foundRects = classifier.detect(detectParams);

		if (foundRects == nullptr) {
			cerr << "Error reading the image";
			return EXIT_FAILURE;
		}

		std::cout << "\nwindow_count:" << foundRects->size() << ";";

		std::cout << "\n{\"window_rects\":[";

		for (int i = 0; i < foundRects->size(); i++) {
			auto rect = (*foundRects)[i];

			printf("{\"x\":%d,\"y\":%d,\"width\":%d,\"height\":%d}", rect.x, rect.y, rect.width, rect.height);

			if (i < foundRects->size() - 1) {
				std::cout << ",";
			}
		}

		std::cout << "]}";

		return EXIT_SUCCESS;
	}
	else if (std::string(argv[1]) == "train") {
		for (int i = 3; i < argc; ++i) {
			if (string(argv[i]) == "--window_size" || string(argv[i]) == "-ws") {
				trainingParams.winSize = Size(atoi(argv[i + 1]), atoi(argv[i + 1]));
			}
			if (string(argv[i]) == "--split_size" || string(argv[i]) == "-ss") {
				trainingParams.maxTrainingImageSize = atoi(argv[i + 1]);
			}
			if (string(argv[i]) == "--gray" || string(argv[i]) == "-g") {
				classifier.setImageType(CV_LOAD_IMAGE_GRAYSCALE);
			}
		}

		trainingParams.trainingDir = std::string(argv[2]);

		return classifier.train(trainingParams);
	}

	return EXIT_FAILURE;
}