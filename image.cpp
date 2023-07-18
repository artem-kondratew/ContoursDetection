#include "image.h"


Image::Image() {
    w = 0;
    h = 0;
    size = 0;
    format = FORMAT_NULL;
    rgb = nullptr;
    gray = nullptr;
    bin = nullptr;
}


Image::Image(std::shared_ptr<QImage> qimage) {
    if (qimage == nullptr) {
        throw Exception("qimage is nullptr");
    }

    w = qimage->width();
    h = qimage->height();
    size = h * w;
    format = FORMAT_RGB;

    rgb = std::shared_ptr<uint8_t[]>(new uint8_t[size*CHANNELS]);
    std::memcpy(rgb.get(), qimage->bits(), sizeof(uint8_t) * size * CHANNELS);

    gray = nullptr;
    bin = nullptr;
}


Image::Image(std::shared_ptr<uint8_t[]> img_values, int img_w, int img_h,  int img_format) {
    if (img_values == nullptr) {
        throw Exception("rgb_values is nullptr");
    }

    w = img_w;
    h = img_h;
    size = h * w;
    format = img_format;

    rgb = nullptr;
    gray = nullptr;
    bin = nullptr;

    if (format == FORMAT_RGB) {
        rgb = img_values;
    }

    if (format == FORMAT_GRAY) {
        gray = img_values;
    }

    if (format == FORMAT_BIN) {
        bin = img_values;
    }
}


Image::Image(const Image& other) {
    w = other.w;
    h = other.h;
    size = other.size;
    format = other.format;
    rgb = other.rgb;
    gray = other.gray;
    bin = other.bin;
}


Image::Image(Image&& other) noexcept {
    w = other.w;
    h = other.h;
    size = other.size;
    format = other.format;
    rgb = other.rgb;
    gray = other.gray;
    bin = other.bin;
    other.rgb.reset();
    other.gray.reset();
    other.bin.reset();
}


Image& Image::operator=(const Image& other) {
    if (this == &other) {
        return *this;
    }
    w = other.w;
    h = other.h;
    size = other.size;
    format = other.format;
    rgb = other.rgb;
    gray = other.gray;
    bin = other.bin;
    return *this;
}


Image& Image::operator=(Image&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    w = other.w;
    h = other.h;
    size = other.size;
    format = other.format;
    rgb = other.rgb;
    gray = other.gray;
    bin = other.bin;
    other.rgb.reset();
    other.gray.reset();
    other.bin.reset();
    return *this;
}


int Image::Width() {
    return w;
}


int Image::Height() {
    return h;
}


int Image::Format() {
    return format;
}


void Image::createGray() {
    if (rgb == nullptr) {
        throw Exception("rgb_data is nullptr");
    }
    gray = std::shared_ptr<uint8_t[]>(new uint8_t[size]);
    for (int pix = 0; pix < size; pix++) {
        gray[pix] = int(GRAY_R * rgb[3*pix+RED] + GRAY_G * rgb[3*pix+GREEN] + GRAY_B * rgb[3*pix+BLUE]);
    }
}


void Image::createBin(uint8_t threshold) {
    if (gray == nullptr) {
        throw Exception("gray is nullptr");
    }

    bin = std::shared_ptr<uint8_t[]>(new uint8_t[size]);

    /*for (int pix = 0; pix < size; pix++) {
        bin[pix] = gray[pix] >= threshold ? HIGH : LOW;
    }*/

    __m128i thr = _mm_set1_epi8(threshold);
    __m128i high = _mm_set1_epi8((uint8_t)HIGH);
    __m128i low = _mm_set1_epi8((uint8_t)LOW);

    for (int pix = 0; pix < size; pix += 16) {
        __m128i in_reg = _mm_loadu_si128((__m128i*)(gray.get() + pix));

        __m128i mask = _mm_cmpeq_epi8( in_reg, _mm_max_epu8(in_reg, thr));
        __m128i out_reg = _mm_blendv_epi8(low, high, mask);

        _mm_storeu_si128((__m128i*)(bin.get() + pix), out_reg);
    }
}


