#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc.hpp>

int main (int argc, char ** argv) {

	int canny_thresh = 50;
	int canny_thresh_r = 50;
	int min_angle = 130;
	int max_angle = 150;
	float average_angle = 0;
	float angle = 0;
	int num_valid_lines = 0;
	int num_valid_lines_r = 0;
	cv::Point valid_line_1(150, 150);
	cv::Point valid_line_2(0, 0);
	cv::Vec4i valid_line;
	cv::Vec4i valid_line_old;
	cv::Vec4i valid_line_r;
	cv::Vec4i valid_line_old_r;

	cv::VideoCapture cap(1);
	cv::VideoCapture cap_r(0);
	cv::Mat raw;
	cv::Mat raw_r;
	cv::Mat canny;
	cv::Mat canny_r;
	cv::Mat color_canny;
	cv::Mat color_canny_r;

	cv::vector<cv::Vec4i> lines;
	cv::vector<cv::Vec4i> lines_r;

	if (argc == 3) {
		canny_thresh = atoi(argv[1]);
		canny_thresh_r = atoi(argv[2]);
		printf("Using Canny Thresh: F: %d R: %d\n", canny_thresh, canny_thresh_r);
	}

	if (!cap.isOpened()){
		return -1;
	}

	if (!cap_r.isOpened()){
		return -1;
	}

	for(;;){
		cap >> raw;
		cap_r >> raw_r;
		cv::flip(raw, raw, -1);
		cv::flip(raw_r, raw_r, 0);
		
		cv::Canny(raw, canny, canny_thresh, canny_thresh*3);
		cv::cvtColor(canny, color_canny, CV_GRAY2BGR);

		cv::Canny(raw_r, canny_r, canny_thresh_r, canny_thresh_r*3);
		cv::cvtColor(canny_r, color_canny_r, CV_GRAY2BGR);

		cv::HoughLinesP(canny, lines, 1, CV_PI/180, 90, 50, 10);

		cv::HoughLinesP(canny_r, lines_r, 1, CV_PI/180, 50, 100, 10);

		num_valid_lines = 0;
		num_valid_lines_r = 0;

		for (size_t i = 0; i < lines.size(); i++){
			cv::Vec4i line = lines[i];
			angle = ((atan2(line[1]-line[3], line[0]-line[2]))*(180/CV_PI));
			if ((angle  > min_angle) && 
			    (angle  < max_angle)){
			 
				if ((line[0] > 50 && line[2] > 50) &&  (line[0] < 380 && line[2] < 380) &&
				    (line[1] > 150 && line[3] > 150) && (line[1] < 380 && line[3] < 380)) {

					cv::line(color_canny, cv::Point(line[0], line[1]),
						 		cv::Point(line[2], line[3]),
								cv::Scalar(0,255,0),
								3, CV_AA);
					cv::line(raw, cv::Point(line[0], line[1]),
						 		cv::Point(line[2], line[3]),
								cv::Scalar(0,255,0),
								3, CV_AA);
					num_valid_lines++;
					if ((line[1] > 150) && (line[3] > 150) && (line[3] < 300)){
						angle = ((atan2(line[1]-line[3], line[0]-line[2]))*(180/CV_PI));
						average_angle = (average_angle + angle) / 2;
						valid_line_1.x = (valid_line_1.x + line[2]) / 2;
						valid_line_1.y = (valid_line_1.y + line[3]) / 2;
					}
				} else {
					cv::line(color_canny, cv::Point(line[0], line[1]),
						 		cv::Point(line[2], line[3]),
								cv::Scalar(0,0,255),
								3, CV_AA);
					cv::line(raw, cv::Point(line[0], line[1]),
						 		cv::Point(line[2], line[3]),
								cv::Scalar(0,0,255),
								3, CV_AA);
				}
			}
		}
		
		//if (lines_r.size() < 150) {
		for (size_t i = 0; i < lines_r.size(); i++) {
			cv::Vec4i line = lines_r[i];
			if (line[1] < 400 && line[3] < 400) {
			if (line[1] < line[3]+20 && line[1] > line[3]-20) {
				
				if ((line[1] > 100 && line[3] > 100) &&  (line[1] < 350 && line[3] < 350)) {

					cv::line(color_canny_r, cv::Point(line[0], line[1]),
						 		cv::Point(line[2], line[3]),
								cv::Scalar(0,255,0),
								3, CV_AA);
					cv::line(raw_r, cv::Point(line[0], line[1]),
						 		cv::Point(line[2], line[3]),
								cv::Scalar(0,255,0),
								3, CV_AA);
					num_valid_lines_r++;
					valid_line_r[0] = (line[0] + valid_line_old_r[0])/2;
					valid_line_r[1] = (line[1] + valid_line_old_r[1])/2;
					valid_line_r[2] = (line[2] + valid_line_old_r[2])/2;
					valid_line_r[3] = (line[3] + valid_line_old_r[3])/2;
				} else {
					cv::line(color_canny_r, cv::Point(line[0], line[1]),
						 		cv::Point(line[2], line[3]),
								cv::Scalar(0,0,255),
								3, CV_AA);
					cv::line(raw_r, cv::Point(line[0], line[1]),
						 		cv::Point(line[2], line[3]),
								cv::Scalar(0,0,255),
								3, CV_AA);
				}
					
			
			}
			}
		//}
		}

		cv::rectangle(raw, cv::Point(50, 150), cv::Point(400, 380), cv::Scalar(0, 255, 0));
		cv::rectangle(raw_r, cv::Point(0, 100), cv::Point(720, 350), cv::Scalar(0, 255, 0));

		if (num_valid_lines < 5) {
			//valid_line_1.x = 400;
			//valid_line_1.y = 150;
			valid_line_2.x = (cos(180-average_angle) * 50);
			valid_line_2.y = 479-(sin(180-average_angle) * 50);
		}

		valid_line_r[0] = 719;
		valid_line_r[2] = 0;


		valid_line[0] = valid_line_1.x;
		valid_line[1] = valid_line_1.y;
		valid_line[2] = valid_line_2.x;
		valid_line[3] = valid_line_2.y;

		valid_line_old[0] = (valid_line[0] + valid_line_old[0]) / 2;
		valid_line_old[1] = (valid_line[1] + valid_line_old[1]) / 2;
		valid_line_old[2] = (valid_line[2] + valid_line_old[2]) / 2;
		valid_line_old[3] = (valid_line[3] + valid_line_old[3]) / 2;		

		valid_line_old_r[0] = (valid_line_r[0] + valid_line_old_r[0]) / 2;
		valid_line_old_r[1] = (valid_line_r[1] + valid_line_old_r[1]) / 2;
		valid_line_old_r[2] = (valid_line_r[2] + valid_line_old_r[2]) / 2;
		valid_line_old_r[3] = (valid_line_r[3] + valid_line_old_r[3]) / 2;		

		cv::line(raw, cv::Point(valid_line[0], valid_line[1]), cv::Point(valid_line[2], valid_line[3]), cv::Scalar(0, 255, 255), 3, CV_AA);
		cv::line(raw_r, cv::Point(valid_line_r[0], valid_line_r[1]), cv::Point(valid_line_r[2], valid_line_r[3]), cv::Scalar(0, 255, 255), 5, CV_AA);

		if (num_valid_lines > 3) {
			canny_thresh += 1;
		} else if (num_valid_lines < 2) {
			canny_thresh -= 1;
		}

		if (num_valid_lines_r > 5) {
			canny_thresh_r += 1;
		} else if (num_valid_lines_r < 2) {
			canny_thresh_r -= 1;
		}
		printf("Angle: %.2f | Angle avg: %.2f | Canny Thresh: %d | Canny Thresh R: %d | Steer: %d\n", atan2(valid_line[1]-valid_line[2], valid_line[0]-valid_line[3]), 
	     												      180-average_angle, canny_thresh, canny_thresh_r, valid_line[0]-valid_line_old[0]);

		if (num_valid_lines == 0) {
			cv::putText(raw, "No Lines!", cv::Point(0, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 255), 3);
		}
		if (num_valid_lines >  0 && num_valid_lines < 10) {
			cv::putText(raw, "Valid Lines", cv::Point(0, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 3);
			if (valid_line[0]-valid_line_old[0] > 0) {
				cv::putText(raw, "Steer Right!", cv::Point(0, 250), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 255), 3);
			}
			if (valid_line[0]-valid_line_old[0] == 0) {
				cv::putText(raw, "Stay Straight.", cv::Point(0, 250), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 255), 3);
			}
			if (valid_line[0]-valid_line_old[0] < 0) {
				cv::putText(raw, "Steer Left!", cv::Point(0, 250), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 255), 3);
			}
		}
		if (num_valid_lines >=  10) {
			cv::putText(raw, "Too Many Valid Lines!", cv::Point(0, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 3);
		}

		if (num_valid_lines_r == 0) {
			cv::putText(raw_r, "No Lines!", cv::Point(0, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 255), 3);
		}
		if (num_valid_lines_r >  0 && num_valid_lines_r < 10) {
			cv::putText(raw_r, "Valid Lines", cv::Point(0, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 3);
		}
		if (num_valid_lines_r >=  10) {
			cv::putText(raw_r, "Too Many Valid Lines!", cv::Point(0, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 3);
		}

		cv::imshow("lines_r", color_canny_r);
		cv::imshow("raw_r", raw_r);
		cv::imshow("Lines", color_canny);
		cv::imshow("raw", raw);

		if (cv::waitKey(30) >= 0) {
			break;
		}
	}

	return 0;
}


