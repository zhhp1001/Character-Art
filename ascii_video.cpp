#include <chrono>
#include <fstream>
#include <iostream>
//#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <sstream>
#include <stdio.h>
#include "opencv4/opencv2/opencv.hpp"
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <thread>
#include <chrono>

enum class ForgroundColor{
    None = -1,
    black = 30,
    red = 31,
    green = 32,
    yellow = 33,
    blue = 34,
    magenta = 35,
    cyan = 36,
    white = 37
};

enum class BackgroundColor{
    None = -1,
    black = 40,
    red = 41,
    green = 42,
    yellow = 43,
    blue = 44,
    magenta = 45,
    cyan = 46,
    white = 47
};

ForgroundColor convert_fg(const std::string &str) {
    if (str == "black") return ForgroundColor::black;
    else if (str == "red") return ForgroundColor::red;
    else if (str == "green") return ForgroundColor::green;
    else if (str == "yellow") return ForgroundColor::yellow;
    else if (str == "blue") return ForgroundColor::blue;
    else if (str == "magenta") return ForgroundColor::magenta;
    else if (str == "cyan") return ForgroundColor::cyan;
    else if (str == "white") return ForgroundColor::white;

    return ForgroundColor::None;
}

BackgroundColor convert_bg(const std::string &str) {
    if (str == "black") return BackgroundColor::black;
    else if (str == "red") return BackgroundColor::red;
    else if (str == "green") return BackgroundColor::green;
    else if (str == "yellow") return BackgroundColor::yellow;
    else if (str == "blue") return BackgroundColor::blue;
    else if (str == "magenta") return BackgroundColor::magenta;
    else if (str == "cyan") return BackgroundColor::cyan;
    else if (str == "white") return BackgroundColor::white;

    return BackgroundColor::None;
}

int brightness_algo(const cv::Vec3b &color) {
    uchar brightness = (0.0722 * color[0] + 0.7152 * color[1] + 0.2126 * color[2]);

    return brightness;
}

int brightness_algo_average(const cv::Vec3b &color) {
    uchar brightness = (color[0] + color[1] + color[2]) / 3;

    return brightness;
}

char convert_pixel_ascii(int brightness) {
    // 65 characters
    const char *ascii_str= "`^\",:;Il!i~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";

    // Should we cast int to float explicitly?
    int index = static_cast<int>((brightness / 255.0) * 65) - 1;

    return ascii_str[index];
}

char convert_pixel_01(int brightness) {
    char zero_one;
    if (brightness < 128) {
        zero_one = '1';
    } else {
        zero_one = '0';
    }

    return zero_one;
}


void resize_image(cv::Mat &image, cv::Mat &image_resized) {
    struct winsize terminal_size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal_size);
    double terminal_width_ = terminal_size.ws_col / 3;
    double terminal_height_ = terminal_size.ws_row;
    double width_ratio = terminal_width_ / image.size().width;
    double height_ratio = terminal_height_ / image.size().height;
    cv::resize(image, image_resized, cv::Size(), width_ratio, height_ratio);
}

void save_frame_to_txt(cv::Mat &image_resized, int frame_number) {
    std::string frame_txt;

    // Handle every pixel in one frame
    for (int y = 0; y < image_resized.rows; y++) {
        for (int x =0; x < image_resized.cols; x++) {
            cv::Vec3b color = image_resized.at<cv::Vec3b>(cv::Point(x, y));
            int brightness = brightness_algo(color);
            //int brightness = brightness_algo_average(color);
            //char ascii_char = convert_pixel_01(brightness);
            char ascii_char = convert_pixel_ascii(brightness);
            frame_txt += ascii_char;
            frame_txt += ascii_char;
            frame_txt += ascii_char;

            /*
                Foreground color `31m`
                Background color `0m`
                         foreground background
                black        30         40
                red          31         41
                green        32         42
                yellow       33         43
                blue         34         44
                magenta      35         45
                cyan         36         46
                white        37         47

                Refer to:
                https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
            */

            /*
            printf("\033[1;%dm%c%c%c\033[%dm", (int)foreground_color,
                    ascii_char, ascii_char, ascii_char, (int)background_color);
            */
        }
        // newline
        //printf("\n");
        frame_txt += "\n";
    }

    // Frame has been handled
    // Delete the last "\n"
    frame_txt.resize(frame_txt.size() - 1);

    std::string frame_txt_file_name = "frame_txt_" + std::to_string(frame_number) + ".txt";
    std::ofstream frame_txt_file(frame_txt_file_name);
    frame_txt_file << frame_txt;
    frame_txt_file.close();
    frame_txt.clear();
}


// Clear screen with ANSI escap codes
// https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
void clear() {
    // CSI[2J clears screen, CSI[H moves the cursor to top-left corner
    std::cout << "\x1B[2J\x1B[H";
}

void split_frames(cv::Mat &frame, int frame_number) {
    cv::Mat frame_gray;
    cv::cvtColor(frame, frame_gray, cv::COLOR_RGB2GRAY);

    std::string frame_name = "frame" + std::to_string(frame_number) + ".jpg";

    cv::imwrite(frame_name, frame_gray);
}

void show(int total_frames, const int fps) {
    clear();
    auto time_interval = std::chrono::microseconds(std::chrono::seconds(1)) / fps;
    auto target = std::chrono::steady_clock::now();

    for (int i = 1; i <= total_frames; i++) {
        std::string frame_txt_file_name = "frame_txt_" + std::to_string(i) + ".txt";
        std::ifstream stream(frame_txt_file_name);
        std::stringstream strStream;
        strStream << stream.rdbuf();
        std::string frame_string = strStream.str();
        stream.close();

        printf("\r");
        printf("%s", frame_string.c_str());
        printf("\n");

        target += time_interval;
        std::this_thread::sleep_until(target);
    }

}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage:\n\t./character_art image foreground_color background_color\n\
        Available color: black, red, green, yellow, blue, magenta, cyan, white\n");

        return 1;
    }

    clear();

    cv::Mat image_resized;
    /*
    ForgroundColor foreground_color = convert_fg(argv[2]);
    BackgroundColor background_color = convert_bg(argv[3]);
    */

    const char *file_name = argv[1];
    //cv::Mat image = cv::imread(file_name, cv::IMREAD_COLOR);

    cv::VideoCapture cap(file_name);
    if (!cap.isOpened()) {
        std::cout << "Cannot open the video: " << file_name << std::endl;
        return -1;
    }

    int fps = cap.get(cv::CAP_PROP_FPS);
    int total_frames = cap.get(cv::CAP_PROP_FRAME_COUNT);

    std::cout << "FPS: " << fps << std::endl;
    std::cout << "Total frames: " << total_frames << std::endl;

    // TBD: wait for a key to display the video

    int frame_number = 1;

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame_number == total_frames)
            break;

        resize_image(frame, image_resized);

        split_frames(image_resized, frame_number);
        save_frame_to_txt(image_resized, frame_number);

        frame_number++;
    }

    show(total_frames, fps);


    return 0;
}