Image Image::Rgb() {
    return Image(rgb, w, h, FORMAT_RGB);
}


Image Image::Gray() {
    if (gray == nullptr) {
        createGray();
    }
    return Image(gray, w, h, FORMAT_GRAY);
}


Image Image::Bin(uint8_t threshold) {
    if (bin == nullptr) {
        createBin(threshold);
    }
    return Image(bin, w, h, FORMAT_BIN);
}


QImage Image::Image2QImage(int format) {
    if (format == FORMAT_RGB) {
        if (rgb == nullptr) {
            throw Exception("rgb is nullptr");
        }
        return QImage(rgb.get(), w, h, QImage::Format_RGB888);
    }
    if (format == FORMAT_GRAY) {
        if (gray == nullptr) {
            throw Exception("gray is nullptr");
        }
        return QImage(gray.get(), w, h, QImage::Format_Grayscale8);
    }
    if (format == FORMAT_BIN) {
        if (bin == nullptr) {
            throw Exception("bin is nullptr");
        }
        return QImage(bin.get(), w, h, QImage::Format_Grayscale8);
    }
    throw Exception("wrong format input");
}


bool Image::isNull() {
    return format == FORMAT_NULL;
}


/*int Image::multiply(const uint8_t* submatrix, const double* kernel, int kernel_size, double k) {
    double sum = 0;
    for (int i = 0; i < kernel_size; i++) {
        sum += submatrix[i] * kernel[i];
    }
    int res = std::abs(sum / k);
    return res > HIGH ? HIGH : res;
}*/


std::shared_ptr<uint8_t[]> Image::convolution(const uint8_t* matrix, const double* kernel, double k) {
    int kernel_edge = 3;
    int kernel_size = kernel_edge * kernel_edge;
    std::shared_ptr<uint8_t[]> empty(new uint8_t[size]);
    uint8_t submatrix[kernel_size];

    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            int cell = 0;
                for (int i = y - 1; i < y + 2; i++) {
                    for (int j = x - 1; j < x + 2; j++) {
                        submatrix[cell] = matrix[i*w+j];
                        cell++;
                    }
                }
            //empty[y*w+x] = multiply(submatrix, kernel, kernel_size, k);
            double sum = 0;
            for (int i = 0; i < kernel_size; i++) {
                sum += submatrix[i] * kernel[i];
            }
            int res = std::abs(sum / k);
            empty[y*w+x] = res > HIGH ? HIGH : res;
        }
    }
    return empty;
}


Image Image::Convolution(double *kernel, double k) {
    if (format == FORMAT_RGB) {
        throw Exception("rgb is not supported");
    }
    if (format == FORMAT_GRAY) {
        return Image(convolution(gray.get(), kernel, k), w, h, format);
    }
    if (format == FORMAT_BIN) {
        return Image(convolution(bin.get(), kernel, k), w, h, format);
    }
    throw Exception("wrong format input");
}


std::shared_ptr<uint8_t[]> Image::gaussianBlur(int format) {
    double matrix3[] = {1., 2., 1.,
                        2., 4., 2.,
                        1., 2., 1.};

    double k3 = 16.;

    if (format == FORMAT_RGB) {
        throw Exception("rgb is not supported");
    }
    if (format == FORMAT_GRAY) {
        return convolution(gray.get(), matrix3, k3);
    }
    if (format == FORMAT_BIN) {
        return convolution(bin.get(), matrix3, k3);
    }
    throw Exception("wrong format input");
}


Image Image::GaussianBlur(int format) {
    return Image(gaussianBlur(format), w, h, format);
}


std::shared_ptr<uint8_t[]> Image::sobel(uint8_t* values, bool vertical) {
    double matrix_x[] = {-1., 0., 1.,
                           -2., 0., 2.,
                           -1., 0., 1.};

    double matrix_y[] = { 1.,  2.,  1.,
                            0.,  0.,  0.,
                           -1., -2., -1.};

    double k = 1;

    if (vertical) {
        return convolution(values, matrix_x, k);
    }
    return convolution(values, matrix_y, k);
}


