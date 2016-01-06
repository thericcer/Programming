#include "line_finder.h"


int main(int argc, char ** argv) {

    cv::VideoCapture cap_f(0);
    cv::VideoCapture cap_r(1);

    struct line_capture line_cap_f;
    struct line_capture line_cap_r;
    struct line_capture args_f;
    struct line_capture args_r;

    if (!cap_f.isOpened() || !cap_r.isOpened()) {
        printf("Failed to open one of the cameras!\n");
        return -1;
    }

    for (;;) {
        cap_f >> line_cap_f.raw;
        cap_r >> line_cap_r.raw;

        // If line_finder F thread is done, update line_cap_f and
        // launch another thread.
        if (pthread_mutex_trylock(&args_f.line_finder_mutex) == 0) {

            line_cap_f = args_f;
            args_f.thread = 0;
            args_f.hough_thresh = 90;
            args_f.hough_min_length = 50;
            args_f.hough_min_gap = 10;
            args_f.min_angle = 130;
            args_f.max_angle = 160;
            args_f.raw = line_cap_f.raw;

            pthread_mutex_unlock(&args_f.line_finder_mutex);
        }

        if (pthread_mutex_trylock(&args_r.line_finder_mutex) == 0) {

            line_cap_f = args_f;
            args_r.thread = 0;
            args_r.hough_thresh = 90;
            args_r.hough_min_length = 50;
            args_r.hough_min_gap = 10;
            args_r.min_angle = -30;
            args_r.max_angle = 30;
            args_r.raw = line_cap_f.raw;

            pthread_mutex_unlock(&args_r.line_finder_mutex);
        }



        cv::imshow("raw_f", line_cap_f.raw);
        cv::imshow("raw_r", line_cap_r.raw);

        if (cv::waitKey(30)) {
            break;
        }
    }
    return 0;
}
