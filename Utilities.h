#pragma once

#include <string>
#include <vector>
#include "dirent.h"
#include <fstream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

namespace utilities
{
	void getFilesInDirectory(const string& dirName, vector<string>& fileNames, const vector<string>& validExtensions);
	string toLowerCase(const string& in);
	void storeCursor(void);
	void resetCursor(void);
	void saveVectorToFile(vector<float>& vec, vector<unsigned int>& _vectorIndices, string fileName);
	void drawRectangles(const vector<Rect>& found, Mat& imageData);
};