#include "line_finder.h"
#include <time.h>

#define CANNY_THRESH_MIN 0
#define CANNY_THRESH_MAX 500
#define LINE_THRESH_MIN 10
#define LINE_THRESH_MAX 50

#define RR false

pthread_cond_t line_finder_cond[2];

int main(int argc, char ** argv) {

    cv::VideoCapture cap_f(0);

    struct line_capture line_cap_f;
    struct line_capture args_f;

    args_f.exit = false;

    int rc = 1;

    pthread_t line_finder_thread_f;
    pthread_t line_search_thread_f;

    pthread_attr_t attr_f;
    pthread_attr_t attr_search_f;

#if RR
    struct sched_param param_f;
    struct sched_param param_search_f;
#endif


    if (!cap_f.isOpened()) {
        printf("Failed to open one of the camera!\n");
        return -1;
    }

    cap_f >> line_cap_f.canny;
    cap_f >> line_cap_f.raw;
    cap_f >> args_f.canny;
    cap_f >> args_f.raw;

    args_f.canny_thresh = 100;
    args_f.thread = 0;
    args_f.hough_thresh = 90;
    args_f.hough_min_length = 50;
    args_f.hough_min_gap = 10;
    args_f.min_angle = 130;
    args_f.max_angle = 160;

    pthread_attr_init(&attr_f);
#if RR
    param_f.sched_priority = 1;
    pthread_attr_setschedpolicy(&attr_f, SCHED_RR);
    pthread_attr_setschedparam(&attr_f, &param_f);
    pthread_attr_setinheritsched(&attr_f, PTHREAD_EXPLICIT_SCHED);
#endif
    pthread_attr_setstacksize(&attr_f, (sizeof(struct line_capture)*500));

    pthread_attr_init(&attr_search_f);
#if RR
    param_search_f.sched_priority = 1;
    pthread_attr_setschedpolicy(&attr_search_f, SCHED_RR);
    pthread_attr_setschedparam(&attr_search_f, &param_search_f);
    pthread_attr_setinheritsched(&attr_search_f, PTHREAD_EXPLICIT_SCHED);
#endif
    pthread_attr_setstacksize(&attr_search_f, (sizeof(struct line_capture)*500));

    pthread_cond_init(&args_f.line_finder_cond, NULL);

    rc = pthread_create(&line_search_thread_f, &attr_f, line_search_thread_routine, (void *) &args_f);

    if (rc) {
        printf("ERROR CREATING THREAD %d\n", rc);
    }

    rc = pthread_create(&line_finder_thread_f, &attr_search_f, line_finder_thread_routine, (void *) &args_f);

    if (rc) {
        printf("ERROR CREATING THREAD %d\n", rc);
    }


    for (;;) {
        clock_t start_time = clock();
        cap_f >> line_cap_f.raw;


        cv::line(line_cap_f.raw, cv::Point(line_cap_f.average_line[0], line_cap_f.average_line[1]),
                        cv::Point(line_cap_f.average_line[2], line_cap_f.average_line[3]),
                        cv::Scalar(0, 255, 255), 3, CV_AA);

        cv::imshow("raw_f", line_cap_f.raw);
        cv::imshow("canny_f", line_cap_f.canny);

//        if (pthread_mutex_trylock(&args_f.line_finder_mutex) == 0) {

            printf("Thread: Main | Copying args_f to line_cap_f\n");

            args_f.thread = 0;
            args_f.hough_thresh = 90;
            args_f.hough_min_length = 50;
            args_f.hough_min_gap = 10;
            args_f.min_angle = 130;
            args_f.max_angle = 160;
            args_f.raw = line_cap_f.raw;

            if (args_f.lines.size() > LINE_THRESH_MAX) {
                if (args_f.canny_thresh < CANNY_THRESH_MAX) {
                    args_f.canny_thresh++;
                }
            }
            if (args_f.lines.size() < LINE_THRESH_MIN) {
                if (args_f.canny_thresh > CANNY_THRESH_MIN) {
                    args_f.canny_thresh--;
                }
            }


            if (args_f.lines.size() > 0) {
                line_cap_f = args_f;
            }
            line_cap_f = args_f;

//            pthread_mutex_unlock(&args_f.line_finder_mutex);
            printf("Num Lines F: %4d canny_thresh: %4d\n", args_f.lines.size(), 
                   args_f.canny_thresh);


//        }

        if (cv::waitKey(30) >= 0) {
            while (!pthread_mutex_trylock(&args_f.line_finder_mutex));
            args_f.exit = true;
            pthread_mutex_unlock(&args_f.line_finder_mutex);
            pthread_join(line_finder_thread_f, NULL);
            break;
        }
        start_time = clock() - start_time;
        printf("Thread: Main | Time: %f s\n", ((float)start_time)/CLOCKS_PER_SEC);
    }
    return 0;
}
