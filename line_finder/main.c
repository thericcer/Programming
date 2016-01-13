#include "line_finder.h"
#include <time.h>

#define CANNY_THRESH_MIN 10
#define CANNY_THRESH_MAX 500
#define LINE_THRESH_MIN 10
#define LINE_THRESH_MAX 50

pthread_cond_t line_finder_cond[2];

int main(int argc, char ** argv) {

    cv::VideoCapture cap_f(1);
    cv::VideoCapture cap_r(0);

    struct line_capture line_cap_f;
    struct line_capture line_cap_r;
    struct line_capture args_f;
    struct line_capture args_r;


    int rc = 1;

    pthread_t line_finder_thread_f;
    pthread_t line_finder_thread_r;

    pthread_t line_search_thread_f;
    pthread_t line_search_thread_r;

	pthread_attr_t attr_f;
	pthread_attr_t attr_r;
    struct sched_param param_f;
    struct sched_param param_r;


    if (!cap_f.isOpened() || !cap_r.isOpened()) {
        printf("Failed to open one of the cameras!\n");
        return -1;
    }

    cap_f >> line_cap_f.canny;
    cap_r >> line_cap_r.canny;

    args_f.canny_thresh = 100;
    args_r.canny_thresh = 100;

    

    for (;;) {
        clock_t start_time = clock();
        cap_f >> line_cap_f.raw;
        cap_r >> line_cap_r.raw;

        cv::flip(line_cap_f.raw, line_cap_f.raw, -1);
        cv::flip(line_cap_r.raw, line_cap_r.raw, 0);

        cv::line(line_cap_f.raw, cv::Point(line_cap_f.average_line[0], line_cap_f.average_line[1]),
                        cv::Point(line_cap_f.average_line[2], line_cap_f.average_line[3]),
                        cv::Scalar(0, 255, 255), 3, CV_AA);
        cv::line(line_cap_r.raw, cv::Point(line_cap_r.average_line[0], line_cap_r.average_line[1]),
                        cv::Point(line_cap_r.average_line[2], line_cap_r.average_line[3]),
                        cv::Scalar(0, 255, 255), 3, CV_AA);

        cv::imshow("raw_f", line_cap_f.raw);
        cv::imshow("canny_f", line_cap_f.canny);
        cv::imshow("raw_r", line_cap_r.raw);
        cv::imshow("canny_r", line_cap_r.canny);

        // If line_finder F thread is done, update line_cap_f and
        // launch another thread.
        if (pthread_mutex_trylock(&args_f.line_finder_mutex) == 0) {

            param_f.sched_priority = 1;
			pthread_attr_init(&attr_f);
			pthread_attr_setstacksize(&attr_f, (sizeof(struct line_capture)*500));
            pthread_attr_setschedpolicy(&attr_f, SCHED_FIFO);
            pthread_attr_setschedparam(&attr_f, &param_f);
            pthread_attr_setinheritsched(&attr_f, PTHREAD_EXPLICIT_SCHED);

            pthread_cond_init(&line_finder_cond[args_f.thread], NULL);

            args_f.thread = 0;
            args_f.hough_thresh = 90;
            args_f.hough_min_length = 50;
            args_f.hough_min_gap = 10;
            args_f.min_angle = 130;
            args_f.max_angle = 160;
            args_f.raw = line_cap_f.raw;

            if (args_f.num_good_lines > LINE_THRESH_MAX) {
                if (args_f.canny_thresh < CANNY_THRESH_MAX) {
                    args_f.canny_thresh++;
                }
            }
            if (args_f.num_good_lines < LINE_THRESH_MIN) {
                if (args_f.canny_thresh > CANNY_THRESH_MIN) {
                    args_f.canny_thresh--;
                }
            }


            if (args_f.lines.size() > 0) {
                line_cap_f = args_f;
            }
            
            pthread_mutex_unlock(&args_f.line_finder_mutex);
            //printf("Num Lines F: %4d canny_thresh: %4d\n", args_f.lines.size(), 
            //       args_f.canny_thresh);

            rc = pthread_create(&line_search_thread_f, &attr_f, line_search_thread_routine, (void *) &args_f);
            
            if (rc) {
                printf("ERROR CREATING THREAD %d\n", rc);
            }

            rc = pthread_create(&line_finder_thread_f, &attr_f, line_finder_thread_routine, (void *) &args_f);
            
            if (rc) {
                printf("ERROR CREATING THREAD %d\n", rc);
            }

        }

        if (pthread_mutex_trylock(&args_r.line_finder_mutex) == 0) {

            param_r.sched_priority = 1;
			pthread_attr_init(&attr_r);
			pthread_attr_setstacksize(&attr_r, (sizeof(struct line_capture)*500));
            pthread_attr_setschedpolicy(&attr_r, SCHED_FIFO);
            pthread_attr_setschedparam(&attr_r, &param_r);
            pthread_attr_setinheritsched(&attr_r, PTHREAD_EXPLICIT_SCHED);

            args_r.thread = 1;
            args_r.hough_thresh = 90;
            args_r.hough_min_length = 50;
            args_r.hough_min_gap = 10;
            args_r.min_angle = -170;
            args_r.max_angle = 170;
            args_r.raw = line_cap_r.raw;

            pthread_cond_init(&line_finder_cond[args_r.thread], NULL);

            if (args_r.num_good_lines > LINE_THRESH_MAX) {
                if (args_r.canny_thresh < CANNY_THRESH_MAX) {
                    args_r.canny_thresh++;
                }
            }
            if (args_r.num_good_lines < LINE_THRESH_MIN) {
                if (args_r.canny_thresh > CANNY_THRESH_MIN) {
                    args_r.canny_thresh--;
                }
            }

            if (args_r.lines.size() > 0) {
                line_cap_r = args_r;
            }
            //printf("Num Lines R: %4d canny_thresh: %4d\n", args_r.lines.size(), 
            //       args_r.canny_thresh);
            
            rc = pthread_create(&line_search_thread_r, &attr_r, line_search_thread_routine, (void *) &args_r);
            
            if (rc) {
                printf("ERROR CREATING THREAD %d\n", rc);
            }

            rc = pthread_create(&line_finder_thread_r, &attr_r, line_finder_thread_routine, (void *) &args_r);
            
            if (rc) {
                printf("EOTHEROR CREATING THREAD %d\n", rc);
            }


            pthread_mutex_unlock(&args_r.line_finder_mutex);
        }

        

        if (cv::waitKey(30) >= 0) {
            pthread_join(line_finder_thread_f, NULL);
            pthread_join(line_finder_thread_r, NULL);
            break;
        }
        start_time = clock() - start_time;
        //printf("Thread: Main | Time: %f s\n", ((float)start_time)/CLOCKS_PER_SEC);
    }
    return 0;
}
