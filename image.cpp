#include "image.h"


Image::Image() {
    w = 0;
    h = 0;
    size = 0;
    rgb_data = nullptr;
    gray = nullptr;
    bin = nullptr;
    qimg = nullptr;
}


Image::Image(QImage* qimage) {
    w = qimage->width();
    h = qimage->height();
    size = h * w;
    qimg = qimage;

    rgb_data = new uint8_t*[size];

    gray = nullptr;
    bin = nullptr;

    for (int pix = 0; pix < size; pix++) {
        rgb_data[pix] = qimg->bits() + pix * CHANNELS;
    }
}


Image::Image(uint8_t* img_values, int img_w, int img_h,  int img_format, QImage* img_qimg) {
    w = img_w;
    h = img_h;
    size = h * w;
    qimg = img_qimg;

    rgb_data = new uint8_t*[size];
    gray = nullptr;
    bin = nullptr;

    for (int pix = 0; pix < size; pix++) {
        rgb_data[pix] = qimg->bits() + pix * CHANNELS;
    }

    if (img_format == FORMAT_GRAY) {
        gray = new uint8_t[size];
        std::memcpy(gray, img_values, sizeof(uint8_t) * size);
    }
    if (img_format == FORMAT_BIN) {
        bin = new uint8_t[size];
        std::memcpy(bin, img_values, sizeof(uint8_t) * size);
    }
}


Image::~Image() {
    delete[] rgb_data;
    delete[] gray;
    delete[] bin;
}


Image::Image(const Image& other) {
    w = other.w;
    h = other.h;
    size = other.size;
    delete[] rgb_data;
    delete[] gray;
    delete[] bin;
    rgb_data = other.rgb_data;
    gray = other.gray;
    bin = other.bin;
    qimg = other.qimg;
}


Image::Image(Image&& other) {
    w = other.w;
    h = other.h;
    size = other.size;
    delete[] rgb_data;
    delete[] gray;
    delete[] bin;
    rgb_data = other.rgb_data;
    gray = other.gray;
    bin = other.bin;
    qimg = other.qimg;
    other.rgb_data = nullptr;
    other.qimg = nullptr;
    other.gray = nullptr;
    other.bin = nullptr;
}


Image& Image::operator=(const Image& other) {
    if (this == &other) {
        return *this;
    }
    w = other.w;
    h = other.h;
    size = other.size;
    delete[] rgb_data;
    delete[] gray;
    delete[] bin;
    rgb_data = other.rgb_data;
    gray = other.gray;
    bin = other.bin;
    qimg = other.qimg;
    return *this;
}


Image& Image::operator=(Image&& other) {
    if (this == &other) {
        return *this;
    }
    w = other.w;
    h = other.h;
    size = other.size;
    delete[] rgb_data;
    delete[] gray;
    delete[] bin;
    rgb_data = other.rgb_data;
    gray = other.gray;
    bin = other.bin;
    qimg = other.qimg;
    other.rgb_data = nullptr;
    other.gray = nullptr;
    other.bin = nullptr;
    other.qimg = nullptr;
    return *this;
}


int Image::width() {
    return w;
}


int Image::height() {
    return h;
}


void Image::createGray() {
    gray = new uint8_t[size];
    for (int pix = 0; pix < size; pix++) {
        gray[pix] = int(GRAY_R * rgb_data[pix][RED] + GRAY_G * rgb_data[pix][GREEN] + GRAY_B * rgb_data[pix][BLUE]);
    }
}

void Image::createBin() {
    if (gray == nullptr) {
        createGray();
    }
    bin = new uint8_t[size];
    for (int pix = 0; pix < size; pix++) {
        bin[pix] = (gray[pix] > BIN_THRESHOLD) ? 255 : 0;
    }
}


Image Image::Rgb() {   
    return Image(nullptr, w, h, FORMAT_RGB, qimg);
}


Image Image::Gray() {
    if (gray == nullptr) {
        createGray();
    }
    return Image(gray, w, h, FORMAT_GRAY, qimg);
}


Image Image::Bin() {
    if (bin == nullptr) {
        createBin();
    }
    return Image(bin, w, h, FORMAT_BIN, qimg);
}


QImage Image::Image2QImage(int format) {
    if (qimg == nullptr) {
        return QImage();
    }

    if (format == FORMAT_RGB) {
        return *qimg;
    }
    if (format == FORMAT_GRAY) {
        if (gray == nullptr) {
            createGray();
        }
        return QImage(gray, w, h, QImage::Format_Grayscale8);
    }
    if (format == FORMAT_BIN) {
        if (bin == nullptr) {
            createBin();
        }
        return QImage(bin, w, h, QImage::Format_Grayscale8);
    }

    return QImage();
}


