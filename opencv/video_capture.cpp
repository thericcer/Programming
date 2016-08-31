#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc.hpp>

int rc = 0;
int thresh = 20;
cv::Mat raw_f;
cv::Mat raw_r;


int main(int argc, char ** argv) {

	cv::VideoCapture cap_f(0);
	if(!cap_f.isOpened()){
        printf("Could not open front camera!\n");
		return -1;
	}

	cv::VideoCapture cap_r(1);
	if(!cap_r.isOpened()){
        printf("Could not open rear camera!\n");
		return -1;
	}

    cv::VideoWriter write_f cvCreateVideoWriter("~/video_f.mpg", VideoWriter::fourcc('M', 'J', 'P', 'G'), 30,

	for(;;){
		cap_f >> raw_f;
		cap_r >> raw_r;
		cv::flip(raw_f, raw_f, -1);
		cv::imshow("raw_f", raw_f);

		cv::flip(raw_r, raw_r, -1);
		cv::flip(raw_r, raw_r, 1);
		cv::imshow("raw_r", raw_r);

		rc = cv::waitKey(30);
        if (rc == 113) {
            printf("Exiting...\n");
            return 0;
        }
	}
	return 0;
}

