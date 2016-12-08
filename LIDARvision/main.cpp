#include <opencv2\opencv.hpp>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

using namespace cv;
using namespace std;

/*void main() {
	Mat dsm = imread("dsm.png");
	Mat dtm = imread("dtm.png");
	if (!dsm.data || !dtm.data) {
		printf("Could not load image(s)");
		waitKey(0);
		return;
	}

	Mat graydsm;
	cvtColor(dsm, graydsm, CV_RGB2GRAY);

	Mat image;
	subtract(dsm, dtm, image, graydsm != 0);

	Mat gray, edges;
	cvtColor(image, gray, CV_RGB2GRAY);
	Canny(gray, edges, 150, 300);

	Mat labels = Mat::zeros(image.size(), CV_8UC1);
	Mat meanimage = Mat::ones(image.size(), image.type());
	meanimage *= 255;
	vector<vector<Point>> contours;
	findContours(edges, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	vector<float> avgs(contours.size(), 0.0f);

	printf("%i", contours.size());
	for (size_t i = 0; i < contours.size(); ++i)
	{
		drawContours(labels, contours, i, Scalar(i), CV_FILLED);
		Rect roi = boundingRect(contours[i]);
		Scalar mean = cv::mean(image(roi), labels(roi) == i);
		avgs[i] = mean[0];

		drawContours(meanimage, contours, i, mean, CV_FILLED);
	}

	imshow("DSM", dsm);
	imshow("DTM", dtm);
	imshow("LIDAR", image);
	imshow("Mask", graydsm != 0);
	imshow("Edges", edges);
	imshow("Mean", meanimage);
	
	waitKey(0);
}*/

#define HEIGHT_TRANSFORM 20

Mat asc(char* filename) {
	ifstream file(filename);

	string tmp;
	int cols, rows, xll, yll, nop;
	float cellsize;
	file >> tmp >> cols >> tmp >> rows >> tmp >> xll >> tmp >> yll >> tmp >> cellsize >> tmp >> nop;
	printf("cols\t= %i\nrows\t= %i\nxll\t= %i\nyll\t= %i\nnop\t= %i\n", cols, rows, xll, yll, nop);

	Mat img(rows, cols, CV_32FC1);

	float h;
	int x = 0, y = 0;
	while (file >> h) {
		++x;
		if (x >= cols) {
			++y;
			x = 0;
		}
		if (y >= rows) break;

		img.at<float>(y, x) = (h == nop) ? 0 : h;
	}

	file.close();
	return img;
}

void main() {
	Mat dsm_mat = asc("dsm.asc");
	Mat dtm_mat = asc("dtm.asc");

	Mat gray_32f;
	subtract(dsm_mat, dtm_mat, gray_32f);
	Mat mask = (gray_32f > 2);
	imshow("Mask", mask);

	vector<vector<Point>> contours;
	findContours(mask, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

	vector<vector<Point>> finals;
	for (unsigned int i = 0; i < contours.size(); ++i) {
		if (contourArea(contours[i]) > 10) {
			vector<Point> c;
			approxPolyDP(contours[i], c, 0.1, true);
			finals.push_back(c);
		}
	}

	Mat trace = Mat::zeros(mask.size(), mask.type());
	for (unsigned int i = 0; i < finals.size(); ++i) {
		drawContours(trace, finals, i, Scalar(128, 128, 0), CV_FILLED);
	}

	imshow("Gray", gray_32f / HEIGHT_TRANSFORM);
	imshow("Trace", trace);

	waitKey(0);
}