Image Image::Sobel(bool vertical, int format) {
    uint8_t* values = nullptr;

    if (format == FORMAT_RGB) {
        throw Exception("rgb is not supported");
    }

    if (format == FORMAT_GRAY) {
        values = gray.get();
    }
    else if (format == FORMAT_BIN) {
        values = bin.get();
    }
    else {
        throw Exception("wrong format input");
    }

    return Image(sobel(values, vertical), w, h, format);
}


bool Image::checkEdges(int x, int y, int w, int h) {
    if (y < 0  || x < 0 || h <= y || w <= x) {
        return false;
    }
    return true;
}


std::shared_ptr<uint16_t[]> Image::findNeighbors(const uint16_t* img, int x, int y, int w, int h) {
    std::shared_ptr<uint16_t[]> neighbors(new uint16_t[NEIGHBORS]);

    int cell = 0;
    for (int i = y - 1; i < y + 2; i++) {
        for (int j = x - 1; j < x + 2; j++) {
            if (i == y && j == x) {
                continue;
            }

            neighbors[cell] = checkEdges(j, i, w, h) ? img[i*w+j] : LOW;
            cell++;
        }
    }
    return neighbors;
}


std::shared_ptr<uint8_t[]> Image::findNeighbors(const uint8_t* img, int x, int y, int w, int h) {
    std::shared_ptr<uint8_t[]> neighbors(new uint8_t[NEIGHBORS]);

    int cell = 0;
    for (int i = y - 1; i < y + 2; i++) {
        for (int j = x - 1; j < x + 2; j++) {
            if (i == y && j == x) {
                continue;
            }

            neighbors[cell] = checkEdges(j, i, w, h) ? img[i*w+j] : LOW;
            cell++;
        }
    }
    return neighbors;
}


bool Image::checkNeighbors(const uint8_t* img, int x, int y, int w, int h, uint8_t key) {
    for (int i = y - 1; i < y + 2; i++) {
        for (int j = x - 1; j < x + 2; j++) {
            if (i == y && j == x) {
                continue;
            }

            uint8_t pixel = checkEdges(j, i, w, h) ? img[i*w+j] : LOW;

            if (pixel == key) {
                return true;
            }
        }
    }
    return false;
}


Image Image::ExternalContouring() {
    std::shared_ptr<uint8_t[]> empty(new uint8_t[size]);
    memset(empty.get(), LOW, size);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (bin[y*w+x] == HIGH) {
                empty[y*w+x] = LOW;
                continue;
            }
            if (checkNeighbors(bin.get(), x, y, w, h, HIGH)) {
                empty[y*w+x] = HIGH;
            }
        }
    }
    return Image(empty, w, h, FORMAT_BIN);
}


Image Image::Dilate() {
    std::shared_ptr<uint8_t[]> dilate(new uint8_t[size]);
    memset(dilate.get(), LOW, size);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (checkNeighbors(bin.get(), x, y, w, h, HIGH)) {
                dilate[y*w+x] = HIGH;
            }
        }
    }
    return Image(dilate, w, h, FORMAT_BIN);
}


Image Image::Erode() {
    std::shared_ptr<uint8_t[]> erode(new uint8_t[size]);
    std::memcpy(erode.get(), bin.get(), size);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (checkNeighbors(bin.get(), x, y, w, h, LOW)) {
                erode[y*w+x] = LOW;
            }
        }
    }
    return Image(erode, w, h, FORMAT_BIN);
}


bool Image::compareNeighbors(const uint8_t* img, int x, int y, int w, int h, double k) {
    uint8_t pixel = int(k * img[y*w+x]);
    uint8_t nb;

    for (int i = y - 1; i < y + 2; i++) {
        for (int j = x - 1; j < x + 2; j++) {
            if (i == y && j == x) {
                continue;
            }

            nb = checkEdges(j, i, w, h) ? img[i*w+j] : LOW;
            if (pixel < nb) {
                return true;
            }
        }
    }
    return false;
}


