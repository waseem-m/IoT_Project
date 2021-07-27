#pragma once

#include "MyIncludes.h"
#include "Detector.h"
#include <algorithm>

using std::string;



class PianoDetector : Detector {
private:
	//private final static String TAG = PianoDetector.class.getSimpleName();
	static const string TAG;
	//private final Mat mHSVMat = new Mat();
	Mat mHSVMat;
	Mat mMaskMat;

	//private final Scalar lowerThreshold = new Scalar(0, 0, 100);
	//const Scalar lowerThreshold = Scalar(0, 0, 100);
	#define LOWER_THRESH Scalar(0, 0, 100)
	//private final Scalar upperThreshold = new Scalar(179, 255, 255);
	#define UPPER_THRESH Scalar(179, 255, 255)

	//const int whiteKeySizeLower = 1000;
	//const int whiteKeySizeUpper = 12500;
	const int whiteKeySizeUpper = 4000;
	const int whiteKeySizeLower = 400;
	//const int whiteKeySizeUpper = 20000;

	//const int blackKeySizeLower = 500;
	//const int blackKeySizeUpper = 5000;
	const int blackKeySizeUpper = 3500;
	//const int blackKeySizeUpper = 10000;
	const int blackKeySizeLower = 200;
	//const int blackKeySizeUpper = 15000;

	vector<vector<Point>> whiteKeysOutLMOP;
	vector<vector<Point>> blackKeysOutLMOP;

	vector<vector<Point>> getContoursBySizeRange(vector<vector<Point>> contours, int lower, int upper) {
		vector<vector<Point>> newContours;
		for (int i = 0; i < contours.size(); i++) {
			int my_area = contourArea(contours.at(i));
			if ( my_area >= lower && my_area <= upper ) {
				newContours.push_back(contours.at(i));
			}
		}
		return newContours;
	}
	
	static Point getMostRight(vector<Point>& contour) {
		Point most_right(-1, -1);

		for (int i = 0; i < contour.size(); i++) {
			if (contour.at(i).x > most_right.x) {
				most_right.x = contour.at(i).x;
				most_right.y = contour.at(i).y;
			}
		}

		return most_right;
	}

	static bool compareContourAreas(vector<Point>&contour1, vector<Point>&contour2) {
		Point most_right_1 = getMostRight(contour1);
		Point most_right_2 = getMostRight(contour2);
		/*double i = fabs(contourArea(contour1));
		double j = fabs(contourArea(contour2));*/
		//return (i < j);
		return (most_right_1.x < most_right_2.x);
	}

	static bool compareContourAreasReverse(vector<Point>&contour1, vector<Point>&contour2) {
		Point most_right_1 = getMostRight(contour1);
		Point most_right_2 = getMostRight(contour2);
		/*double i = fabs(contourArea(contour1));
		double j = fabs(contourArea(contour2));*/
		//return (i > j);
		return (most_right_1.x > most_right_2.x);
	}

