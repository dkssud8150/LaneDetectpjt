#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;


Mat calcGrayHist(const Mat& img)
{
	CV_Assert(img.type() == CV_8UC1);

	Mat hist;
	int channels[] = { 0 };
	int dims = 1;
	const int histSize[] = { 256 };
	float graylevel[] = { 0, 256 };
	const float* ranges[] = { graylevel };

	calcHist(&img, 1, channels, noArray(), hist, dims, histSize, ranges);

	return hist;
}

Mat getGrayHistImage(const Mat& hist)
{
	CV_Assert(hist.type() == CV_32FC1);
	CV_Assert(hist.size() == Size(1, 256));

	double histMax = 0.;
	minMaxLoc(hist, 0, &histMax);

	Mat imgHist(100, 256, CV_8UC1, Scalar(255));
	for (int i = 0; i < 256; i++) {
		line(imgHist, Point(i, 100),
			Point(i, 100 - cvRound(hist.at<float>(i, 0) * 100 / histMax)), Scalar(0));
	}

	return imgHist;
}

int main()
{
	VideoCapture cap("../data/subProject.avi");

	if (!cap.isOpened()) {
		cerr << "Camera open failed" << endl;
		return -1;
	}

	// src image size
	int width = cvRound(cap.get(CAP_PROP_FRAME_WIDTH));
	int height = cvRound(cap.get(CAP_PROP_FRAME_HEIGHT));

	// warped image size
	int w = (int)width / 3, h = (int)height / 3;

	// point about warp transform
	vector<Point2f> src_pts(4);
	vector<Point2f> dst_pts(4);
	src_pts[0] = Point2f(10, 395); src_pts[1] = Point2f(250, 250); src_pts[2] = Point2f(360, 250); src_pts[3] = Point2f(570, 395);
	dst_pts[0] = Point2f(0, h - 1); dst_pts[1] = Point2f(0, 0); dst_pts[2] = Point2f(w - 1, 0); dst_pts[3] = Point2f(w - 1, h - 1);

	// point about polylines
	vector<Point> pts(4);
	pts[0] = Point(10, 395); pts[1] = Point(250, 250); pts[2] = Point(360, 250); pts[3] = Point(570, 395);


	Mat per_mat = getPerspectiveTransform(src_pts, dst_pts);

	Mat frame, roi;
	while (true) {
		cap >> frame;

		if (frame.empty()) break;

		// perspective transform
		Mat roi;
		warpPerspective(frame, roi, per_mat, Size(w, h), INTER_LINEAR);

		// roi box indicate
		polylines(frame, pts, true, Scalar(255, 255, 0), 2);


		// binary processing
#if 0	// full shot binary
		Mat hsv, v_thres;
		int lane_binary_thres = 140;
		cvtColor(frame, hsv, COLOR_BGR2HSV);
		vector<Mat> hsv_planes;
		split(frame, hsv_planes);
		Mat v_plane = hsv_planes[2];
		v_plane = 255 - v_plane;
		int means = mean(v_plane)[0];
		v_plane = v_plane + (128 - means);
		GaussianBlur(v_plane, v_plane, Size(), 1.0);
		inRange(v_plane, lane_binary_thres, 255, v_thres);
		imshow("v_thres", v_thres);

#elif 0	// 1-1 grayscale -> gaissian -> canny
		Mat roi_gray, roi_edge;
		cvtColor(roi, roi_gray, COLOR_BGR2GRAY);
		GaussianBlur(roi_gray, roi_gray, Size(), 1.0);

		threshold(roi_gray, roi_gray, 30, 75, THRESH_BINARY);

		Canny(roi_gray, roi_edge, 50, 150);
		imshow("roi", roi_gray);
		imshow("roi", roi_edge);


#elif 1	// 2-1 hsv -> gaussian -> inRange -> canny
		Mat hsv, v_thres;
		int lane_binary_thres = 155; // contrast : 185
		cvtColor(roi, hsv, COLOR_BGR2HSV);

		// split H/S/V
		vector<Mat> hsv_planes;
		split(roi, hsv_planes);
		Mat v_plane = hsv_planes[2];

		// inverse
		v_plane = 255 - v_plane;

		// brightness control
		int means = mean(v_plane)[0];
		// v_plane.convertTo(v_plane, -1, 1.5, 128 - means); // roi = roi + (128 - m);
		v_plane = v_plane + (128 - means);

		GaussianBlur(v_plane, v_plane, Size(), 1.0);

		inRange(v_plane, lane_binary_thres, 255, v_thres);


		//		imshow("hsv", hsv);
		imshow("v_plane", v_plane);
		imshow("v_thres", v_thres);


#elif 0	// 3-1 lab -> gaussian -> inrange -> canny
		Mat lab, lab_thres, roi_edge;
		int lane_binary_thres = 130;
		cvtColor(roi, lab, COLOR_BGR2Lab);

		GaussianBlur(roi, roi, Size(), 1.0);

		inRange(lab, Scalar(0, 0, lane_binary_thres), Scalar(255, 255, 255), lab_thres); //00130
		Canny(lab_thres, lab_edge, 50, 150);

		imshow("lab", lab);
		imshow("lab_thres", lab_thres);
		imshow("lab_edge", roi_edge);


#else	// grayscale -> sobel -> threshold
		Mat roi_gray;
		cvtColor(roi, roi_gray, COLOR_BGR2GRAY);

		Mat dx, dy;
		Sobel(roi, dx, CV_32FC1, 1, 0);
		Sobel(roi, dy, CV_32FC1, 0, 1);

		Mat mag;
		magnitude(dx, dy, mag);
		mag.convertTo(mag, CV_8UC1);

		Mat roi_edge = mag > 65;

		imshow("roi", roi);
		imshow("mag", mag);
		imshow("roi_edge", roi_edge);


#endif	

		// define constant for sliding window
		int nwindows = 9;
		int margin = 12;
		int minpixel = 5;




		imshow("src", frame);
		imshow("roi", roi);

		if (waitKey(10) == 27) break;

	}
	cap.release();
	destroyAllWindows();
}
