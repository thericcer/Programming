#include "line_finder.h"


int main(int argc, char ** argv) {

    cv::VideoCapture cap_f(0);
    cv::VideoCapture cap_r(1);

    cv::Mat raw_f;
    cv::Mat raw_r;

    for (;;) {
        cap_f >> raw_f;
        cap_r >> raw_r;

        cv::imshow("raw_f", raw_f);
        cv::imshow("raw_r", raw_r);

        if (cv::waitKey(30)) {
            break;
        }
    }
    return 0;
}
