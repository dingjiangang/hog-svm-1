# Object Detection - SVM & Histogram of Oriented Gradients

### SVM HOG Training and Detection from code
- Visual Studio 2012 is required
- Download OpenCV 2.4.13
- Add a user variable named `OPENCV_DIR` that points to `opencv2\build` folder
- Add the following system variable: `%OPENCV_DIR%\x64\vc11\bin`
- Go to `svm\svm-trainer`
- Launch a command prompt in admin and train the SVM (read the documentation from main.cpp)
- Copy the `genfiles` folder to where you want to run the `HOG.exe` executable from
- Launch a command prompt in admin and use the SVM to detect either windows or garage doors (read the documentation from main.cpp)

### SVM HOG Training and Detection from current builds
- Download Visual C++ Redistributable for Visual Studio 2012
- Go to one of the following folders:
-- To detect windows: `svm\svm-trainer\builds\window`
-- To detect garage doors: `svm\svm-trainer\builds\garage`
- Launch a command prompt in admin and run the executable `HOG.exe run` with the right parameters (read the documentation from main.cpp)


#### Librairies: svmlight, OpenCV 2.4.13
