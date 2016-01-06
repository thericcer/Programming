#include <cstdlib>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc.hpp>

struct line_capture{
    int thread;
    int hough_thresh;
    int hough_min_length;
    int hough_min_gap;
    int min_angle;
    int max_angle;
    int canny_thresh;
    int num_good_lines;

    float average_angle;

    cv::Mat raw;
    cv::Mat canny;
    cv::vector<cv::Vec4i> lines;
    cv::Vec4i roi;
    cv::Vec4i average_line;
};

pthread_mutex_t line_finder_mutex[2] = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t line_search_mutex[2] = PTHREAD_MUTEX_INITIALIZER;

void * line_finder_thread_routine(void * input) {

    size_t i = 0;
    float angle = 0;

    struct line_capture * args = (struct line_capture *)input;

    if (pthread_mutex_trylock(&line_finder_mutex[args->thread]) != 0) {
        pthread_mutex_unlock(&line_finder_mutex[args->thread]);
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
        pthread_mutex_unlock(&line_finder_mutex[args->thread]);
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
    pthread_mutex_unlock(&line_finder_mutex[args->thread]);
    pthread_exit(NULL);
}

int main(int argc, char ** argv) {

    cv::VideoCapture cap_f(0);
    cv::VideoCapture cap_r(1);

    cv::Mat raw_f;
    cv::Mat raw_r;

    for (;;) {
        cap_f >> raw_f;
        cap_r >> raw_r;

        cv::imshow("raw_f", raw_f);
        cv::imshow("raw_r", raw_r);

        if (cv::waitKey(30)) {
            break;
        }
    }
    return 0;
}
