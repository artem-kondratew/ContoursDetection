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

#define CHANNELS 3

#define KERNEL_SIZE 9
#define KERNEL_EDGE 3

#define BIN_THRESHOLD 128

#define FORMAT_RGB  0
#define FORMAT_GRAY 1
#define FORMAT_BIN  2

#define NEIGHBORS 8


class Image {
private:
    int w;
    int h;
    unsigned int size;
    QImage* qimg;

    uint8_t** rgb_data;

    uint8_t* gray;
    uint8_t* bin;

public:
    Image();
    Image(QImage* qimage);
    Image(uint8_t* img_values, int img_w, int img_h, int img_format, QImage* img_qimg);
    ~Image();
    Image(const Image &other);
    Image(Image &&other);
    Image& operator=(const Image &other);
    Image& operator=(Image &&other);

    int width();
    int height();

private:
    void createGray();
    void createBin();

public:
    Image Rgb();
    Image Gray();
    Image Bin();

    QImage Image2QImage(int format);

private:
    int multiply(uint8_t* img, double* kernel);

public:
    Image conv(double* conv);

    Image sobel(bool vertical);

private:
    bool checkEdges(int x, int y, int h, int w);
    uint8_t* findNeighbors(uint8_t* img, int x, int y, int w, int h);
    bool checkNeighbors(uint8_t* img, int x, int y, int w, int h, bool find_high);

public:
    Image externalContouring();
    Image dilate();
    Image erode();
private:
    bool compareNeighbors(uint8_t* img, int x, int y, int w, int h, double k);

public:
    Image fd(double k=1.0);
};


#endif // IMAGE_H
