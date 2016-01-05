#include <cstdlib>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc.hpp>

struct line_capture{
	int thread;
	cv::Mat raw;
	cv::Mat canny;
	int canny_thresh;
	cv::vector<cv::Vec4i> lines;
	int hough_thresh;
	int hough_min_length;
	int hough_min_gap;
	int min_angle;
	int max_angle;
	cv::Vec4i roi;
	float average_angle;
	cv::Vec4i average_line;
	int num_good_lines;
};

pthread_mutex_t line_mutex[2] = PTHREAD_MUTEX_INITIALIZER; 


void * line_thread_routine(void * input) {

	size_t i = 0;
	float angle = 0;

	struct line_capture * args = (struct line_capture *)input;

	if (pthread_mutex_trylock(&line_mutex[args->thread]) != 0) {
		pthread_mutex_unlock(&line_mutex[args->thread]);
		pthread_exit(NULL);
	}

	args->num_good_lines = 0;
	args->average_angle = 0;
	args->average_line[0] = 0;
	args->average_line[1] = 0;
	args->average_line[2] = 0;
	args->average_line[3] = 0;

	cv::Canny(args->raw, args->canny, args->canny_thresh, args->canny_thresh*3);
	cv::HoughLinesP(args->canny, args->lines, 1, CV_PI/180, args->hough_thresh,
			args->hough_min_length, args->hough_min_gap);


	if (args->lines.size() > 350) {
		//printf("thread: %d | Too Many Lines! %d | canny_thresh: %d\n", args->thread, args->lines.size(), args->canny_thresh);
		pthread_mutex_unlock(&line_mutex[args->thread]);
		pthread_exit(NULL);
	}

	for (i=0; i < args->lines.size(); i++) {
		cv::Vec4i line = args->lines[i];
		angle = (atan2(line[1]-line[3], line[0]-line[2])*(180/CV_PI));
		args->average_angle = (angle + args->average_angle) / 2;
		if ((angle > args->min_angle) && angle < args->max_angle) {
		//	if (line[0] > args->roi[0] && line[0] < args->roi[2]){
		//	if (line[1] > args->roi[1] && line[1] < args->roi[3]){
		//	if (line[2] > args->roi[0] && line[2] < args->roi[2]){
		//	if (line[3] > args->roi[1] && line[1] < args->roi[3]){
				args->num_good_lines++;
				args->average_line[0] = (line[0] + args->average_line[0]) / 2;
				args->average_line[1] = (line[1] + args->average_line[1]) / 2;
				args->average_line[2] = (line[2] + args->average_line[2]) / 2;
				args->average_line[3] = (line[3] + args->average_line[3]) / 2;
		//	}
		//	}
		//	}
		//	}
		}
	}
	
	//printf("thread: %d | num_good_lines: %d | canny_thresh: %d | average_angle: %f | average_line: [%d, %d] [%d, %d]\n",
	//	args->thread, args->num_good_lines, args->canny_thresh, args->average_angle, args->average_line[0], 
	//	args->average_line[1], args->average_line[2], args->average_line[3]); 
	pthread_mutex_unlock(&line_mutex[args->thread]);
	pthread_exit(NULL);
}

