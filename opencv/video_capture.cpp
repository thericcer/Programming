#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc.hpp>

int color = 0;
int color_sat = 0;
int color_val = 0;
int thresh = 20;
cv::Mat raw;
cv::Mat hsv;
cv::Mat bw;
cv::Mat edges;
cv::Mat red;
cv::Mat output;


int main(int argc, char ** argv) {

	if (argc != 5) {
		printf("Usage video.elf <color> <color_sat> <color_val> <canny_thresh>\n");
		return -1;
	}

	color = atoi(argv[1]);
	color_sat = atoi(argv[2]);
	color_val = atoi(argv[3]);
	thresh = atoi(argv[4]);

	printf("Using color threshold: %d\nUsing color sat: %d\nUsing Canny Threshold: %d\n", color, color_sat, thresh);

	cv::VideoCapture cap(0);
	if(!cap.isOpened()){
		return -1;
	}

	for(;;){
		cap >> raw;
		cv::cvtColor(raw, hsv, CV_BGR2HSV);
		cv::cvtColor(raw, bw, CV_BGR2GRAY);

		cv::Scalar lower_thresh(color, color_sat, color_val);
		cv::Scalar upper_thresh(color+10, color_sat+50, color_val+50);
		cv::inRange(hsv, lower_thresh, upper_thresh, red);

		cv::blur(bw, edges, cv::Size(3,3));
		cv::Canny(bw, edges,  thresh, thresh*3);
		cv::bitwise_and(raw, raw, output, red);
		cv::bitwise_or(red, edges, output, edges);

	//	cv::cvtColor(red, red, CV_HSV2BGR);
	//	cv::blur(red, output, cv::Size(3,3));
	//	cv::Canny(red, output,  thresh, thresh*3);

		cv::imshow("raw", raw);
		//cv::imshow("hsv",hsv);
		cv::imshow("edges", edges);
//		cv::imshow("output", output);
		cv::imshow("red", red);
		output = cv::Scalar(0,0,0);

		if(cv::waitKey(30) >=0) break;
	}
	return 0;
}