Image Image::FindDark(double k) {
    std::shared_ptr<uint8_t[]> fd(new uint8_t[size]);
    memset(fd.get(), LOW, size);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (compareNeighbors(gray.get(), x, y, w, h, k)) {
                fd[y*w+x] = HIGH;
            }
        }
    }
    return Image(fd, w, h, FORMAT_BIN);
}


uint8_t Image::getGradientDirection(uint8_t x, uint8_t y) {
    double rad = std::atan(y / double(x));
    int angle = rad * 180 / M_PI;

    if (-45 - 22.5 <= angle && angle < -45 + 22.5) {
        return 1;
    }
    if (-22.5 <= angle && angle < 22.5) {
        return 2;
    }
    if (45 - 22.5 <= angle && angle < 45 + 22.5) {
        return 3;
    }
    return 0;
}


uint16_t Image::suppressNonMax(const uint16_t* neighbors, uint16_t value, uint8_t dir) {
    uint16_t nb1 = LOW;
    uint16_t nb2 = LOW;

    if (dir == 0) {
        nb1 = neighbors[1];
        nb2 = neighbors[6];
    }
    else if (dir == 1) {
        nb1 = neighbors[0];
        nb2 = neighbors[7];
    }
    else if (dir == 2) {
        nb1 = neighbors[3];
        nb2 = neighbors[4];
    }
    else if (dir == 3) {
        nb1 = neighbors[2];
        nb2 = neighbors[5];
    }

    if (nb1 < value && value == nb2 || nb1 == value && value > nb2) {
        return LOW;
    }

    return (value < nb1 || value < nb2) ? LOW : value;
}


std::shared_ptr<uint8_t[]> Image::doubleThreshold(const uint16_t* img, uint16_t lower_threshold, uint16_t upper_threshold) {
    if (lower_threshold >= upper_threshold) {
        throw Exception("wrong thresholds");
    }

    std::shared_ptr<uint8_t[]> empty(new uint8_t[size]);
    for (int i = 0; i < size; i++) {
        if (img[i] < lower_threshold) {
            empty[i] = 0;
            continue;
        }
        if (img[i] > upper_threshold) {
            empty[i] = 2;
            continue;
        }
        empty[i] = 1;
    }
    return empty;
}


std::shared_ptr<uint8_t[]> Image::checkStrongIds(const uint8_t* db_threshold) {
    std::shared_ptr<uint8_t[]> empty(new uint8_t[size]);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (db_threshold[y*w+x] == 1 && checkNeighbors(db_threshold, x, y, w, h, 2) || db_threshold[y*w+x] == 2) {
                empty[y*w+x] = HIGH;
            }
            else {
                empty[y*w+x] = LOW;
            }
        }
    }

    return empty;
}


Image Image::Canny(uint16_t lower_threshold, uint16_t upper_threshold) {
    if (format != FORMAT_GRAY) {
        throw Exception("wrong format");
    }

    std::shared_ptr<uint8_t[]> gauss(gaussianBlur(FORMAT_GRAY));

    std::shared_ptr<uint8_t[]> sobel_x(sobel(gauss.get(), true));
    std::shared_ptr<uint8_t[]> sobel_y(sobel(gauss.get(), false));

    uint16_t grad_val[size];
    uint8_t grad_dir[size];
    double atan;
    for (int i = 0; i < size; i++) {
        grad_val[i] = std::sqrt(sobel_x[i] * sobel_x[i] + sobel_y[i] * sobel_y[i]);
        grad_dir[i] = getGradientDirection(sobel_x[i], sobel_y[i]);
    }

    std::shared_ptr<uint16_t[]> max(new uint16_t[size]);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            std::shared_ptr<uint16_t[]> neighbors = findNeighbors(grad_val, x, y, w, h);
            max[y*w+x] = suppressNonMax(neighbors.get(), grad_val[y*w+x], grad_dir[y*w+x]);
        }
    }

    std::shared_ptr<uint8_t[]> db_threshold = doubleThreshold(max.get(), lower_threshold, upper_threshold);

    std::shared_ptr<uint8_t[]> canny = checkStrongIds(db_threshold.get());

    return Image(canny, w, h, FORMAT_BIN);
}


