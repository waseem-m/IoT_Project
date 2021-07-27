#pragma once

#include "MyIncludes.h"
#include "Detector.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

#define SEARCH_BOTTLENECK false

class HandDetector : Detector {

private:
	static const string TAG;

	//const int redArea = 100;
	const int redArea = 3;
	const int handArea = 5000;

	//threshold for skin color
	const int lowerHue = 3;
	const int upperHue = 33;
	const Scalar lowerThreshold = Scalar(lowerHue, 50, 50);
	const Scalar upperThreshold = Scalar(upperHue, 255, 255);

	//two threshold ranges for red
	const Scalar redLowerThreshold1 = Scalar(0, 100, 20);
	const Scalar redUpperThreshold1 = Scalar(10, 255, 255);
	const Scalar redLowerThreshold2 = Scalar(160,100,20);
	const Scalar redUpperThreshold2 = Scalar(179,255,255);

	//threshold for blue
	const Scalar blueLowerThreshold = Scalar(95, 100, 100);
	const Scalar blueUpperThreshold = Scalar(145, 255, 180);
	
	//threshold for green
	const Scalar greenLowerThreshold = Scalar(40, 40, 40);
	const Scalar greenUpperThreshold = Scalar(70, 255, 255);

	Mat mMat;

	vector<vector<Point>> contoursOut;

	Point lowestPoint;
	vector<Point> lowestPoints;

#if SEARCH_BOTTLENECK == true
	int e1 = 0, e2 = 0;
	double freq = 0;
#endif

	Point findCenterPoint(vector<Point> contour) {

		cv::Moments m = cv::moments(contour);
		//cv::Rect br = cv::boundingRect(contour);
		//Point contourCenter(br.x + br.width / 2, br.y + br.height / 2);
		Point contourCenter(m.m10 / m.m00 , m.m01 / m.m00);
		return contourCenter;
	}
	Point findHighestPoint(vector<Point> contour) {
		Point highest(-1, -1);

		for (int i = 0; i < contour.size(); i++) {
			if (contour.at(i).y > highest.y) {
				highest.x = contour.at(i).x;
				highest.y = contour.at(i).y;
			}
		}

		return highest;
	}

	Point findLowestPoint(vector<Point> contour) {
		Point lowest(5000,5000);
		
		for (int i = 0; i < contour.size(); i++) {
			if (contour.at(i).y < lowest.y) {
				lowest.x = contour.at(i).x;
				lowest.y = contour.at(i).y;
			}
		}

		return lowest;
	}
	
	int findHighestContourIndex(vector<vector<Point>> contours) {
		int index = -1;
		int maxY = -1;

		for (int i = 0; i < contours.size(); i++) {
			int thisY = findHighestPoint(contours.at(i)).y;
				if (contourArea(contours.at(i)) >= redArea && thisY > maxY) {
					index = i;
					maxY = thisY;
				}
		}

		return index;
	}


	
public:
	//	public void apply(final Mat dst, final Mat src) {

