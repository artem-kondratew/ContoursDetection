#include "image.h"


Image::Image() {
    format = -1;
    w = 0;
    h = 0;
    size = 0;
    values = nullptr;
    data = nullptr;
    qimg = nullptr;
}


Image::Image(QImage* qimage) {
    format = FORMAT_RGB;
    w = qimage->width();
    h = qimage->height();
    size = h * w;
    qimg = qimage;

    values = new uint8_t[size * format];
    data = new uint8_t*[size];

    std::memcpy(values, qimg->bits(), sizeof(uint8_t) * size * format);

    for (int pix = 0; pix < size; pix++) {
        data[pix] = values + pix * format;
    }
}


Image::Image(uint8_t* img_values, int img_w, int img_h, int img_format) {
    format = img_format;
    w = img_w;
    h = img_h;
    size = w * h;
    qimg = nullptr;

    values = new uint8_t[size * format];
    data = new uint8_t*[size];

    std::memcpy(values, img_values, sizeof(uint8_t) * size * format);
    for (int pix = 0; pix < size; pix++) {
        data[pix] = values + pix * format;
    }
}


Image::~Image() {
    delete[] values;
    delete[] data;
}


Image::Image(const Image& other) {
    format = other.format;
    w = other.w;
    h = other.h;
    size = other.size;
    delete[] values;
    delete[] data;
    values = other.values;
    data = other.data;
    qimg = other.qimg;
}


Image::Image(Image&& other) {
    format = other.format;
    w = other.w;
    h = other.h;
    size = other.size;
    delete[] values;
    delete[] data;
    values = other.values;
    data = other.data;
    qimg = other.qimg;
    other.values = nullptr;
    other.data = nullptr;
    other.qimg = nullptr;
}


Image& Image::operator=(const Image& other) {
    if (this == &other) {
        return *this;
    }
    format = other.format;
    w = other.w;
    h = other.h;
    size = other.size;
    delete[] values;
    delete[] data;
    values = other.values;
    data = other.data;
    qimg = other.qimg;
    return *this;
}


Image& Image::operator=(Image&& other) {
    if (this == &other) {
        return *this;
    }
    format = other.format;
    w = other.w;
    h = other.h;
    size = other.size;
    delete[] values;
    delete[] data;
    values = other.values;
    data = other.data;
    qimg = other.qimg;
    other.values = nullptr;
    other.data = nullptr;
    other.qimg = nullptr;
    return *this;
}


bool Image::isExists() {
    return size != 0;
}


int Image::width() {
    return w;
}


int Image::height() {
    return h;
}


QImage Image::QImg() {
    return *qimg;
}


Image Image::Rgb() {
    Image img;
    img.format = FORMAT_RGB;
    img.w = w;
    img.h = h;
    img.size = size;
    img.qimg = qimg;

    img.values = new uint8_t[size * img.format];
    img.data = new uint8_t*[size];

    if (format == FORMAT_RGB) {
        std::memcpy(img.values, values, sizeof(uint8_t) * size * format);
        for (int pix = 0; pix < size; pix++) {
            img.data[pix] = img.values + pix * format;
        }
        return img;
    }
    return Image();
}


Image Image::Gray() {
    Image img;
    img.format = FORMAT_GRAY;
    img.w = w;
    img.h = h;
    img.size = h * w;
    img.qimg = qimg;

    img.values = new uint8_t[img.size];
    img.data = new uint8_t*[img.size];

    if (format == FORMAT_RGB) {
        for (int pix = 0; pix < img.size; pix++) {
            img.values[pix] = int(GRAY_R * data[pix][RED] + GRAY_G * data[pix][GREEN] + GRAY_B * data[pix][BLUE]);
            img.data[pix] = img.values + pix * format;
        }
        return img;
    }

    if (format == FORMAT_GRAY) {
        for (int pix = 0; pix < size; pix++) {
            std::memcpy(img.values, values, sizeof(uint8_t) * size);
            for (int pix = 0; pix < size; pix++) {
                img.data[pix] = img.values + pix;
            }
        }
        return img;
    }
    return img;
}


QImage Image::Image2QImage() {
    if (format == FORMAT_RGB) {
        return QImage(values, w, h, QImage::Format_RGB888);
    }
    if (format == FORMAT_GRAY) {
        return QImage(values, w, h, QImage::Format_Grayscale8);
    }
    return QImage();
}


int Image::multiply(uint8_t* img, double* kernel) {
    int sum = 0;
    for (int i = 0; i < KERNEL_SIZE; i++) {
        sum += img[i] * kernel[i];
    }
    return sum;
}


Image Image::conv(double* kernel) {
    if (format != FORMAT_GRAY) {
        return Image();
    }
    uint8_t* matrix = new uint8_t[KERNEL_SIZE];
    uint8_t new_img[size];
    for (int y = 1; y < h-1; y++) {
        for (int x = 1; x < w-1; x++) {
            for (int i = y - 1; i < y + 2; i++) {
                int filled_rows = 0;
                uint8_t* src = values + sizeof(uint8_t) * (w * (y - 1) + x - 1);
                uint8_t* dst = matrix + sizeof(uint8_t) * filled_rows * KERNEL_EDGE;
                std::memcpy(dst, src, sizeof(uint8_t) * KERNEL_EDGE);
                filled_rows++;
            }
            new_img[y*w+x] = multiply(matrix, kernel);
        }
    }
    delete[] matrix;
    return Image(new_img, w, h, FORMAT_GRAY);
}
