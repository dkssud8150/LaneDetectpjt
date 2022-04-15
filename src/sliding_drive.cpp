#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

Vec4f n_window_sliding(int left_start, int right_start, Mat roi, Mat v_thres, int w, int h, vector<Vec4i>& prev_lwindow, vector<Vec4i>& prev_rwindow, Vec4f left_line, Vec4f right_line) {
	// define constant for sliding window
	int nwindows = 12;
	int window_height = (int)(h / nwindows);
	int window_width = (int)(w / nwindows * 1.5);

	int margin = window_width / 2;
	
	// 좌우 둘 다 인식이 되지 않을 경우 지난 화면의 윈도우를 사용
	if (left_start == 0 && right_start == 960) {
		for (auto i = 0; i <= prev_lwindow.size(); i++) {
			int win_y_high = prev_lwindow[i][0];
			int win_x_leftb_right = prev_lwindow[i][1];
			int win_y_low = prev_lwindow[i][2];
			int win_x_leftb_left = prev_lwindow[i][3];

			int win_x_rightb_right = prev_rwindow[i][1];
			int win_x_rightb_left = prev_rwindow[i][3];

			rectangle(roi, Rect(win_x_leftb_left, win_y_high, window_width, window_height), Scalar(0, 150, 0), 2);
			rectangle(roi, Rect(win_x_rightb_left, win_y_high, window_width, window_height), Scalar(150, 0, 0), 2);
		}



		return left_line, right_line;
	}

	// 양쪽이 인식이 되었다면 초기화하고 다시 입력
	left_line = Vec4f(0.0, 0.0, 0.0, 0.0);
	right_line = Vec4f(0.0, 0.0, 0.0, 0.0);
	vector<Point> lpoints(nwindows), rpoints(nwindows), mpoints(nwindows);

	// init value setting
	int lane_mid = w / 2;

	int win_y_high = h - window_height;
	int win_y_low = h;

	int win_x_leftb_right = left_start + margin;
	int win_x_leftb_left = left_start - margin;

	int win_x_rightb_right = right_start + margin;
	int win_x_rightb_left = right_start - margin;

	lpoints[0] = Point(left_start, (int)((win_y_high + win_y_low) / 2));
	rpoints[0] = Point(right_start, (int)((win_y_high + win_y_low) / 2));
	mpoints[0] = Point((int)((left_start + right_start) / 2), (int)((win_y_high + win_y_low) / 2));

	prev_lwindow[0] = (Vec4i(win_y_high, win_x_leftb_right, win_y_low, win_x_leftb_left));
	prev_rwindow[0] = (Vec4i(win_y_high, win_x_rightb_right, win_y_low, win_x_rightb_left));

	// init box draw
	rectangle(roi, Rect(win_x_leftb_left, win_y_high, window_width, window_height), Scalar(0, 150, 0), 2);
	rectangle(roi, Rect(win_x_rightb_left, win_y_high, window_width, window_height), Scalar(150, 0, 0), 2);



	// window search start, i drew the init box at the bottom, so i start from 1 to nwindows
	for (int window = 1; window < nwindows; window++) {

		win_y_high = h - (window + 1) * window_height;
		win_y_low = h - window * window_height;

		win_x_leftb_right = left_start + margin;
		win_x_leftb_left = left_start - margin;

		win_x_rightb_right = right_start + margin;
		win_x_rightb_left = right_start - margin;


		// i will detect the lane at top of box
//		int offset = win_y_high + 4;
		// i will detect the lane at bottom of box
//		int offset = win_y_low - 4;
		// i will detect the lane at the mid of box 
		int offset = (int)((win_y_high + win_y_low) / 2);

		int pixel_thres = window_width * 0.2;

		int ll = 0, lr = 0; int rl = 960, rr = 960;
		int li = 0; // nonzero가 몇개인지 파악하기 위한 벡터에 사용될 인자
		// window의 위치를 고려해서 벡터에 집어넣으면 불필요한 부분이 많아질 수 있다. 어차피 0의 개수를 구하기 위한 벡터이므로 0부터 window_width+1 개수만큼 생성
		vector<int> lhigh_vector(window_width + 1); // nonzero가 몇개 인지 파악할 때 사용할 벡터
		for (auto x = win_x_leftb_left; x < win_x_leftb_right; x++) {
			li++;
			lhigh_vector[li] = v_thres.at<uchar>(offset, x);

			// 차선의 중앙을 계산하기 위해 255 시작점과 255 끝점을 계산
			if (v_thres.at<uchar>(offset, x) == 255 && ll == 0) {
				ll = x;
				lr = x;
			}
			if (v_thres.at<uchar>(offset, x) == 255 && lr != 0) {
				lr = x;
			}
		}

		int ri = 0;
		vector<int> rhigh_vector(window_width + 1);
		for (auto x = win_x_rightb_left; x < win_x_rightb_right; x++) {
			ri++;
			rhigh_vector[ri] = v_thres.at<uchar>(offset, x);
			if (v_thres.at<uchar>(offset, x) == 255 && rl == 960) {
				rl = x;
				rr = x;
			}
			if (v_thres.at<uchar>(offset, x) == 255 && lr != 960) {
				rr = x;
			}
		}

		// window안에서 0이 아닌 픽셀의 개수를 구함
		int lnonzero = countNonZero(lhigh_vector);
		int rnonzero = countNonZero(rhigh_vector);


		// 255인 픽셀의 개수가 threshold를 넘으면, 방금 구했던 255 픽셀 시작 지점과 끝 지점의 중앙 값을 다음 window의 중앙으로 잡는다.
		if (lnonzero >= pixel_thres) {
			left_start = (ll + lr) / 2;
		}
		if (rnonzero >= pixel_thres) {
			right_start = (rl + rr) / 2;
		}

		// 차선 중앙과 탐지한 차선과의 거리 측정
		int lane_mid = (right_start + left_start) / 2;
		int left_diff = lane_mid - left_start;
		int right_diff = -(lane_mid - right_start);


		// 한쪽 차선의 nonzero가 임계값을 넘지 못할 경우 중간을 기점으로 반대편 차선 위치를 기준으로 대칭
		if (lnonzero < pixel_thres && rnonzero > pixel_thres) {
			left_start = lane_mid - right_diff;
			lane_mid = right_start - right_diff;
		}
		else if (lnonzero > pixel_thres && rnonzero < pixel_thres) {
			right_start = lane_mid + left_diff;
			lane_mid = left_start + left_diff;
		}



		// draw window at v_thres
		rectangle(roi, Rect(win_x_leftb_left, win_y_high, window_width, window_height), Scalar(0, 150, 0), 2);
		rectangle(roi, Rect(win_x_rightb_left, win_y_high, window_width, window_height), Scalar(150, 0, 0), 2);



		mpoints[window] = Point(lane_mid, (int)((win_y_high + win_y_low) / 2));
		lpoints[window] = Point(left_start, (int)((win_y_high + win_y_low) / 2));
		rpoints[window] = Point(right_start, (int)((win_y_high + win_y_low) / 2));

		// 윈도우를 prev_window에 저장하여 다음 프레임에서 맨 아래 부분이 인식이 안되면 지난 거를 사용한다.
		prev_lwindow.push_back(Vec4i(win_y_high, win_x_leftb_right, win_y_low, win_x_leftb_left));
		prev_rwindow.push_back(Vec4i(win_y_high, win_x_rightb_right, win_y_low, win_x_rightb_left));
	}

	Vec4f mid_line;
	fitLine(lpoints, left_line, DIST_L2, 0, 0.01, 0.01); // 출력의 0,1 번째 인자는 단위벡터, 3,4번째 인자는 선 위의 한 점
	fitLine(rpoints, right_line, DIST_L2, 0, 0.01, 0.01);
	fitLine(mpoints, mid_line, DIST_L2, 0, 0.01, 0.01);

	// 방향이 항상 아래를 향하도록 만들기 위해 단위 벡터의 방향을 바꿔준다.
	if (left_line[1] > 0) {
		left_line[1] = -left_line[1];
	}
	if (right_line[1] > 0) {
		right_line[1] = -right_line[1];
	}
	if (mid_line[1] > 0) {
		mid_line[1] = -mid_line[1];
	}

	int lx0 = left_line[2], ly0 = left_line[3]; // 선 위의 한 점
	int lx1 = lx0 + h * left_line[0], ly1 = ly0 + h * left_line[1]; // 단위 벡터 -> 그리고자 하는 길이를 빼주거나 더해줌
	int lx2 = 2 * lx0 - lx1, ly2 = 2 * ly0 - ly1;

	int rx0 = right_line[2], ry0 = right_line[3];
	int rx1 = rx0 + h * right_line[0], ry1 = ry0 + h * right_line[1];
	int rx2 = 2 * rx0 - rx1, ry2 = 2 * ry0 - ry1;

	int mx0 = mid_line[2], my0 = mid_line[3];
	int mx1 = mx0 + h * mid_line[0], my1 = my0 + h * mid_line[1];
	int mx2 = 2 * mx0 - mx1, my2 = 2 * my0 - my1;

	line(roi, Point(lx1, ly1), Point(lx2, ly2), Scalar(0, 0, 255), 3);
	line(roi, Point(rx1, ry1), Point(rx2, ry2), Scalar(0, 0, 255), 3);
	line(roi, Point(mx1, my1), Point(mx2, my2), Scalar(0, 0, 255), 3);

	return left_line, right_line, mid_line;
}