int main(int argc, char ** argv) {

	cv::VideoCapture cap(1);
	cv::VideoCapture cap_r(0);
	cv::Mat raw;
	cv::Mat raw_r;

	cv::Vec4i average_line;	
	cv::Vec4i average_line_r;	

	struct line_capture args;
	struct line_capture args_r;
	struct line_capture line_cap;
	struct line_capture line_cap_r;
	args.canny_thresh = 50;
	args_r.canny_thresh = 50;

	pthread_t line_thread;
	pthread_t line_thread_r;
	pthread_attr_t attr;
	int rc = 0;
	args.average_angle = 0;
	args.average_line[0] = 0;
	args.average_line[1] = 0;
	args.average_line[2] = 0;
	args.average_line[3] = 0;
	args.num_good_lines = 0;
	cap >> args.canny;

	args_r.average_angle = 0;
	args_r.average_line[0] = 0;
	args_r.average_line[1] = 0;
	args_r.average_line[2] = 0;
	args_r.average_line[3] = 0;
	args_r.num_good_lines = 0;
	cap_r >> args_r.canny;

	for (;;) {
		cap >> raw;
		cap_r >> raw_r;
		cv::flip(raw, raw, -1);
		cv::flip(raw_r, raw_r, 0);
		
		// Line Thread
		if (pthread_mutex_trylock(&line_mutex[0]) == 0) {
			pthread_attr_init(&attr);
			pthread_attr_setstacksize(&attr, (sizeof(struct line_capture)*500));

			args.thread = 0;
			args.raw = raw;
			args.hough_thresh = 40;
			args.hough_min_length = 50;
			args.hough_min_gap = 10;
			args.min_angle = 130;
			args.max_angle = 150;
			args.roi[0] = 150;
			args.roi[1] = 0;
			args.roi[2] = 500;
			args.roi[3] = 300;

			if (args.lines.size() < 150) {
				if (args.canny_thresh > 0){
					args.canny_thresh -= 5;
				}
			} 
			if (args.lines.size() > 350){
				if (args.canny_thresh < 500) {
					args.canny_thresh += 5;
 				}
			}

			line_cap = args;
			printf("thread: %d | num_total_lines: %d |  num_good_lines: %d | canny_thresh: %d | average_angle: %f | average_line: [%d, %d] [%d, %d]\n",
				line_cap.thread, line_cap.lines.size(), line_cap.num_good_lines, line_cap.canny_thresh, line_cap.average_angle, 
				line_cap.average_line[0],line_cap.average_line[1],line_cap.average_line[2],line_cap.average_line[3]);
						
			average_line = args.average_line;
			cv::imshow("canny", line_cap.canny);
			pthread_mutex_unlock(&line_mutex[0]);
			

			rc = pthread_create(&line_thread, &attr, line_thread_routine, (void *) &args);

			if (rc) {
				printf("ERROR CREATING THREAD %d\n", rc);
			}

		}

		// Line R Thread
		if (pthread_mutex_trylock(&line_mutex[1]) == 0) {
			pthread_attr_init(&attr);
			pthread_attr_setstacksize(&attr, (sizeof(struct line_capture)*500));

			args_r.thread = 1;
			args_r.raw = raw_r;
			args_r.hough_thresh = 90;
			args_r.hough_min_length = 150;
			args_r.hough_min_gap = 10;
			args_r.min_angle = -20;
			args_r.max_angle = 20;
			args_r.roi[0] = 0;
			args_r.roi[1] = 150;
			args_r.roi[2] = 719;
			args_r.roi[3] = 300;
			pthread_mutex_unlock(&line_mutex[1]);
			

			if (args_r.lines.size() < 10) {
				if (args_r.canny_thresh > 0){
					args_r.canny_thresh -=5;
				}
			} 
			if (args_r.lines.size() > 150){
				if (args_r.canny_thresh < 500) {
					args_r.canny_thresh += 5;
				}
			}

			line_cap_r = args_r;
			printf("thread: %d | num_total_lines: %d | num_good_lines: %d | canny_thresh: %d | average_angle: %f | average_line: [%d, %d] [%d, %d]\n",
				line_cap_r.thread, line_cap_r.lines.size(), line_cap_r.num_good_lines, line_cap_r.canny_thresh, line_cap_r.average_angle, 
				line_cap_r.average_line[0],line_cap_r.average_line[1],line_cap_r.average_line[2],line_cap_r.average_line[3]);
			
			average_line_r = args_r.average_line;
			cv::imshow("canny_r", line_cap_r.canny);
			pthread_mutex_unlock(&line_mutex[1]);

			rc = pthread_create(&line_thread_r, &attr, line_thread_routine, (void *) &args_r);
			if (rc) {
				printf("ERROR CREATING THREAD %d\n", rc);
			}

		}

		cv::line(raw, cv::Point(line_cap.average_line[0], line_cap.average_line[1]),
			      cv::Point(line_cap.average_line[2], line_cap.average_line[3]),
			      cv::Scalar(0, 255, 255), 3, CV_AA);

		cv::imshow("raw", raw);

		cv::line(raw_r, cv::Point(line_cap_r.average_line[0], line_cap_r.average_line[1]),
			      cv::Point(line_cap_r.average_line[2], line_cap_r.average_line[3]),
			      cv::Scalar(0, 255, 255), 3, CV_AA);

		cv::imshow("raw_r", raw_r);

		if (cv::waitKey(30) >= 0) {
			break;
		}
	}
	printf("For done.\n");

	return 0;

}
