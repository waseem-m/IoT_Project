#pragma once

#include <opencv2/core/types.hpp>

using cv::Scalar;

class Colors {
public:
	static const Scalar mLineColorRed;
	static const Scalar mLineColorGreen;
	static const Scalar mLineColorBlue;
	static const Scalar mLineColorBlack;
	static const Scalar mLineColorYellow;
	static const Scalar mLineColorWhite;
};

const Scalar Colors::mLineColorRed(255, 0, 0);
const Scalar Colors::mLineColorGreen(0, 255, 0);
const Scalar Colors::mLineColorBlue(0, 0, 255);
const Scalar Colors::mLineColorBlack(0, 0, 0);
const Scalar Colors::mLineColorYellow(255, 255, 0);
const Scalar Colors::mLineColorWhite(255, 255, 255);