void draw_line(Mat frame, Mat roi, Vec4f left_line, Vec4f right_line, Mat per_mat_tosrc, int width = 960, int height = 480) {
	Mat newframe;
	warpPerspective(roi, newframe, per_mat_tosrc, Size(width, height), INTER_LINEAR);

	//	imshow("newframe", newframe);
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
	int w = (int)width * 1.5, h = (int)height * 1.5;

	// point about warp transform
	vector<Point2f> src_pts(4);
	vector<Point2f> dst_pts(4);

	// 파란색 선 보이게 하는 roi
//	src_pts[0] = Point2f(0, 420); src_pts[1] = Point2f(213, 280); src_pts[2] = Point2f(395, 280); src_pts[3] = Point2f(595, 420);

	// 파란색 선 없는 roi
	src_pts[0] = Point2f(0, 395); src_pts[1] = Point2f(198, 280); src_pts[2] = Point2f(403, 280); src_pts[3] = Point2f(580, 395);
	dst_pts[0] = Point2f(0, h - 1); dst_pts[1] = Point2f(0, 0); dst_pts[2] = Point2f(w - 1, 0); dst_pts[3] = Point2f(w - 1, h - 1);

	// point about polylines
	vector<Point> pts(4);
	pts[0] = Point(src_pts[0]); pts[1] = Point(src_pts[1]); pts[2] = Point(src_pts[2]); pts[3] = Point(src_pts[3]);

	//	pts[0] = Point(0, 420); pts[1] = Point(213, 280); pts[2] = Point(395, 280); pts[3] = Point(595, 420);


	Mat per_mat_todst = getPerspectiveTransform(src_pts, dst_pts);
	Mat per_mat_tosrc = getPerspectiveTransform(dst_pts, src_pts);

	Mat frame, roi;
	Vec4f left_line, right_line;
	while (true) {
		cap >> frame;

		if (frame.empty()) break;

		// perspective transform
		Mat roi;
		warpPerspective(frame, roi, per_mat_todst, Size(w, h), INTER_LINEAR);

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
		threshold(v_plane, v_thres, lane_binary_thres, 255, THRESH_OTSU);
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
		Mat hsv;
		Mat v_thres = Mat::zeros(w, h, CV_8UC1);
		int lane_binary_thres = 125; // contrast : 155
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
		v_plane = v_plane + (100 - means);

		GaussianBlur(v_plane, v_plane, Size(), 1.0);

		inRange(v_plane, lane_binary_thres, 255, v_thres);
		// OTSU 알고리즘
//		threshold(v_plane, v_thres, lane_binary_thres, 255, THRESH_OTSU);

//		imshow("hsv", hsv);
//		imshow("v_plane", v_plane);
		imshow("v_thres", v_thres);


#else 0	// 3-1 lab -> gaussian -> inrange -> canny
		Mat lab, lab_thres, roi_edge;
		int lane_binary_thres = 130;
		cvtColor(roi, lab, COLOR_BGR2Lab);

		GaussianBlur(roi, roi, Size(), 1.0);

		inRange(lab, Scalar(0, 0, lane_binary_thres), Scalar(255, 255, 255), lab_thres); //00130
		Canny(lab_thres, lab_edge, 50, 150);

		imshow("lab", lab);
		imshow("lab_thres", lab_thres);
		imshow("lab_edge", roi_edge);

#endif	
		// 첫위치 지정
		int cnt = 0;
		int left_l_init = 0, left_r_init = 0;
		int right_l_init = 960, right_r_init = 960;
		for (auto x = 0; x < w; x++) {
			if (x < w / 2) {
				if (v_thres.at<uchar>(h - 1, x) == 255 && left_l_init == 0) {
					left_l_init = x;
					left_r_init = x;
				}
				if (v_thres.at<uchar>(h - 1, x) == 255 && left_r_init != 0) {
					left_r_init = x;
				}
			}
			else {
				if (v_thres.at<uchar>(h - 1, x) == 255 && right_l_init == 960) {
					right_l_init = x;
					right_r_init = x;
				}
				if (v_thres.at<uchar>(h - 1, x) == 255 && right_r_init != 960) {
					right_r_init = x;
				}
			}
		}

		int left_start = (left_l_init + left_r_init) / 2;
		int right_start = (right_l_init + right_r_init) / 2;

		vector<Vec4i> prev_lwindow(1), prev_rwindow(1);

		left_line, right_line = n_window_sliding(left_start, right_start, roi, v_thres, w, h, prev_lwindow, prev_rwindow, left_line, right_line);

		draw_line(frame, roi, left_line, right_line, per_mat_tosrc);

		imshow("src", frame);
		imshow("roi", roi);

		if (waitKey(10) == 27) break;

	}
	cap.release();
	destroyAllWindows();
}
