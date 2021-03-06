#include <time.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <curses.h>

enum {
    PIPELINE,
    THRESHOLD

};

enum {
    THRESH_CANNY,
    THRESH_LIGHTNESS
};


int main(int argc, char ** argv) {

    cv::VideoCapture cap_f();
    int ch = ' ';
    int mode = THRESHOLD;
    int thresh_adjust = THRESH_CANNY;
    int lines = 0;
    int cols  = 0;
    unsigned int canny_thresh = 100;
    unsigned int lightness = 180;

    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, true);
    keypad(stdscr, true);

    WINDOW * threshold_win;
    WINDOW * pipeline_win;

    threshold_win = newwin((LINES/2)-1, COLS/2, (LINES/2)-1, 0);
    pipeline_win = newwin((LINES/2)-1, COLS/2, 0, COLS/2);

    box(threshold_win, 0, 0);
    box(pipeline_win, 0, 0);

    mvwprintw(threshold_win, 1, 2, "THRESHOLD");
    mvwprintw(threshold_win, 2, 2, "%-15s: %3d", "Canny Thresh", canny_thresh);
    mvwprintw(threshold_win, 3, 2, "%-15s: %3d", "Lightness", lightness);

    getmaxyx(pipeline_win, lines, cols);
    mvwprintw(pipeline_win, 1, 2, "PIPELINE");
    mvwprintw(pipeline_win, 1, cols-31, "%-30s", "Camera Thread:      Stopped");
    mvwprintw(pipeline_win, 2, cols-31, "%-30s", "Threshold Thread:   Stopped");
    mvwprintw(pipeline_win, 3, cols-31, "%-30s", "Line Search Thread: Stopped");
    mvwprintw(pipeline_win, 2, 2, "%-15s: %3d", "Num Lines", 23);

    mvprintw(LINES-1, 1, "%-20s", "Threshold Mode");

    refresh();
    wrefresh(threshold_win);
    wrefresh(pipeline_win);

    while (ch != 'q') {
        ch = getch();

        switch (ch) {
            case 'L':
                mode = PIPELINE;
                mvprintw(LINES-1, 1, "%-20s", "Line Mode");
                mvwprintw(threshold_win, 1, 2, "LINE");
                wrefresh(threshold_win);
                break;

            case 'T':
                mode = THRESHOLD;
                mvprintw(LINES-1, 1, "%-20s", "Threshold Mode");
                mvwprintw(threshold_win, 1, 2, "THRESHOLD");
                wrefresh(threshold_win);
                break;

            case 'o':
                switch (mode) {
                    default:
                        break;
                }
                break;


            case KEY_UP:
                switch (mode) {
                    case THRESHOLD:
                        switch(thresh_adjust) {
                            case THRESH_LIGHTNESS:
                                lightness++;
                                wattron(threshold_win, A_BOLD);
                                mvwprintw(threshold_win, 3, 2, "%-15s: %3d", "Lightness", lightness);
                                wattroff(threshold_win, A_BOLD);
                                wrefresh(threshold_win);
                                break;
                            case THRESH_CANNY:
                                canny_thresh++;
                                wattron(threshold_win, A_BOLD);
                                mvwprintw(threshold_win, 2, 2, "%-15s: %3d", "Canny Thresh", canny_thresh);
                                wattroff(threshold_win, A_BOLD);
                                wrefresh(threshold_win);
                                break;
                            default:
                                break;
                        }

                    default:
                        break;
                }
                break;

            case KEY_DOWN:
                switch (mode) {
                    case THRESHOLD:
                         switch(thresh_adjust) {
                            case THRESH_LIGHTNESS:
                                lightness--;
                                wattron(threshold_win, A_BOLD);
                                mvwprintw(threshold_win, 3, 2, "%-15s: %3d", "Lightness", lightness);
                                wattroff(threshold_win, A_BOLD);
                                wrefresh(threshold_win);
                                break;
                            case THRESH_CANNY:
                                canny_thresh--;
                                wattron(threshold_win, A_BOLD);
                                mvwprintw(threshold_win, 2, 2, "%-15s: %3d", "Canny Thresh", canny_thresh);
                                wattroff(threshold_win, A_BOLD);
                                wrefresh(threshold_win);
                                break;
                            default:
                                break;
                        }

                    default:
                        break;
                }
                break;

            case KEY_LEFT:
                thresh_adjust = THRESH_CANNY;
                getmaxyx(threshold_win, lines, cols);
                wattron(threshold_win, A_BOLD);
                mvwprintw(threshold_win, 2, 2, "%-15s: %3d", "Canny Thresh", canny_thresh);
                wattroff(threshold_win, A_BOLD);
                mvwprintw(threshold_win, 3, 2, "%-15s: %3d", "Lightness", lightness);
                wrefresh(threshold_win);
                break;

            case KEY_RIGHT:
                thresh_adjust = THRESH_LIGHTNESS;
                getmaxyx(threshold_win, lines, cols);
                wattron(threshold_win, A_BOLD);
                mvwprintw(threshold_win, 3, 2, "%-15s: %3d", "Lightness", lightness);
                wattroff(threshold_win, A_BOLD);
                mvwprintw(threshold_win, 2, 2, "%-15s: %3d", "Canny Thresh", canny_thresh);
                wrefresh(threshold_win);
                break;

            default:
                refresh();
                break;
        }
    }

    endwin();

    return 0;
}
