#include "line_finder.h"

void * line_finder_thread_routine(void * input) {

    size_t i = 0;
    float angle = 0;

    struct line_capture * args = (struct line_capture *)input;

    if (pthread_mutex_trylock(&args->line_finder_mutex) != 0) {
        pthread_mutex_unlock(&args->line_finder_mutex);
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
        pthread_mutex_unlock(&args->line_search_mutex);
        pthread_exit(NULL);
    }

    for (i=0; i < args->lines.size(); i++) {
        cv::Vec4i line = args->lines[i];
        angle = (atan2(line[1]-line[3], line[0]-line[2])*(180/CV_PI));
        args->average_angle = (angle + args->average_angle) / 2;
        if ((angle > args->min_angle) && angle < args->max_angle) {
            args->num_good_lines++;
            args->average_line[0] = (line[0] + args->average_line[0]) / 2;
            args->average_line[1] = (line[1] + args->average_line[1]) / 2;
            args->average_line[2] = (line[2] + args->average_line[2]) / 2;
            args->average_line[3] = (line[3] + args->average_line[3]) / 2;
        }
    }

    pthread_mutex_unlock(&args->line_finder_mutex);
    pthread_exit(NULL);
}


void * line_search_thread_routine (void * input) {

    struct line_capture * args = (struct line_capture *)input;

    if (pthread_mutex_trylock(&args->line_search_mutex) != 0) {
        pthread_mutex_unlock(&args->line_search_mutex);
        pthread_exit(NULL);
    }


    pthread_mutex_unlock(&args->line_search_mutex);
    pthread_exit(NULL);

}
