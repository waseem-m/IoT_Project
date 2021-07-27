#pragma once

#include "MyIncludes.h"

//contours:	Detected contours.
//			Each contour is stored as a vector of points
//			(e.g. std::vector<std::vector<cv::Point> >).

using namespace cv;

class Detector {
public:
	void apply(const Mat src, const Mat dst) {
	}

	//void drawAllContours(const Mat dst, list<vector<Point>> contours) {
	void drawAllContours(const Mat dst, vector<vector<Point>> contours) {
		for (int i = 0; i < contours.size(); i++) {
			cv::drawContours(dst, contours, i, Colors::mLineColorGreen, -1);
			//Imgproc.drawContours(dst, contours, i, Colors.mLineColorGreen, -1);

		}
	}

	//public int findLargestContourIndex(List<vector<Point>> contours) {
	int findLargestContourIndex(vector<vector<Point>> contours) {
		int index = -1;
		double maxArea = 0;

		for (int i = 0; i < contours.size(); i++) {
			double thisArea = contourArea(contours.at(i));
			if (thisArea > maxArea) {
				index = i;
				maxArea = thisArea;
			}
		}

		return index;
	}

	//public vector<Point> reduceContourPoints(vector<Point> contours) {
	vector<Point> reduceContourPoints(vector<Point> contours) {
		//vector<Point>2f approxCurve = new vector<Point>2f();
		vector<Point> approxCurve;
		//vector<Point>2f contour2f(contours.toArray());

		double approxDistance = arcLength(contours, true) * 0.01;

		approxPolyDP(contours, approxCurve, approxDistance, true);

		vector<Point> points(approxCurve);

		return points;
	}

	/*
	template<class T>
	Mat VectorOfVector_to_Mat(vector<vector<T>> my_Vecvector<Point>) {
		cv::Mat my_Mat(my_Vecvector<Point>.size(), my_Vecvector<Point>.at(0).size(), CV_64FC1);
		for (int i = 0; i < my_Mat.rows; ++i)
			for (int j = 0; j < my_Mat.cols; ++j)
				my_Mat.at<T>(i, j) = my_Vecvector<Point>.at(i).at(j);
	}
	*/
	
	
	vector<Point> hullToContour(vector<int> hullMOI, vector<Point> contourMOP) {

		vector<Point> mopOut;
		//mopOut.create((int) hullMOI.size().height, 1, CV_32SC2);
		
		for(int i=0; i<hullMOI.size(); i++) {
			
			int index = (int) hullMOI.at(i);
			Point point = { contourMOP.at(index).x, contourMOP.at(index).y };
			
			mopOut.push_back(point);
			
			//Point x = new Point(point[0], point[1]);
			//Log.i(TAG, "Point " + i + ": " + point[0] + ", " + point[1]);
		}
		
		return mopOut;
	}
	
};