#include "line_finder.h"
#include <time.h>
#include <unistd.h>

#define CANNY_THRESH_MIN 0
#define CANNY_THRESH_MAX 500
#define LINE_THRESH_MIN 50
#define LINE_THRESH_MAX 20

#define RR false

pthread_cond_t line_finder_cond[2];

int main(int argc, char ** argv) {

    cv::VideoCapture cap_f(0);
    cv::VideoCapture cap_r(1);

    cv::Mat hls;
    cv::Mat mask;
    cv::Mat zero = cv::Mat(1, 1, CV_32F, cv::Scalar(0));

    cvNamedWindow("canny_f", CV_WINDOW_FREERATIO);
    cvNamedWindow("raw_f", CV_WINDOW_FREERATIO);
    cvNamedWindow("warped", CV_WINDOW_FREERATIO);
    cvNamedWindow("raw_r", CV_WINDOW_FREERATIO);


    cap_f.set(cv::CAP_PROP_BRIGHTNESS, 0);
    // cap_f.set(cv::CAP_PROP_EXPOSURE, 100);

    struct line_capture line_cap_f;
    struct line_capture args_f;

    struct line_capture line_cap_r;

    args_f.exit = false;

    int rc = 1;
    unsigned char lightness_min = 0;
    unsigned char lightness_range = 0;
    unsigned char lightness_max = 100;
    unsigned char lightness = lightness_min;

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
    if (!cap_r.isOpened()) {
        printf("Failed to open one of the camera!\n");
        return -1;
    }

    cap_f >> line_cap_f.canny;
    cap_f >> line_cap_f.raw;
    cap_f >> args_f.canny;
    cap_f >> args_f.raw;

    cap_f >> line_cap_r.canny;
    cap_f >> line_cap_r.raw;

    args_f.canny_thresh = 100;
    args_f.thread = 0;
    args_f.hough_thresh = 90;
    args_f.hough_min_length = 100;
    args_f.hough_min_gap = 10;
    args_f.min_angle = 85;
    args_f.max_angle = 95;

    pthread_attr_init(&attr_f);
#if RR
    param_f.sched_priority = 1;
    pthread_attr_setschedpolicy(&attr_f, SCHED_FIFO);
    pthread_attr_setschedparam(&attr_f, &param_f);
    pthread_attr_setinheritsched(&attr_f, PTHREAD_EXPLICIT_SCHED);
#endif
    pthread_attr_setstacksize(&attr_f, (sizeof(struct line_capture)*500));

    pthread_attr_init(&attr_search_f);
#if RR
    param_search_f.sched_priority = 1;
    pthread_attr_setschedpolicy(&attr_search_f, SCHED_FIFO);
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

        cap_r >> line_cap_r.raw;

        cv::flip(line_cap_f.raw, line_cap_f.raw, -1);
        cv::flip(line_cap_r.raw, line_cap_r.raw, 0);

        // cv::cvtColor(line_cap_f.raw, hls, CV_RGB2HLS);

        // cv::inRange(hls, cv::Scalar(0, lightness, 0),
        //                             cv::Scalar(255, lightness + lightness_range, 255),
        //                             mask);

        // mask = ~mask;
        // cv::bitwise_and(line_cap_f.raw, zero, line_cap_f.raw, mask);

        // cv::line(line_cap_f.raw, cv::Point(line_cap_f.average_line[0], 0),
        //                 cv::Point(line_cap_f.average_line[2], 480),
        //                 cv::Scalar(0, 255, 255), 3, CV_AA);

        cv::imshow("raw_f", line_cap_f.raw);
        cv::imshow("canny_f", line_cap_f.canny);
        cv::imshow("raw_r", line_cap_r.raw);

        if (pthread_mutex_trylock(&args_f.line_finder_mutex) == 0) {

            // printf("Thread: Main | Copying args_f to line_cap_f\n");

            args_f.thread = 0;
            args_f.min_angle = 80;
            args_f.max_angle = 100;
            args_f.raw = line_cap_f.raw;
            if (argc == 6) {
                args_f.hough_min_length = atoi(argv[1]);
                args_f.hough_min_gap = atoi(argv[2]);
                args_f.hough_thresh = atoi(argv[3]);
                lightness = atoi(argv[4]);
                lightness_range = atoi(argv[5]);
            } else {
                args_f.hough_thresh = 20;
                args_f.hough_min_length = 50;
                args_f.hough_min_gap = 20;
                lightness_min = 0;
                lightness_range = 0;
            }



            if (args_f.num_parallel_lines > LINE_THRESH_MAX) {
                if (args_f.canny_thresh < CANNY_THRESH_MAX) {
                    //args_f.canny_thresh+=15;
                }
                if (lightness > lightness_min) {
                    lightness-=5;
                    cap_f.set(cv::CAP_PROP_BRIGHTNESS, lightness);
                }
            }
            if (args_f.num_parallel_lines < LINE_THRESH_MIN) {
                if (args_f.lines.size() > 80) {
                    //args_f.canny_thresh += 15;
                }
                if (args_f.canny_thresh > CANNY_THRESH_MIN) {
                    //args_f.canny_thresh-=5;
                }
                if (lightness < lightness_max) {
                    lightness+=5;
                    cap_f.set(cv::CAP_PROP_BRIGHTNESS, lightness);
                    if (lightness >= lightness_max-lightness_range) {
                        lightness = lightness-15;
                    }
                }
            }


            args_f.raw = line_cap_f.raw;
            line_cap_f.canny = args_f.canny;

            pthread_cond_signal(&args_f.line_finder_cond);

            pthread_mutex_unlock(&args_f.line_finder_mutex);
            printf("Num Good Lines F: %4d Average angle: %4f canny_thresh: %4d lightness: %4d\n", line_cap_f.num_parallel_lines,
                   line_cap_f.average_angle, line_cap_f.canny_thresh, lightness);


        }

        rc = cv::waitKey(1);
        if (rc == 113) {

            printf ("Main Thread: Exit, locking mutex\n");
            sleep(1);

            while (pthread_mutex_trylock(&args_f.line_finder_mutex));
            args_f.exit = true;
            printf("Main Thread: Signaling line_finder to exit\n");
            pthread_cond_signal(&args_f.line_finder_cond);
            pthread_mutex_unlock(&args_f.line_finder_mutex);
            printf("Main Thread: Waiting for line_finder to join\n");
            pthread_join(line_finder_thread_f, NULL);
            pthread_join(line_search_thread_f, NULL);

            break;
        }
        start_time = clock() - start_time;
        //printf("Thread: Main   | Time: %f s | FPS: %f\n", ((float)start_time)/CLOCKS_PER_SEC,
        //        1/(((float)start_time)/CLOCKS_PER_SEC) );
    }
    return 0;
}
