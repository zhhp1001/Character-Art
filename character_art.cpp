#include <iostream>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgcodecs.hpp>
#include <stdio.h>
#include "opencv4/opencv2/opencv.hpp"
#include <sys/ioctl.h>
#include <unistd.h>

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

class CharcterArt {
    public:
        CharcterArt(const char *file_name,
                ForgroundColor foreground_color, BackgroundColor background_color);
        virtual ~CharcterArt();

        void show();

    private:
        virtual int brightness_algo(const cv::Vec3b &color) const = 0;
        virtual char convert_pixel(int brightness) const = 0;
        void resize_image();

        const char *file_name_;
        cv::Mat image_;
        cv::Mat image_resized_;
        double terminal_width_;
        double terminal_height_;
        ForgroundColor foreground_color_;
        BackgroundColor background_color_;
};

class AsciiArt : public CharcterArt {
    public:
        AsciiArt(const char *file_name,
                ForgroundColor foreground_color, BackgroundColor background_color);
        ~AsciiArt();

    private:
        int brightness_algo(const cv::Vec3b &color) const override;
        char convert_pixel(int brightness) const override;
};

class ZeroOneArt : public CharcterArt {
    public:
        ZeroOneArt(const char *file_name,
                ForgroundColor foreground_color, BackgroundColor background_color);
        ~ZeroOneArt();

    private:
        int brightness_algo(const cv::Vec3b &color) const override;
        char convert_pixel(int brightness) const override;
};

CharcterArt::CharcterArt(const char *file_name, ForgroundColor foreground_color,
        BackgroundColor background_color) : file_name_(file_name),
                                            foreground_color_(foreground_color),
                                            background_color_(background_color){

    image_ = cv::imread(file_name_, cv::IMREAD_COLOR);

    // Resize image to fit terminal size
    resize_image();
}

CharcterArt::~CharcterArt() {
}

AsciiArt::AsciiArt(const char *file_name, ForgroundColor foreground_color,
        BackgroundColor background_color) : CharcterArt(file_name, foreground_color, background_color) {
}

ZeroOneArt::ZeroOneArt(const char *file_name, ForgroundColor foreground_color,
        BackgroundColor background_color) : CharcterArt(file_name, foreground_color, background_color) {
}

AsciiArt::~AsciiArt() {
}

ZeroOneArt::~ZeroOneArt(){
}

int AsciiArt::brightness_algo(const cv::Vec3b &color) const {
    uchar brightness = (0.0722 * color[0] + 0.7152 * color[1] + 0.2126 * color[2]);

    return brightness;
}

int ZeroOneArt::brightness_algo(const cv::Vec3b &color) const {
    uchar brightness = (color[0] + color[1] + color[2]) / 3;

    return brightness;
}

char AsciiArt::convert_pixel(int brightness) const {
    // 65 characters
    const char *ascii_str= "`^\",:;Il!i~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";

    // Should we cast int to float explicitly?
    int index = static_cast<int>((brightness / 255.0) * 65) - 1;

    return ascii_str[index];
}

char ZeroOneArt::convert_pixel(int brightness) const {
    char zero_one;
    if (brightness < 128) {
        zero_one = '1';
    } else {
        zero_one = '0';
    }

    return zero_one;
}


void CharcterArt::resize_image() {
    struct winsize terminal_size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal_size);
    terminal_width_ = terminal_size.ws_col / 3;
    terminal_height_ = terminal_size.ws_row;
    double width_ratio = terminal_width_ / image_.size().width;
    double height_ratio = terminal_height_ / image_.size().height;
    cv::resize(image_, image_resized_, cv::Size(), width_ratio, height_ratio);
}

void CharcterArt::show() {
    for (int y = 0; y < image_resized_.rows; y++) {
        for (int x =0; x < image_resized_.cols; x++) {
            cv::Vec3b color = image_resized_.at<cv::Vec3b>(cv::Point(x, y));
            //int brightness = lumi_algo(color);
            int brightness = brightness_algo(color);
            //char ascii_char = bright2char(brightness);
            char ascii_char = convert_pixel(brightness);

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
            printf("\033[1;%dm%c%c%c\033[%dm", (int)foreground_color_,
                    ascii_char, ascii_char, ascii_char, (int)background_color_);
        }
        // newline
        printf("\n");
    }
}

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

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage:\n\t./character_art image ascii/zero foreground_color background_color\n\
        Available color: black, red, green, yellow, blue, magenta, cyan, white\n");

        return 1;
    }

    if (!strncmp(argv[2], "ascii", 5)) {
        AsciiArt *ascii_art = new AsciiArt(argv[1], convert_fg(argv[3]), convert_bg(argv[4]));
        ascii_art->show();
        delete ascii_art;
    } else if (!strncmp(argv[2], "zero", 4)) {
        ZeroOneArt *zero_one = new ZeroOneArt(argv[1], convert_fg(argv[3]), convert_bg(argv[4]));
        zero_one->show();
        delete zero_one;
    } else {

    }

    return 0;
}