Image Image::debug_canny() {
    if (format != FORMAT_GRAY) {
        throw Exception("wrong format");
    }

    std::shared_ptr<uint8_t[]> gauss(gaussianBlur(FORMAT_GRAY));

    std::shared_ptr<uint8_t[]> sobel_x(sobel(gauss.get(), true));
    std::shared_ptr<uint8_t[]> sobel_y(sobel(gauss.get(), false));

    uint16_t grad_val[size];
    uint8_t grad_dir[size];
    double atan;
    for (int i = 0; i < size; i++) {
        grad_val[i] = std::sqrt(sobel_x[i] * sobel_x[i] + sobel_y[i] * sobel_y[i]);
        grad_dir[i] = getGradientDirection(sobel_x[i], sobel_y[i]);
    }

    std::shared_ptr<uint16_t[]> max(new uint16_t[size]);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            std::shared_ptr<uint16_t[]> neighbors = findNeighbors(grad_val, x, y, w, h);
            max[y*w+x] = suppressNonMax(neighbors.get(), grad_val[y*w+x], grad_dir[y*w+x]);
        }
    }

    uint16_t lower_threshold = 10;
    uint16_t upper_threshold = 50;
    std::shared_ptr<uint8_t[]> db_threshold = doubleThreshold(max.get(), lower_threshold, upper_threshold);

    std::shared_ptr<uint8_t[]> canny = checkStrongIds(db_threshold.get());

    /*std::shared_ptr<uint8_t[]> check_gauss(new uint8_t[size*3]);
    for (int i = 0; i < size; i++) {
        check_gauss[3*i] = gauss[i];
        check_gauss[3*i+1] = gauss[i];
        check_gauss[3*i+2] = gauss[i];
    }*/

    /*std::shared_ptr<uint8_t[]> check_sobel_x(new uint8_t[size*3]);
    for (int i = 0; i < size; i++) {
        check_sobel_x[3*i] = sobel_x[i];
        check_sobel_x[3*i+1] = sobel_x[i];
        check_sobel_x[3*i+2] = sobel_x[i];
    }*/

    /*std::shared_ptr<uint8_t[]> check_sobel_y(new uint8_t[size*3]);
    for (int i = 0; i < size; i++) {
        check_sobel_y[3*i] = sobel_y[i];
        check_sobel_y[3*i+1] = sobel_y[i];
        check_sobel_y[3*i+2] = sobel_y[i];
    }*/

    /*std::shared_ptr<uint8_t[]> check_grad_val(new uint8_t[size*3]);
    memset(check_grad_val.get(), 0, size);
    for (int i = 0; i < size; i++) {
        if (grad_val[i] > 255) {//r
            check_grad_val[3*i] = 255;
            check_grad_val[3*i+1] = 255;
            check_grad_val[3*i+2] = 255;
        }
        else {
            check_grad_val[3*i] = grad_val[i];
            check_grad_val[3*i+1] = grad_val[i];
            check_grad_val[3*i+2] = grad_val[i];
        }
    }*/

    /*std::shared_ptr<uint8_t[]> check_grad_val_color(new uint8_t[size*3]);
    memset(check_grad_val_color.get(), 0, size);
    for (int i = 0; i < size; i++) {
        if (grad_val[i] > 255) {//r
            check_grad_val_color[i*3] = 255;
        }
        if (255 > grad_val[i] && grad_val[i] > 0) {//g
            check_grad_val_color[i*3+1] = 255;
        }
        if (grad_val[i] == 0) {//b
            check_grad_val_color[i*3+2] = 255;
        }
        if (grad_val[i] == 255) {//p
            check_grad_val_color[i*3] = 255;
            check_grad_val_color[i*3+2] = 255;
        }
    }*/

    /*std::shared_ptr<uint8_t[]> check_grad_dir(new uint8_t[size*3]);
    memset(check_grad_dir.get(), 0, size);
    for (int i = 0; i < size; i++) {
        if (grad_dir[i] == 0) {//r
            check_grad_dir[i*3] = 255;
        }
        if (grad_dir[i] == 1) {//g
            check_grad_dir[i*3+1] = 255;
        }
        if (grad_dir[i] == 2) {//b
            check_grad_dir[i*3+2] = 255;
        }
        if (grad_dir[i] == 3) {//p
            check_grad_dir[i*3] = 255;
            check_grad_dir[i*3+2] = 255;
        }
    }*/

    /*std::shared_ptr<uint8_t[]> check_max(new uint8_t[size*3]);
    memset(check_max.get(), 0, size);
    for (int i = 0; i < size; i++) {
        if (max[i] > 255) {
            check_max[i*3] = 255;
            check_max[i*3+1] = 255;
            check_max[i*3+2] = 255;
        }
        else {
            check_max[i*3] = max[i];
            check_max[i*3+1] = max[i];
            check_max[i*3+2] = max[i];
        }
    }*/

    /*std::shared_ptr<uint8_t[]> check_max_color(new uint8_t[size*3]);
    memset(check_max_color.get(), 0, size);
    for (int i = 0; i < size; i++) {
        if (max[i] > 255) {//r
            check_max_color[i*3] = 255;
        }
        if (255 > max[i] && max[i] > 0) {//g
            check_max_color[i*3+1] = 255;
        }
        if (max[i] == 0) {//b
            check_max_color[i*3+2] = 255;
        }
        if (max[i] == 255) {//p
            check_max_color[i*3] = 255;
            check_max_color[i*3+2] = 255;
        }
    }*/


    /*std::shared_ptr<uint8_t[]> check_threshold(new uint8_t[size*3]);
    memset(check_threshold.get(), 0, size);
    for (int i = 0; i < size; i++) {
        if (db_threshold[i] == 2) {//r
            check_threshold[i*3] = 255;
        }
        if (db_threshold[i] == 1) {//g
            check_threshold[i*3+1] = 255;
        }
        if (db_threshold[i] == 0) {//b
            check_threshold[i*3+2] = 255;
        }
    }*/


    /*std::shared_ptr<uint8_t[]> check_canny(new uint8_t[size*3]);
    for (int i = 0; i < size; i++) {
        check_canny[i*3] = canny[i];
        check_canny[i*3+1] = canny[i];
        check_canny[i*3+2] = canny[i];
    }*/

    return Image(canny, w, h, FORMAT_BIN);
}


Image Image::Flip(bool vertical) {
    if (format == FORMAT_RGB) {
        throw Exception("rgb is not supported");
    }

    std::shared_ptr<uint8_t[]> flip(new uint8_t[size]);
    uint8_t* values;

    if (format == FORMAT_GRAY) {
        values = gray.get();
    }
    if (format == FORMAT_BIN) {
        values = bin.get();
    }

    if (vertical) {
        for (int i = 0; i < h; i++) {
            uint8_t* src = values + sizeof(uint8_t) * (size - w * (1 + i));
            uint8_t* dst = flip.get() + sizeof(uint8_t) * w * i;
            std::memcpy(dst, src, sizeof(uint8_t) * w);
        }
        return Image(flip, w, h, format);
    }
    else {
        std::memcpy(flip.get(), values, sizeof(uint8_t) * size);
        for (int i = 0; i < h; i++) {
            std::reverse(flip.get() + sizeof(uint8_t) * w * i, flip.get() + sizeof(uint8_t) * w * (i + 1));
        }
        return Image(flip, w, h, format);
    }
}