int Image::multiply(uint8_t* img, double* kernel) {
    double sum = 0;
    for (int i = 0; i < KERNEL_SIZE; i++) {
        sum += img[i] * kernel[i];
    }
    return sum;
}


Image Image::conv(double* kernel) {
    if (gray == nullptr) {
        createGray();
    }

    uint8_t new_img[size];
    uint8_t matrix[KERNEL_SIZE];

    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            int filled_rows = 0;
            for (int i = y - 1; i < y + 2; i++) {
                uint8_t* src = gray + sizeof(uint8_t) * (w * (y - 1) + x - 1);
                uint8_t* dst = matrix + sizeof(uint8_t) * filled_rows * KERNEL_EDGE;
                std::memcpy(dst, src, sizeof(uint8_t) * KERNEL_EDGE);
                filled_rows++;
            }
            new_img[y*w+x] = multiply(matrix, kernel);
        }
    }
    return Image(new_img, w, h, FORMAT_GRAY, qimg);
}


Image Image::sobel(bool vertical) {
    double matrix_x[] = {1., 0., -1.,
                         2., 0., -2.,
                         1., 0., -1.};

    double matrix_y[] = { 1.,  2.,  1.,
                          0.,  0.,  0.,
                         -1., -2., -1.};

    if (vertical) {
        return conv(matrix_x);
    }
    return conv(matrix_y);
}


bool Image::checkEdges(int x, int y, int h, int w) {
    if (y < 0  || x < 0 || h <= y || w <= x) {
        return false;
    }
    return true;
}


uint8_t* Image::findNeighbors(uint8_t* img, int x, int y, int w, int h) {
    uint8_t* neighbors = new uint8_t[NEIGHBORS];

    int cell = 0;
    for (int i = y - 1; i < y + 2; i++) {
        for (int j = x - 1; j < x + 2; j++) {
            if (i == y && j == x) {
                continue;
            }

            neighbors[cell] = checkEdges(j, i, h, w) ? img[i*w+j] : 0;
            cell++;
        }
    }
    return neighbors;
}


bool Image::checkNeighbors(uint8_t* img, int x, int y, int w, int h, bool find_high) {
    uint8_t pixel;

    for (int i = y - 1; i < y + 2; i++) {
        for (int j = x - 1; j < x + 2; j++) {
            if (i == y && j == x) {
                continue;
            }

            pixel = checkEdges(j, i, h, w) ? img[i*w+j] : 0;

            if (pixel && find_high || !pixel && !find_high) {
                return true;
            }
        }
    }
    return false;
}


Image Image::externalContouring() {
    uint8_t empty[size];
    memset(empty, 0, size);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (bin[y*w+x] == 255) {
                empty[y*w+x] = 0;
                continue;
            }
            if (checkNeighbors(bin, x, y, w, h, true)) {
                empty[y*w+x] = 255;
            }
        }
    }
    return Image(empty, w, h, FORMAT_BIN, qimg);
}


Image Image::dilate() {
    uint8_t empty[size];
    memset(empty, 0, size);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (checkNeighbors(bin, x, y, w, h, true)) {
                empty[y*w+x] = 255;
            }
        }
    }
    return Image(empty, w, h, FORMAT_BIN, qimg);
}


Image Image::erode() {
    uint8_t empty[size];
    std::memcpy(empty, bin, size);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (checkNeighbors(bin, x, y, w, h, false)) {
                empty[y*w+x] = 0;
            }
        }
    }
    return Image(empty, w, h, FORMAT_BIN, qimg);
}


bool Image::compareNeighbors(uint8_t* img, int x, int y, int w, int h, double k) {
    uint8_t pixel = int(k * img[y*w+x]);
    uint8_t nb;

    for (int i = y - 1; i < y + 2; i++) {
        for (int j = x - 1; j < x + 2; j++) {
            if (i == y && j == x) {
                continue;
            }

            nb = checkEdges(j, i, h, w) ? img[i*w+j] : 255;
            if (pixel < nb) {
                return true;
            }
        }
    }
    return false;
}


Image Image::fd(double k) {
    uint8_t empty[size];
    memset(empty, 0, size);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (compareNeighbors(gray, x, y, w, h, k)) {
                empty[y*w+x] = 255;
            }
        }
    }
    return Image(empty, w, h, FORMAT_BIN, qimg);
}
