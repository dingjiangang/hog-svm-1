#include "stdafx.h"
#include "Utilities.h"

void utilities::getFilesInDirectory(const string& dirName, vector<string>& fileNames, const vector<string>& validExtensions) {
	printf("Opening directory %s\n", dirName.c_str());
#ifdef __MINGW32__
	struct stat s;
#endif
	struct dirent* ep;
	size_t extensionLocation;
	DIR* dp = opendir(dirName.c_str());
	if (dp != NULL) {
		while ((ep = readdir(dp))) {
			// Ignore (sub-)directories like . , .. , .svn, etc.
#ifdef __MINGW32__	
			stat(ep->d_name, &s);
			if (s.st_mode & S_IFDIR) {
				continue;
			}
#else
			if (ep->d_type & DT_DIR) {
				continue;
			}
#endif
			extensionLocation = string(ep->d_name).find_last_of("."); // Assume the last point marks beginning of extension like file.ext
			// Check if extension is matching the wanted ones
			string tempExt = toLowerCase(string(ep->d_name).substr(extensionLocation + 1));
			if (find(validExtensions.begin(), validExtensions.end(), tempExt) != validExtensions.end()) {
				printf("Found matching data file '%s'\n", ep->d_name);
				fileNames.push_back((string) dirName + ep->d_name);
			} else {
				printf("Found file does not match required file type, skipping: '%s'\n", ep->d_name);
			}
		}
		(void) closedir(dp);
	} else {
		printf("Error opening directory '%s'!\n", dirName.c_str());
	}
	return;
}

void utilities::saveVectorToFile(vector<float>& vec, vector<unsigned int>& _vectorIndices, string fileName) {
	printf("Saving descriptor vector to file '%s'\n", fileName.c_str());
	string separator = " "; // Use blank as default separator between single features
	fstream File;
	float percent;
	File.open(fileName.c_str(), ios::out);
	if (File.good() && File.is_open()) {
		printf("Saving %lu descriptor vector features:\t", vec.size());
		storeCursor();
		for (int feature = 0; feature < vec.size(); ++feature) {
			if ((feature % 10 == 0) || (feature == (vec.size()-1)) ) {
				percent = ((1 + feature) * 100 / vec.size());
				printf("%4u (%3.0f%%)", feature, percent);
				fflush(stdout);
				resetCursor();
			}
			File << vec.at(feature) << separator;
		}
		printf("\n");
		File << endl;
		File.flush();
		File.close();
	}
}

string utilities::toLowerCase(const string& in) {
	string t;
	for (string::const_iterator i = in.begin(); i != in.end(); ++i) {
		t += tolower(*i);
	}
	return t;
}


void utilities::storeCursor(void) {
	printf("\033[s");
}

void utilities::resetCursor(void) {
	printf("\033[u");
}

void utilities::drawRectangles(const vector<Rect>& found, Mat& imageData) {
	vector<Rect> found_filtered;
	size_t i, j;
	for (i = 0; i < found.size(); ++i) {
		Rect r = found[i];
		for (j = 0; j < found.size(); ++j)
			if (j != i && (r & found[j]) == r)
				break;
		if (j == found.size())
			found_filtered.push_back(r);
	}
	for (i = 0; i < found_filtered.size(); i++) {
		Rect r = found_filtered[i];
		rectangle(imageData, r.tl(), r.br(), Scalar(64, 255, 64), 3);
	}
}
