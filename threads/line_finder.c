#include "line_finder.h"
#include <time.h>

#define DEBUG false

extern pthread_cond_t line_finder_cond[2];

void * line_finder_thread_routine(void * input) {

    struct line_capture * args = (struct line_capture *)input;
    printf ("Hough Thread: %d | Launching.\n", args->thread);

    cv::Mat lambda(2, 4, CV_32FC1);
    cv::Mat warped;

    cv::Point2f input_quad[4];
    cv::Point2f output_quad[4];




    while(1) {

        clock_t start_time = clock();

#if DEBUG
        printf ("Hough Thread: %d | Starting.\n", args->thread);
#endif


        if (pthread_mutex_trylock(&args->line_finder_mutex) != 0) {
            continue;
        }


#if DEBUG
        printf("Hough Thread: %d | Waiting for signal.\n", args->thread);
#endif
        pthread_cond_wait(&args->line_finder_cond, &args->line_finder_mutex);

        if (args->exit) {
            printf ("Hough Thread: %d | Exiting.\n", args->thread);
            pthread_cond_signal(&args->line_search_cond);
            pthread_mutex_unlock(&args->line_finder_mutex);
            pthread_exit(NULL);
        }


        lambda = cv::Mat::zeros(args->raw.rows, args->raw.cols, args->raw.type());

        input_quad[0] = cv::Point2f(80, args->raw.rows-450);
        input_quad[1] = cv::Point2f(args->raw.cols - 80, args->raw.rows-450);
        input_quad[2] = cv::Point2f(args->raw.cols, args->raw.rows-100);
        input_quad[3] = cv::Point2f(0, args->raw.rows-100);

        output_quad[0] = cv::Point2f(0, 0);
        output_quad[1] = cv::Point2f(args->raw.cols, 0);
        output_quad[2] = cv::Point2f(args->raw.cols-270, args->raw.rows);
        output_quad[3] = cv::Point2f(270, args->raw.rows);

        lambda = cv::getPerspectiveTransform(input_quad, output_quad);


        cv::warpPerspective(args->raw, warped, lambda, args->raw.size());
        //cv::GaussianBlur(warped, warped, cv::Size(5, 5), 0, 0);

        cv::imshow("warped", warped);

        cv::Canny(warped, args->canny, args->canny_thresh, args->canny_thresh*3);

#if DEBUG
        printf("Hough Thread: %d | Running Canny.\n", args->thread);
#endif


#if DEBUG
        printf("Hough Thread: %d | Running Hough Lines P.\n", args->thread);
#endif
        cv::HoughLinesP(args->canny, args->lines, 1, CV_PI/180, args->hough_thresh,
                        args->hough_min_length, args->hough_min_gap);

        start_time = clock() - start_time;
        printf("Hough Thread: %d | Time: %f s | FPS: %f\n", args->thread, ((float)start_time)/CLOCKS_PER_SEC,
                 1/(((float)start_time)/CLOCKS_PER_SEC));

        pthread_cond_signal(&args->line_search_cond);

        pthread_mutex_unlock(&args->line_finder_mutex);

#if DEBUG
        printf("Hough Thread: %d | Done.\n", args->thread);
#endif
    }
}


void * line_search_thread_routine (void * input) {

    struct line_capture * args = (struct line_capture *)input;

    printf("Search Thread: %d | Launching.\n", args->thread);

    while (1) {

        clock_t start_time = clock();
        size_t i = 0;
        size_t j = 0;
        float angle = 0;
        float slope = 0;
        float intercept = 0;
        cv::vector<cv::Vec4f> line_stats;
        cv::vector<cv::Vec2f> line_equations;
#if DEBUG
        printf("Search Thread: %d | Starting.\n", args->thread);
#endif

        if (pthread_mutex_trylock(&args->line_finder_mutex) != 0) {
            continue;
        }


#if DEBUG
        printf("Search Thread: %d | Waiting for signal.\n", args->thread);
#endif

        pthread_cond_wait(&args->line_search_cond, &args->line_finder_mutex);

        if (args->exit) {
            printf("Search Thread: %d | Exiting.\n", args->thread);
            pthread_mutex_unlock(&args->line_finder_mutex);
            pthread_exit(NULL);
        }

        args->num_parallel_lines = 0;
        args->average_angle = 0;
        args->average_line[0] = 0;
        args->average_line[1] = 0;
        args->average_line[2] = 0;
        args->average_line[3] = 0;

		cv::cvtColor(args->canny, args->canny, CV_GRAY2BGR);

#if DEBUG
        printf("Search Thread: %d running line search.\n", args->thread);
#endif
        args->good_lines.clear();
        line_stats.clear();
        line_equations.clear();
        for (i=0; i < args->lines.size(); i++) {
            cv::Vec4i line = args->lines[i];
            angle = fabs((atan2(line[1]-line[3], line[0]-line[2])*(180/CV_PI)));
            if (line[0] != line[2]) {
                slope = (line[1] - line[3]) / (line[0] - line[2]);
            } else {
                slope = 9999;
            }
            intercept = line[1] - (slope * line[0]);
            line_equations.push_back(cv::Vec2f(slope, intercept));
            printf("Search Thread: %d line %d equation: y=%.02fx+%.2f\n", args->thread, i, slope, intercept);
            // cv::line(args->canny, cv::Point(line[0], line[1]),
            //                       cv::Point(line[2], line[3]),
            //                       cv::Scalar(0, 255, 0), 2, CV_AA);

            line_stats.push_back(cv::Vec4f(i, angle, 0, 0));
            if ((angle > args->min_angle) && angle < args->max_angle) {
                args->average_angle = (angle + args->average_angle) / 2;

                cv::line(args->canny, cv::Point(line[0], line[1]),
                                cv::Point(line[2], line[3]),
                                cv::Scalar(0, 255, 0), 2, CV_AA);
                
                args->num_parallel_lines++;
                args->good_lines.push_back(line);
                args->average_line[0] = (line[0] + args->average_line[0]) / 2;
                args->average_line[1] = (line[1] + args->average_line[1]) / 2;
                args->average_line[2] = (line[2] + args->average_line[2]) / 2;
                args->average_line[3] = (line[3] + args->average_line[3]) / 2;
            } else {
            }

        }
        cv::line(args->canny, cv::Point(args->canny.cols/2, 0),
                              cv::Point(args->canny.cols/2, args->canny.rows),
                              cv::Scalar(255, 0, 0), 2, CV_AA);

        start_time = clock() - start_time;
        printf("Search Thread: %d | Time: %f s | Done.\n", args->thread, ((float)start_time)/CLOCKS_PER_SEC);
        pthread_mutex_unlock(&args->line_finder_mutex);
    }
}

