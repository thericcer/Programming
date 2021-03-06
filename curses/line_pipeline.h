#ifndef __LINE_PIPELINE_H_
#define __LINE_PIPELINE_H_

#include <cstdlib>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc.hpp>


struct capture {
    int thread;
    int hough_thresh;
    int hough_min_length;
    int hough_min_gap;
    int min_angle;
    int max_angle;
    int canny_thresh;
    int num_good_lines;

    bool exit;

    float average_angle;

    WINDOW * camera_win;

    char raw_win[5] = "raw";

    cv::VideoCapture cap;
    cv::Mat raw;
    cv::Mat canny;
    cv::vector<cv::Vec4i> lines;
    cv::Vec4i roi;
    cv::Vec4i average_line;

    pthread_mutex_t capture_mutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_cond_t threshold_cond = PTHREAD_COND_INITIALIZER;
};


void * line_finder_thread_routine(void * input);
void * line_search_thread_routine(void * input);

#endif // __LINE_FINDER_H_
