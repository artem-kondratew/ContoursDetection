#ifndef IMAGE_H
#define IMAGE_H


#include <cmath>
#include <cstring>
#include <memory>
#include <QMainWindow>


#define GRAY_R 0.299
#define GRAY_G 0.587
#define GRAY_B 0.114

#define RED   0
#define GREEN 1
#define BLUE  2

#define CHANNELS 3

#define KERNEL_SIZE 9

#define BIN_THRESHOLD 128

#define FORMAT_RGB  0
#define FORMAT_GRAY 1
#define FORMAT_BIN  2
#define FORMAT_NULL 3

#define NEIGHBORS 8

#define LOWER_THRESHOLD 10
#define UPPER_THRESHOLD 50

#define HIGH 255
#define LOW    0


class Exception: std::exception {
private:
    std::string message;
public:
    explicit Exception(std::string input_message) {message = std::move(input_message);};

    std::string getMessage() const {return message;};
};


class Image {
private:
    int w;
    int h;
    unsigned int size;
    int format;

    std::shared_ptr<uint8_t[]> rgb;
    std::shared_ptr<uint8_t[]> gray;
    std::shared_ptr<uint8_t[]> bin;

public:
    Image();
    Image(std::shared_ptr<QImage> qimage);
    Image(std::shared_ptr<uint8_t[]>, int img_w, int img_h, int img_format);
    ~Image() = default;
    Image(const Image &other);
    Image(Image &&other);
    Image& operator=(const Image &other);
    Image& operator=(Image &&other);

    int Width();
    int Height();
    int Format();

private:
    void createGray();
    void createBin(uint8_t threshold);

public:
    Image Rgb();
    Image Gray();
    Image Bin(uint8_t threshold=BIN_THRESHOLD);

    QImage Image2QImage(int format);

    bool isNull();

private:
    int multiply(const uint8_t* submatrix, double* kernel, int kernel_size, double k);
    std::shared_ptr<uint8_t[]> convolution(const uint8_t*  values, double* kernel, double k);

    std::shared_ptr<uint8_t[]> multiplyRgb(const uint8_t* submatrix, double* kernel, int kernel_size, double k);
    std::shared_ptr<uint8_t[]> convolutionRgb(const uint8_t*  values, double* kernel, double k);

public:
    Image Convolution(double* kernel, double k);

private:
    std::shared_ptr<uint8_t[]> gaussianBlur(int format);

public:
    Image GaussianBlur(int format);

private:
    std::shared_ptr<uint8_t[]> sobel(uint8_t* values, bool vertical);

public:
    Image Sobel(bool vertical, int format);

private:
    bool checkEdges(int x, int y, int w, int h);

    std::shared_ptr<uint16_t[]> findNeighbors(const uint16_t* img, int x, int y, int w, int h);
    std::shared_ptr<uint8_t[]> findNeighbors(const uint8_t* img, int x, int y, int w, int h);

    bool checkNeighbors(const uint8_t* img, int x, int y, int w, int h, uint8_t key);

public:
    Image ExternalContouring();
    Image Dilate();
    Image Erode();

private:
    bool compareNeighbors(const uint8_t* img, int x, int y, int w, int h, double k);

public:
    Image FindDark(double k=1.0);

private:
    uint8_t getGradientDirection(uint8_t x, uint8_t y);
    uint16_t suppressNonMax(const uint16_t* neighbors, uint16_t value, uint8_t dir);
    std::shared_ptr<uint8_t[]> doubleThreshold(const uint16_t* img, uint16_t bottom_val, uint16_t top_val);
    std::shared_ptr<uint8_t[]> checkStrongIds(const uint8_t* db_threshold);

public:
    Image Canny(uint16_t lower_threshold=LOWER_THRESHOLD, uint16_t upper_threshold=UPPER_THRESHOLD);

    Image Flip(bool vertical);
};


#endif // IMAGE_H
