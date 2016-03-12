#include "line_finder.h"
#include <time.h>

#define DEBUG false


void * threshold_thread_routine(void * input) {

    struct capture * args = (struct capture *)input;
    printf ("Threshold Thread: %d | Launching.\n", args->thread);

    while(1) {

        if (pthread_mutex_trylock(&args->capture_mutex) != 0) {
            continue;
        }

        if (args->exit) {
            printf ("Threshold Thread: %d | Exiting.\n", args->thread);
            pthread_exit(NULL);
        }

        pthread_cond_wait(&args->threshold_cond, &args->capture_mutex);

        clock_t start_time = clock();

        cv::Canny(args->raw, args->canny, args->canny_thresh, args->canny_thresh*3);

        cv::HoughLinesP(args->canny, args->lines, 1, CV_PI/180, args->hough_thresh,
                        args->hough_min_length, args->hough_min_gap);

        start_time = clock() - start_time;
        printf("Threshold Thread: %d | Time: %f s | Signaling.\n", args->thread, ((float)start_time)/CLOCKS_PER_SEC);

        pthread_cond_signal(&args->line_search_cond);

        pthread_mutex_unlock(&args->capture_mutex);

    }
}


void * line_search_thread_routine (void * input) {

    struct capture * args = (struct capture *)input;

    printf("Search Thread: %d | Launching.\n", args->thread);

    while (1) {

        size_t i = 0;
        float angle = 0;
#if DEBUG
        printf("Search Thread: %d | Starting.\n", args->thread);
#endif

        if (pthread_mutex_trylock(&args->capture_mutex) != 0) {
            continue;
        }

        if (args->exit) {
            printf("Search Thread: %d | Exiting.\n", args->thread);
            pthread_exit(NULL);
        }

#if DEBUG
        printf("Search Thread: %d | Waiting for signal.\n", args->thread);
#endif

        pthread_cond_wait(&args->line_finder_cond, &args->capture_mutex);

        clock_t start_time = clock();

        args->num_good_lines = 0;
        args->average_angle = 0;
        args->average_line[0] = 0;
        args->average_line[1] = 0;
        args->average_line[2] = 0;
        args->average_line[3] = 0;

#if DEBUG
        printf("Search Thread: %d running line search.\n", args->thread);
#endif
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


        start_time = clock() - start_time;
        printf("Search Thread: %d | Time: %f s | Done.\n", args->thread, ((float)start_time)/CLOCKS_PER_SEC);
        pthread_mutex_unlock(&args->capture_mutex);
    }
}