	void apply_blue(const Mat& dst, const Mat& src) {
		vector<vector<Point>> contours;

#if SEARCH_BOTTLENECK == true
		cout << endl << endl << endl;
		cout << "-------------------------------------------" << endl;
		cout << "Apply blue performance:" << endl << endl;
		e1 = e1 = cv::getTickCount();
#endif
		// 1. Convert image to HSV color space
		cvtColor(src, mMat, COLOR_BGR2HSV);
#if SEARCH_BOTTLENECK == true
		double time_diff = 0;
		double total_time = 0;
		freq = cv::getTickFrequency();
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "cvtColor(.., RGB2HSB) = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// 2. Apply static blue threshold

		inRange(mMat, blueLowerThreshold, blueUpperThreshold, mMat);


		//imshow("1. threshold mask", mMat);
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "inRange(red_mask) = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// 3a. Perform dilation
		// dilate(mMat, mMat, Mat()); //(commented in source)

		// 3a. Perform erosion
		erode(mMat, mMat, Mat());
		//imshow("2. mask erode", mMat);
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "erode = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// 4. Find contours
		vector<Vec4i> hierarchy;
		//todo: changed RETR_LIST to RETR_EXTERNAL
		//findContours(mMat, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);
		findContours(mMat, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "findContours = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// 5. If no contours, return
		if (contours.size() == 0) {
			//cout << TAG << ": No contours found" << endl;
			lowestPoint.x = -1;
			lowestPoint.y = -1;
			return;
		}


		/*Mat drawing = Mat::zeros(dst.size(), CV_8UC3);
		cv::RNG rng(12345);
		for (size_t i = 0; i < contours.size(); i++)
		{
			Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
			drawContours(drawing, contours, (int)i, color, 2, LINE_8, hierarchy, 0);
		}
		imshow("3. Contours", drawing);*/


		// 6. Find index of the largest contour, assume that is the hand
		// todo: changed to highest contour instead of largest
		int largestContourIndex = findHighestContourIndex(contours);
		//cout << TAG << ": Contour index=" << largestContourIndex endl;
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "***findHighestContourIndex*** = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// 7. If index equals -1, return
		if (largestContourIndex == -1) {
			//cout << TAG << ": No hand detected" << endl;
			lowestPoint.x = -1;
			lowestPoint.y = -1;
			return;
		};

		// 8. Reduce number of points using DP algorithm
		vector<vector<Point>> reducedHandContours;
		reducedHandContours.push_back(reduceContourPoints(contours.at(largestContourIndex)));
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "reducedHandContours.push_back(reduceContourPoints(contours.at(largestContourIndex))) = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		/*Mat drawing3 = Mat::zeros(dst.size(), CV_8UC3);
		cv::RNG rng3(12345);
		for (size_t i = 0; i < reducedHandContours.size(); i++)
		{
			Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
			drawContours(drawing3, reducedHandContours, (int)i, color, 2, LINE_8, hierarchy, 0);
		}
		imshow("4. reduced", drawing3);*/


		//cout << TAG << ": contourArea(reducedHandContours.at(0)) = " << contourArea(reducedHandContours.at(0)) << endl;

		// Draw contours
		//drawContours(dst, reducedHandContours, 0, Colors::mLineColorGreen, 2);
		drawContours(dst, reducedHandContours, 0, Colors::mLineColorBlue, 2);
		//imshow("6. added reducedHandContours", dst);
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "drawContours = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// Find lowest point
		// todo: changed to reduced instead of hull.
		// todo: changed to center of contour.
		//lowestPoint = findHighestPoint(reducedHandContours.at(0));
		lowestPoint = findCenterPoint(reducedHandContours.at(0));
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "findCenterPoint = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// Draw lowest point
		if (lowestPoint != Point(-1, -1)) {
			circle(dst, lowestPoint, 2, (Colors::mLineColorGreen + Colors::mLineColorBlue), -1);
#if SEARCH_BOTTLENECK == true
			e2 = cv::getTickCount();
			time_diff = (e2 - e1) / freq;
			cout << "circle (Draw lowest point) = " << time_diff << endl;
			total_time += time_diff;
#endif
		}

#if SEARCH_BOTTLENECK == true
		cout << endl;
		cout << "total_time = " << total_time << endl;
		total_time = 0;
		cout << "-------------------------------------------" << endl;
#endif

	}

	void apply_green(const Mat& dst, const Mat& src) {
		vector<vector<Point>> contours;

#if SEARCH_BOTTLENECK == true
		cout << endl << endl << endl;
		cout << "-------------------------------------------" << endl;
		cout << "Apply green performance:" << endl << endl;
		e1 = e1 = cv::getTickCount();
#endif
		// 1. Convert image to HSV color space
		cvtColor(src, mMat, COLOR_BGR2HSV);
#if SEARCH_BOTTLENECK == true
		double time_diff = 0;
		double total_time = 0;
		freq = cv::getTickFrequency();
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "cvtColor(.., RGB2HSB) = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// 2. Apply static green threshold

		inRange(mMat, greenLowerThreshold, greenUpperThreshold, mMat);


		//imshow("1. threshold mask", mMat);
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "inRange(red_mask) = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// 3a. Perform dilation
		// dilate(mMat, mMat, Mat()); //(commented in source)

		// 3a. Perform erosion
		erode(mMat, mMat, Mat());
		//imshow("2. mask erode", mMat);
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "erode = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// 4. Find contours
		vector<Vec4i> hierarchy;
		//todo: changed RETR_LIST to RETR_EXTERNAL
		//findContours(mMat, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);
		findContours(mMat, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "findContours = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// 5. If no contours, return
		if (contours.size() == 0) {
			//cout << TAG << ": No contours found" << endl;
			lowestPoint.x = -1;
			lowestPoint.y = -1;
			return;
		}


		/*Mat drawing = Mat::zeros(dst.size(), CV_8UC3);
		cv::RNG rng(12345);
		for (size_t i = 0; i < contours.size(); i++)
		{
			Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
			drawContours(drawing, contours, (int)i, color, 2, LINE_8, hierarchy, 0);
		}
		imshow("3. Contours", drawing);*/


		// 6. Find index of the largest contour, assume that is the hand
		// todo: changed to highest contour instead of largest
		int largestContourIndex = findHighestContourIndex(contours);
		//cout << TAG << ": Contour index=" << largestContourIndex endl;
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "***findHighestContourIndex*** = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// 7. If index equals -1, return
		if (largestContourIndex == -1) {
			//cout << TAG << ": No hand detected" << endl;
			lowestPoint.x = -1;
			lowestPoint.y = -1;
			return;
		};

		// 8. Reduce number of points using DP algorithm
		vector<vector<Point>> reducedHandContours;
		reducedHandContours.push_back(reduceContourPoints(contours.at(largestContourIndex)));
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "reducedHandContours.push_back(reduceContourPoints(contours.at(largestContourIndex))) = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		/*Mat drawing3 = Mat::zeros(dst.size(), CV_8UC3);
		cv::RNG rng3(12345);
		for (size_t i = 0; i < reducedHandContours.size(); i++)
		{
			Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
			drawContours(drawing3, reducedHandContours, (int)i, color, 2, LINE_8, hierarchy, 0);
		}
		imshow("4. reduced", drawing3);*/


		//cout << TAG << ": contourArea(reducedHandContours.at(0)) = " << contourArea(reducedHandContours.at(0)) << endl;

		// Draw contours
		//drawContours(dst, reducedHandContours, 0, Colors::mLineColorGreen, 2);
		drawContours(dst, reducedHandContours, 0, Colors::mLineColorBlue, 2);
		//imshow("6. added reducedHandContours", dst);
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "drawContours = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// Find lowest point
		// todo: changed to reduced instead of hull.
		// todo: changed to center of contour.
		//lowestPoint = findHighestPoint(reducedHandContours.at(0));
		lowestPoint = findCenterPoint(reducedHandContours.at(0));
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "findCenterPoint = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// Draw lowest point
		if (lowestPoint != Point(-1, -1)) {
			circle(dst, lowestPoint, 2, (Colors::mLineColorGreen + Colors::mLineColorBlue), -1);
#if SEARCH_BOTTLENECK == true
			e2 = cv::getTickCount();
			time_diff = (e2 - e1) / freq;
			cout << "circle (Draw lowest point) = " << time_diff << endl;
			total_time += time_diff;
#endif
		}

#if SEARCH_BOTTLENECK == true
		cout << endl;
		cout << "total_time = " << total_time << endl;
		total_time = 0;
		cout << "-------------------------------------------" << endl;
#endif

	}


	void apply_red(const Mat& dst, const Mat& src) {
		vector<vector<Point>> contours;

#if SEARCH_BOTTLENECK == true
		cout << endl << endl << endl;
		cout << "-------------------------------------------" << endl;
		cout << "Apply red performance:" << endl << endl;
		e1 = e1 = cv::getTickCount();
#endif
		// 1. Convert image to HSV color space
		cvtColor(src, mMat, COLOR_BGR2HSV);
#if SEARCH_BOTTLENECK == true
		double time_diff = 0;
		double total_time = 0;
		freq = cv::getTickFrequency();
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "cvtColor(.., RGB2HSB) = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// 2. Apply static red threshold
		
		Mat red_mask_1;
		inRange(mMat, redLowerThreshold1, redUpperThreshold1, red_mask_1);
		Mat red_mask_2;
		inRange(mMat, redLowerThreshold2, redUpperThreshold2, red_mask_2);
		mMat = red_mask_1 + red_mask_2;

		//inRange(mMat, blueLowerThreshold, blueUpperThreshold, mMat);
		

		//imshow("1. threshold mask", mMat);
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "inRange(red_mask) = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// 3a. Perform dilation
		// dilate(mMat, mMat, Mat()); //(commented in source)

		// 3a. Perform erosion
		erode(mMat, mMat, Mat());
		//imshow("2. mask erode", mMat);
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "erode = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// 4. Find contours
		vector<Vec4i> hierarchy;
		//todo: changed RETR_LIST to RETR_EXTERNAL
		//findContours(mMat, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);
		findContours(mMat, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "findContours = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// 5. If no contours, return
		if (contours.size() == 0) {
			//cout << TAG << ": No contours found" << endl;
			lowestPoint.x = -1;
			lowestPoint.y = -1;
			return;
		}


		/*Mat drawing = Mat::zeros(dst.size(), CV_8UC3);
		cv::RNG rng(12345);
		for (size_t i = 0; i < contours.size(); i++)
		{
			Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
			drawContours(drawing, contours, (int)i, color, 2, LINE_8, hierarchy, 0);
		}
		imshow("3. Contours", drawing);*/


		// 6. Find index of the largest contour, assume that is the hand
		// todo: changed to highest contour instead of largest
		int largestContourIndex = findHighestContourIndex(contours);
		//cout << TAG << ": Contour index=" << largestContourIndex endl;
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "***findHighestContourIndex*** = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// 7. If index equals -1, return
		if (largestContourIndex == -1) {
			//cout << TAG << ": No hand detected" << endl;
			lowestPoint.x = -1;
			lowestPoint.y = -1;
			return;
		};

		// 8. Reduce number of points using DP algorithm
		vector<vector<Point>> reducedHandContours;
		reducedHandContours.push_back(reduceContourPoints(contours.at(largestContourIndex)));
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "reducedHandContours.push_back(reduceContourPoints(contours.at(largestContourIndex))) = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		/*Mat drawing3 = Mat::zeros(dst.size(), CV_8UC3);
		cv::RNG rng3(12345);
		for (size_t i = 0; i < reducedHandContours.size(); i++)
		{
			Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
			drawContours(drawing3, reducedHandContours, (int)i, color, 2, LINE_8, hierarchy, 0);
		}
		imshow("4. reduced", drawing3);*/


		//cout << TAG << ": contourArea(reducedHandContours.at(0)) = " << contourArea(reducedHandContours.at(0)) << endl;

		// Draw contours
		//drawContours(dst, reducedHandContours, 0, Colors::mLineColorGreen, 2);
		drawContours(dst, reducedHandContours, 0, Colors::mLineColorBlue, 2);
		//imshow("6. added reducedHandContours", dst);
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "drawContours = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// Find lowest point
		// todo: changed to reduced instead of hull.
		// todo: changed to center of contour.
		//lowestPoint = findHighestPoint(reducedHandContours.at(0));
		lowestPoint = findCenterPoint(reducedHandContours.at(0));
#if SEARCH_BOTTLENECK == true
		e2 = cv::getTickCount();
		time_diff = (e2 - e1) / freq;
		cout << "findCenterPoint = " << time_diff << endl;
		e1 = cv::getTickCount();
		total_time += time_diff;
#endif

		// Draw lowest point
		if (lowestPoint != Point(-1,-1)) {
			circle(dst, lowestPoint, 2, (Colors::mLineColorGreen + Colors::mLineColorBlue), -1);
#if SEARCH_BOTTLENECK == true
			e2 = cv::getTickCount();
			time_diff = (e2 - e1) / freq;
			cout << "circle (Draw lowest point) = " << time_diff << endl;
			total_time += time_diff;
#endif
		}

#if SEARCH_BOTTLENECK == true
		cout << endl;
		cout << "total_time = " << total_time << endl;
		total_time = 0;
		cout << "-------------------------------------------" << endl;
#endif
		
	}

	void apply(const Mat &dst, const Mat &src) {
		//	List<vector<Point>> contours = new ArrayList<vector<Point>>();
		vector<vector<Point>> contours;

		// 1. Convert image to HSV color space
		//Imgproc.cvtColor(src, mMat, Imgproc.COLOR_RGB2HSV);
		//cvtColor(src, mMat, COLOR_RGB2HSV);
		cvtColor(src, mMat, COLOR_BGR2HSV);
		
		// 2. Apply static skin color threshold
		//Core.inRange(mMat, lowerThreshold, upperThreshold, mMat);
		//todo: added range for red
		/*Mat red_mask_1;
		inRange(mMat, redLowerThreshold1, redUpperThreshold1, red_mask_1);
		Mat red_mask_2;
		inRange(mMat, redLowerThreshold1, redUpperThreshold1, red_mask_2);
		mMat = red_mask_1 + red_mask_2;*/
		inRange(mMat, lowerThreshold, upperThreshold, mMat);

		//imshow("1. threshold mask", mMat);
		
		// 3a. Perform dilation
		//	//Imgproc.dilate(mMat, mMat, new Mat());
		// (commented in source)

		// 3a. Perform erosion
		//	Imgproc.erode(mMat, mMat, new Mat());
		erode(mMat, mMat, Mat());
		//imshow("2. mask erode", mMat);

		// 4. Find contours
		//	Imgproc.findContours(mMat, contours, new Mat(), Imgproc.RETR_LIST, Imgproc.CHAIN_APPROX_SIMPLE);
		vector<Vec4i> hierarchy;
		//todo: changed RETR_LIST to RETR_EXTERNAL
		//findContours(mMat, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);
		findContours(mMat, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		// 5. If no contours, return
		if (contours.size() == 0) {
			//Log.i(TAG, "No contours found");
			lowestPoint.x = -1;
			lowestPoint.y = -1;
			return;
		}


		/*Mat drawing = Mat::zeros(dst.size(), CV_8UC3);
		cv::RNG rng(12345);
		for (size_t i = 0; i < contours.size(); i++)
		{
			Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
			drawContours(drawing, contours, (int)i, color, 2, LINE_8, hierarchy, 0);
		}
		imshow("3. Contours", drawing);*/


		// 6. Find index of the largest contour, assume that is the hand
		// todo: changed to highest contour instead of largest
		int largestContourIndex = findLargestContourIndex(contours);
		//int largestContourIndex = findHighestContourIndex(contours);
		//cout << "Contour index = " << largestContourIndex << endl;

		// 7. If index equals -1, return
		if (largestContourIndex == -1 || contourArea(contours.at(largestContourIndex)) < handArea) {
			//Log.i(TAG, "No hand detected");
			lowestPoint.x = -1;
			lowestPoint.y = -1;
			return;
		};
		
		// 8. Reduce number of points using DP algorithm
		vector<vector<Point>> reducedHandContours;
		reducedHandContours.push_back(reduceContourPoints(contours.at(largestContourIndex)));
		
		/*Mat drawing3 = Mat::zeros(dst.size(), CV_8UC3);
		cv::RNG rng3(12345);
		for (size_t i = 0; i < reducedHandContours.size(); i++)
		{
			Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
			drawContours(drawing3, reducedHandContours, (int)i, color, 2, LINE_8, hierarchy, 0);
		}
		imshow("4. reduced", drawing3);*/


		// //Log.i(TAG, Double.toString(Imgproc.contourArea(reducedHandContours.get(0))));
		//cout << TAG << ": contourArea(reducedHandContours.at(0)) = " << contourArea(reducedHandContours.at(0)) << endl;

		// 9. Get convex hull of hand
		vector<int> hullMOI;
		convexHull(reducedHandContours.at(0), hullMOI);

		////todo: added this to sort hull
		//for (size_t k = 0; k < hullMOI.size(); k++) //Here we resort the indices
		//{
		//	if (hullMOI.size() > 0)
		//	{
		//		hullMOI[k] = (int)((hullMOI.size() - k) - 1);
		//	}
		//}

		// 10. Convert hull to contours
		vector<vector<Point>> hullContourLMOP;
		hullContourLMOP.push_back(hullToContour(hullMOI, reducedHandContours.at(0)));
		
		/*Mat drawing2 = Mat::zeros(dst.size(), CV_8UC3);
		RNG rng2(12345);
		for (size_t i = 0; i < hullContourLMOP.size(); i++)
		{
			Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
			drawContours(drawing2, hullContourLMOP, (int)i, color, 2, LINE_8, hierarchy, 0);
		}
		imshow("5. Hull", drawing2);*/

		// 11. Draw convex hull points
		/*for(int i=0; i<hullContourLMOP.get(0).rows(); i++) {
			Point p = new Point(hullContourLMOP.get(0).get(i, 0));
			Imgproc.circle(dst, p, 10, Colors.mLineColorGreen, 2);
		}*/
		/* Original is commented */

		// 12. Find convex hull points that are within piano area
		// (Create new method)
		// getPointsByRegion();
		/* Original is commented */

		//	// 13. Reduce convex hull points to (maximum) 5 distinct points
		//	// to correspond to 5 finger tips (Create new method)
		//	// getFingerTipPoints();
		/* Original is commented */

		// 11. Get convexity defects
		//todo: commented this
		//vector<Vec4i> convDefMOI4;
		//convexityDefects(reducedHandContours.at(0), hullMOI, convDefMOI4);
		
		// Draw contours
		//Imgproc.drawContours(dst, contours, largestContourIndex, Colors.mLineColorGreen, 2);
		//Imgproc.drawContours(dst, reducedHandContours, 0, Colors.mLineColorRed, 2);
		//todo: changed this to reduced instead of hull
		drawContours(dst, hullContourLMOP, 0, Colors::mLineColorBlue, 2);
		//imshow("6. added hullContourLMOP", dst);
		//drawContours(dst, reducedHandContours, 0, Colors::mLineColorBlue, 2);
		//imshow("6. added reducedHandContours", dst);


		// Draw convexity defect points
		//todo: commented this
		//if (!convDefMOI4.empty()) {
		//	//vector<int> cdList = convDefMOI4.toList();
		//	vector<int> cdList;
		//	for (int i = 0; i < convDefMOI4.size(); i++) {
		//		//mWhiteKeysLP.addAll(mWhiteKeysLMOP.get(i).toList());
		//		Vec4i myElement = convDefMOI4.at(i);
		//		cdList.push_back(myElement[0]);
		//		cdList.push_back(myElement[1]);
		//		cdList.push_back(myElement[2]);
		//		cdList.push_back(myElement[3]);
		//	}

		//	vector<Point> data_v = reducedHandContours.at(0);
		//	Point* data = &data_v[0];
		//	//Point data[] = reducedHandContours.at(0);

		//	for (int i = 0; i < cdList.size(); i += 4) {
		//		Point start = data[cdList.at(i)];
		//		Point end = data[cdList.at(i + 1)];
		//		Point defect = data[cdList.at(i + 2)];

		//		//Imgproc.circle(dst, start, 15, Colors.mLineColorGreen, 2);
		//		//Imgproc.circle(dst, end, 20, Colors.mLineColorRed, 2);
		//		//Imgproc.circle(dst, defect, 10, Colors.mLineColorYellow, 2);
		//		
		//		/*circle(dst, start, 15, Colors::mLineColorGreen, 2);
		//		circle(dst, end, 20, Colors::mLineColorBlue, 2);
		//		circle(dst, defect, 10, Colors::mLineColorYellow, 2);*/
		//	}
		//}

		// Find lowest point
		// todo: changed to reduced instead of hull. change back?
		// todo: added lowestPoints.
		////lowestPoint = findLowestPoint(hullContourLMOP.get(0));
		//lowestPoint = findHighestPoint(hullContourLMOP.at(0));
		lowestPoints = reduceContourPoints(hullContourLMOP.at(0));
		//lowestPoint = findHighestPoint(reducedHandContours.at(0));

		// Draw lowest point
		//todo: circles of contour instead of lowestPoint
		/*if (lowestPoint != Point(-1,-1)) {
			circle(dst, lowestPoint, 5, Colors::mLineColorRed, -1);
		}*/
		for (int i = 0; i < lowestPoints.size(); i++) {
			circle(dst, lowestPoints[i], 5, Colors::mLineColorRed, -1);
		}
	}


	vector<vector<Point>> getHandContours() {
		return contoursOut;
	}

	Point getLowestPoint() {
		return lowestPoint;
	}

	vector<Point> getLowestPoints() {
		return lowestPoints;
	}
};

const string HandDetector::TAG("HandDetector");