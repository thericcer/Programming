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
