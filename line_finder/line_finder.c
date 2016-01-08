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

    // if (args->lines.size() > 150) {
    //     printf("Thread: %d too many lines: %d\n", args->thread, args->lines.size());
    //     pthread_mutex_unlock(&args->line_search_mutex);
    //     pthread_exit(NULL);
    // }

    printf("Thread: %d running canny.\n", args->thread);
    cv::Canny(args->raw, args->canny, args->canny_thresh, args->canny_thresh*3);
    printf("Thread: %d running hough.\n", args->thread);
    cv::HoughLinesP(args->canny, args->lines, 1, CV_PI/180, args->hough_thresh,
                    args->hough_min_length, args->hough_min_gap);


    printf("Thread: %d running line search.\n", args->thread);
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

    printf("Thread: %d done.\n", args->thread);
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
