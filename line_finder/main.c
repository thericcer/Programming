#include "line_finder.h"

#define CANNY_THRESH_MIN 10
#define CANNY_THRESH_MAX 50


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
    pthread_t line_search_threas_r;
	pthread_attr_t attr_f;
	pthread_attr_t attr_r;

    if (!cap_f.isOpened() || !cap_r.isOpened()) {
        printf("Failed to open one of the cameras!\n");
        return -1;
    }

    cap_f >> line_cap_f.canny;
    cap_r >> line_cap_r.canny;

    args_f.canny_thresh = 100;
    args_r.canny_thresh = 100;

    for (;;) {
        cap_f >> line_cap_f.raw;
        cap_r >> line_cap_r.raw;

        cv::flip(line_cap_f.raw, line_cap_f.raw, -1);
        cv::flip(line_cap_r.raw, line_cap_r.raw, 0);

        cv::imshow("raw_f", line_cap_f.raw);
        cv::imshow("canny_f", line_cap_f.canny);
        cv::imshow("raw_r", line_cap_r.raw);
        cv::imshow("canny_r", line_cap_r.canny);

        // If line_finder F thread is done, update line_cap_f and
        // launch another thread.
        if (pthread_mutex_trylock(&args_f.line_finder_mutex) == 0) {

			pthread_attr_init(&attr_f);
			pthread_attr_setstacksize(&attr_f, (sizeof(struct line_capture)*500));

            args_f.thread = 0;
            args_f.hough_thresh = 90;
            args_f.hough_min_length = 50;
            args_f.hough_min_gap = 10;
            args_f.min_angle = 130;
            args_f.max_angle = 160;
            args_f.raw = line_cap_f.raw;

            if (args_f.lines.size() > CANNY_THRESH_MAX) {
                if (args_f.canny_thresh < 200) {
                    args_f.canny_thresh++;
                }
            }
            if (args_f.lines.size() < CANNY_THRESH_MIN) {
                if (args_f.canny_thresh > 10) {
                    args_f.canny_thresh--;
                }
            }


            if (args_f.lines.size() > 0) {
                line_cap_f = args_f;
            }
            
            printf("Num Lines F: %4d canny_thresh: %4d\n", args_f.lines.size(), 
                   args_f.canny_thresh);

            rc = pthread_create(&line_finder_thread_f, &attr_f, line_finder_thread_routine, (void *) &args_f);
            
            if (rc) {
                printf("ERROR CREATING THREAD %d\n", rc);
            }

            pthread_mutex_unlock(&args_f.line_finder_mutex);
        }

        if (pthread_mutex_trylock(&args_r.line_finder_mutex) == 0) {

			pthread_attr_init(&attr_r);
			pthread_attr_setstacksize(&attr_r, (sizeof(struct line_capture)*500));

            args_r.thread = 1;
            args_r.hough_thresh = 90;
            args_r.hough_min_length = 50;
            args_r.hough_min_gap = 10;
            args_r.min_angle = -30;
            args_r.max_angle = 30;
            args_r.raw = line_cap_r.raw;

            if (args_r.lines.size() > CANNY_THRESH_MAX) {
                if (args_r.canny_thresh < 200) {
                    args_r.canny_thresh++;
                }
            }
            if (args_r.lines.size() < CANNY_THRESH_MIN) {
                if (args_r.canny_thresh > 0) {
                    args_r.canny_thresh--;
                }
            }

            if (args_r.lines.size() > 0) {
                line_cap_r = args_r;
            }
            printf("Num Lines R: %4d canny_thresh: %4d\n", args_r.lines.size(), 
                   args_r.canny_thresh);
            

            rc = pthread_create(&line_finder_thread_r, &attr_r, line_finder_thread_routine, (void *) &args_r);
            
            if (rc) {
                printf("ERROR CREATING THREAD %d\n", rc);
            }


            pthread_mutex_unlock(&args_r.line_finder_mutex);
        }
        

        if (cv::waitKey(30) >= 0) {
            pthread_join(line_finder_thread_f, NULL);
            pthread_join(line_finder_thread_r, NULL);
            break;
        }
    }
    return 0;
}
