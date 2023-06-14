#ifndef IMAGE_H
#define IMAGE_H


#include <QMainWindow>


#define GRAY_R 0.299
#define GRAY_G 0.587
#define GRAY_B 0.114


class Pixel {
public:
    int r, g, b;
public:
    Pixel(int red, int green, int blue);
    ~Pixel() = default;
};


class Image {
private:
    int w;
    int h;
    int** r;
    int** g;
    int** b;
    int** gray;

private:
    int** createDoubleArray(int h, int w);
    void createGray();

public:
    Image(QImage* img);

    Pixel rgb_pixel(int y, int x);
    QImage Image2QImage();
    QImage Gray2QImage();

    int width();
    int height();
};


#endif // IMAGE_H
