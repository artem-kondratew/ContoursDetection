#ifndef IMAGE_H
#define IMAGE_H


#include <cstring>
#include <QMainWindow>


#define GRAY_R 0.299
#define GRAY_G 0.587
#define GRAY_B 0.114

#define RED   0
#define GREEN 1
#define BLUE  2

#define KERNEL_SIZE 9
#define KERNEL_EDGE 3

#define FORMAT_RGB  3
#define FORMAT_GRAY 1


class Image {
private:
    int format;
    int w;
    int h;
    int size;
    QImage* qimg;

    uint8_t* values;
    uint8_t** data;

public:
    Image();
    Image(QImage* qimage);
    Image(uint8_t* img_values, int img_w, int img_h, int img_format);
    ~Image();
    Image(const Image &other);
    Image(Image &&other);
    Image& operator=(const Image &other);
    Image& operator=(Image &&other);

    bool isExists();
    int width();
    int height();
    QImage QImg();

    Image Gray();
    Image Rgb();

    QImage Image2QImage();

    int multiply(uint8_t* img, double* kernel);
    Image conv(double* conv);
};


#endif // IMAGE_H