	vector<vector<Point>> sortPianoKeys(vector<vector<Point>> contours, bool reverse) {
		if (reverse) {
			std::sort(contours.begin(), contours.end(), compareContourAreasReverse);
		}
		else {
			std::sort(contours.begin(), contours.end(), compareContourAreas);
		}
		return contours;
	}

public:
	//public void apply(final Mat src, final Mat dst) {
	void apply(const Mat &src, const Mat &dst, Scalar &lowerThreshold = LOWER_THRESH, Scalar &upperThreshold = UPPER_THRESH) {
		
		vector<vector<Point>> mWhiteContoursLMOP;
		vector<vector<Point>> mWhiteKeysLMOP;

		vector<vector<Point>> mBlackContoursLMOP;
		vector<vector<Point>> mBlackKeysLMOP;

		vector<Point> mWhiteKeysLP;
		vector<Point> mWhiteKeysMOP;

		vector<int> hullMOI;
		//vector<Point> hullMOI;

		vector<Point> mPianoMaskMOP;
		vector<vector<Point>> mPianoMaskLMOP;
		//todo: is this ok?
		Mat mPianoMaskMat(src.size(), CV_8U, Scalar(0));

		
		// 1. Convert the image to HSV color space
		cvtColor(src, mHSVMat, COLOR_RGB2HSV);

		
		// 2. Apply threshold to detect white piano keys
		inRange(mHSVMat, lowerThreshold, upperThreshold, mMaskMat);
		//imshow("2.White Mask", mMaskMat);

		
		// 3. Perform erosion
		//Imgproc.erode(mMaskMat, mMaskMat, new Mat());
		erode(mMaskMat, mMaskMat, Mat()); //(original is commented)
		erode(mMaskMat, mMaskMat, Mat()); //(original is commented)
		//todo:added
		//dilate(mMaskMat, mMaskMat, Mat());

		// 4. Find contours
		//Imgproc.findContours(mMaskMat, mWhiteContoursLMOP, new Mat(), Imgproc.RETR_LIST, Imgproc.CHAIN_APPROX_SIMPLE);
		//findContours(mMaskMat, mWhiteContoursLMOP, Mat(), RETR_LIST, CHAIN_APPROX_SIMPLE);

		vector<Vec4i> hierarchy;
		findContours(mMaskMat, mWhiteContoursLMOP, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);

		//imshow("source", src);
		
		
		// 5. If no contours detected, return.
		if (mWhiteContoursLMOP.size() == 0) {
			cout << TAG << ": No white contours found!" << endl;
			return;
		}

		/*Mat drawing = Mat::zeros(mMaskMat.size(), CV_8UC3);
		cv::RNG rng(12345);
		for (size_t i = 0; i < mWhiteContoursLMOP.size(); i++)
		{
			Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
			drawContours(drawing, mWhiteContoursLMOP, (int)i, color, 2, LINE_8, hierarchy, 0);
		}
		imshow("White Contours", drawing);*/

		
		// 7. Get contours that are within certain contour size range
		mWhiteKeysLMOP = getContoursBySizeRange(mWhiteContoursLMOP, whiteKeySizeLower, whiteKeySizeUpper);
		
		
		// 8. Reduce number of points of each contour using DP algorithm
		for (int i = 0; i < mWhiteKeysLMOP.size(); i++) {
			//mWhiteKeysLMOP.set(i, reduceContourPoints(mWhiteKeysLMOP.get(i)));
			mWhiteKeysLMOP.at(i) = reduceContourPoints(mWhiteKeysLMOP.at(i));
		}

		
		// 9. Eliminate contours that have less than 6 points or more than 8 points

		// 10. If no contours, just return
		if (mWhiteKeysLMOP.size() == 0) {
			cout << TAG << ": No white keys found!" << endl;
			return;
		}

		
		// 11. Draw white keys
		drawAllContours(dst, mWhiteKeysLMOP, Colors::mLineColorRed, -1);
		//imshow("whiteKeys", dst);
		
		
		// 12. Get convex hull of piano
		/*todo: working
		vector<vector<Point>> hull(mWhiteKeysLMOP.size());
		for (size_t i = 0; i < mWhiteKeysLMOP.size(); i++)
		{
			convexHull(mWhiteKeysLMOP[i], hull[i]);
		}*/
		
		//// 12a. Convert LMOP to LP
		for (int i = 0; i < mWhiteKeysLMOP.size(); i++) {
			//mWhiteKeysLP.addAll(mWhiteKeysLMOP.get(i).toList());
			vector<Point> myElement = mWhiteKeysLMOP.at(i);
			mWhiteKeysLP.insert(mWhiteKeysLP.end(), myElement.begin(), myElement.end());
		}

		//// 12b. Convert LP to MOP
		//mWhiteKeysMOP.fromList(mWhiteKeysLP);
		mWhiteKeysMOP = mWhiteKeysLP;
		

		// 12c. Get convex hull
		convexHull(mWhiteKeysMOP, hullMOI);
		
		/*
		vector<vector<Point>> hullL;
		hullL.push_back(hullMOI);
		for (size_t i = 0; i < hullL.size(); i++)
		{
			drawContours(drawing, hullL, (int)i, Colors::mLineColorRed);
		}
		imshow("src", src);
		imshow("Hull demo", drawing);*/

		
		// 12d. Convert hullMOI to MOP
		mPianoMaskMOP = hullToContour(hullMOI, mWhiteKeysMOP);

		
		// 12e. Convert MOP to LMOP
		mPianoMaskLMOP.push_back(mPianoMaskMOP);
		

		
		// 13. Create piano mask mat
		//drawContours(mPianoMaskMat, mPianoMaskLMOP, 0, Colors::mLineColorWhite, -1);
		drawContours(mPianoMaskMat, mPianoMaskLMOP, 0, Colors::mLineColorWhite, -1);
		inRange(mHSVMat, lowerThreshold, upperThreshold, mMaskMat);
		//imshow("13.inRange", mMaskMat);
		
		// 14. Dilate image 3 times to remove piano lines
		dilate(mMaskMat, mMaskMat, Mat());
		dilate(mMaskMat, mMaskMat, Mat());
		dilate(mMaskMat, mMaskMat, Mat());
		//imshow("14.dilate", mMaskMat);
		
		// 15. Invert piano mask
		bitwise_not(mMaskMat, mMaskMat);
		//imshow("15.invert", mMaskMat);

		
		// 16. Apply piano mask to binary image
		mMaskMat.copyTo(mPianoMaskMat, mPianoMaskMat);
		
		// 17. Find black key contours
		findContours(mPianoMaskMat, mBlackContoursLMOP, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);
		
		// 18. If no contours detected, return.
		if (mBlackContoursLMOP.size() == 0) {
			//Log.i(TAG, "No black contours found!");
			cout << TAG << ": No black contours found!" << endl;
			return;
		}
		
		// 19. Get contours that are within certain contour size range
		mBlackKeysLMOP = getContoursBySizeRange(mBlackContoursLMOP, blackKeySizeLower, blackKeySizeUpper);
		
		// 20. Reduce number of points of each contour using DP algorithm
		for (int i = 0; i < mBlackKeysLMOP.size(); i++) {
			mBlackKeysLMOP.at(i) = reduceContourPoints(mBlackKeysLMOP.at(i));
		}

		// 21. If no contours, just return
		if (mBlackKeysLMOP.size() == 0) {
			//Log.i(TAG, "No black key found!");
			cout << TAG << ": No black keys found!" << endl;
			return;
		}
		
		// 22. Draw black key contours
		drawAllContours(dst, mBlackKeysLMOP, Colors::mLineColorGreen, -1);
		
		// 25. Sort piano keys and update whiteKeysOutLMOP and blackKeysOutLMOP
		whiteKeysOutLMOP = sortPianoKeys(mWhiteKeysLMOP, true);
		blackKeysOutLMOP = sortPianoKeys(mBlackKeysLMOP, true);

		/*Mat drawing1 = Mat::zeros(dst.size(), CV_8UC3);
		for (size_t i = 0; i < whiteKeysOutLMOP.size(); i++)
		{
			drawContours(drawing1, whiteKeysOutLMOP, (int)i, Colors::mLineColorBlue);
		}
		Mat drawing2 = Mat::zeros(dst.size(), CV_8UC3);
		for (size_t i = 0; i < blackKeysOutLMOP.size(); i++)
		{
			drawContours(drawing2, blackKeysOutLMOP, (int)i, Colors::mLineColorBlue);
		}
		imshow("whitekeys", drawing1);
		imshow("blackkeys", drawing2);*/
		
		
	}

	vector<vector<Point>> getWhiteKeysLMOP() {
		return whiteKeysOutLMOP;
	}

	vector<vector<Point>> getBlackKeysLMOP() {
		return blackKeysOutLMOP;
	}

	void drawAllContours(const Mat dst, vector<vector<Point>> contours, Scalar color, int thickness) {
		//todo: check if correct
		/*for (int i = 0; i < contours.size(); i++) {
			drawContours(dst, contours, i, color, thickness);
		}*/
		vector<Vec4i> hierarchy;
		for (size_t i = 0; i < contours.size(); i++)
		{
			//Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
			drawContours(dst, contours, (int)i, color, 2, LINE_8, hierarchy, thickness);
		}
		//todo: delete
		//imshow("Contours", dst);
	}
};

const string PianoDetector::TAG("PianoDetector");