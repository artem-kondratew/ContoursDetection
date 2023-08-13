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


void Image::createBin(uint8_t threshold_value) {
    if (gray == nullptr) {
        throw Exception("gray is nullptr");
    }

    bin = std::shared_ptr<uint8_t[]>(new uint8_t[size]);

    /*for (int pix = 0; pix < size; pix++) {
        bin[pix] = gray[pix] >= threshold ? HIGH : LOW;
    }*/

    __m128i reg_threshold = _mm_set1_epi8(threshold_value);
    __m128i reg_high = _mm_set1_epi8((uint8_t)HIGH);
    __m128i reg_low = _mm_set1_epi8((uint8_t)LOW);

    for (int pix = 0; pix < size; pix += 16) {
        __m128i reg_in = _mm_loadu_si128((__m128i*)(gray.get() + pix));

        __m128i reg_mask = _mm_cmpeq_epi8(reg_in, _mm_max_epu8(reg_in, reg_threshold));
        __m128i out_reg = _mm_blendv_epi8(reg_low, reg_high, reg_mask);

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


__m128i mul_int32x4_floatx4(__m128i reg_int32, __m128 reg_float) {
        __m128 reg_float_from_int32 = _mm_cvtepi32_ps(reg_int32);
    __m128 reg_mul = _mm_mul_ps(reg_float_from_int32, reg_float);
    __m128i reg_mul_to_int32 = _mm_cvtps_epi32(reg_mul);
    return reg_mul_to_int32;
}


std::shared_ptr<uint8_t[]> Image::convolution(const uint8_t* matrix, const float* kernel) {
    int kernel_size = 9;
    std::shared_ptr<uint8_t[]> empty(new uint8_t[size]);
    memset(empty.get(), 0, size);

    __m128 simd_kernel[kernel_size];
    for (int i = 0; i < kernel_size; i++) {
        simd_kernel[i] = _mm_set1_ps(kernel[i]);
    }

    __m128i submatrix[kernel_size];

    __m128i reg_high = _mm_set1_epi32(255);

    int main_w = w - 16;

    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < main_w; x += 4) {

            int pix_ptr = w * y + x;

            submatrix[0] = _mm_loadu_si128((__m128i*)(matrix + pix_ptr - w - 1));
            submatrix[1] = _mm_loadu_si128((__m128i*)(matrix + pix_ptr - w));
            submatrix[2] = _mm_loadu_si128((__m128i*)(matrix + pix_ptr - w + 1));

            submatrix[3] = _mm_loadu_si128((__m128i*)(matrix + pix_ptr - 1));
            submatrix[4] = _mm_loadu_si128((__m128i*)(matrix + pix_ptr));
            submatrix[5] = _mm_loadu_si128((__m128i*)(matrix + pix_ptr + 1));

            submatrix[6] = _mm_loadu_si128((__m128i*)(matrix + pix_ptr + w - 1));
            submatrix[7] = _mm_loadu_si128((__m128i*)(matrix + pix_ptr + w));
            submatrix[8] = _mm_loadu_si128((__m128i*)(matrix + pix_ptr + w + 1));

            __m128i reg_mul;
            __m128i reg_sum = _mm_set1_epi32(0);

            for (int i = 0; i < kernel_size; i++) {
                reg_mul = mul_int32x4_floatx4(_mm_cvtepu8_epi32(submatrix[i]), simd_kernel[i]);
                reg_sum = _mm_add_epi32(reg_sum, reg_mul);
            }

            __m128i mask_upper_threshold = _mm_cmpgt_epi32(reg_sum, reg_high);
            __m128i reg_upper_threshold = _mm_blendv_epi8(reg_sum, reg_high, mask_upper_threshold);

            __m128i reg16 = _mm_packus_epi32(reg_upper_threshold, reg_upper_threshold);
            __m128i reg8 = _mm_packus_epi16(reg16, reg16);

            _mm_storeu_si128((__m128i*)(empty.get() + pix_ptr), reg8);
        }

        uint8_t sub[kernel_size];
        for (int x = main_w; x < w - 1; x++) {
            int cell = 0;
            for (int i = y - 1; i < y + 2; i++) {
                for (int j = x - 1; j < x + 2; j++) {
                    sub[cell] = matrix[i * w + j];
                    cell++;
                }
            }
            float sum = 0;
            for (int i = 0; i < kernel_size; i++) {
                sum += (float)sub[i] * kernel[i];
            }
            empty[y * w + x] = sum > 255 ? 255 : sum;
        }
    }
    return empty;

    /*
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
            double sum = 0;
            for (int i = 0; i < kernel_size; i++) {
                sum += submatrix[i] * kernel[i];
            }
            int res = std::abs(sum);
            empty[y*w+x] = res > HIGH ? HIGH : res;
        }
    }
    return empty;
    */
}


Image Image::Convolution(float *kernel) {
    if (format == FORMAT_RGB) {
        throw Exception("rgb is not supported");
    }
    if (format == FORMAT_GRAY) {
        return Image(convolution(gray.get(), kernel), w, h, format);
    }
    if (format == FORMAT_BIN) {
        return Image(convolution(bin.get(), kernel), w, h, format);
    }
    throw Exception("wrong format input");
}


std::shared_ptr<uint8_t[]> Image::gaussianBlur(int format) {
    float matrix[] = {0.0625, 0.1250, 0.0625,
                        0.1250, 0.2500, 0.1250,
                        0.0625, 0.1250, 0.0625};

    if (format == FORMAT_RGB) {
        throw Exception("rgb is not supported");
    }
    if (format == FORMAT_GRAY) {
        return convolution(gray.get(), matrix);
    }
    if (format == FORMAT_BIN) {
        return convolution(bin.get(), matrix);
    }
    throw Exception("wrong format input");
}


Image Image::GaussianBlur(int format) {
    return Image(gaussianBlur(format), w, h, format);
}


std::shared_ptr<uint8_t[]> Image::add_uint8_arrays(const uint8_t* arr1, const uint8_t* arr2, int size) {
    std::shared_ptr<uint8_t[]> sum(new uint8_t[size]);

    for (int i = 0; i < size - 16; i += 16) {
        __m128i reg1 = _mm_loadu_si128((__m128i*)(arr1+i));
        __m128i reg2 = _mm_loadu_si128((__m128i*)(arr2+i));
        __m128i reg_sum = _mm_adds_epu8(reg1, reg2);
        _mm_storeu_si128((__m128i*)(sum.get() + i), reg_sum);
    }

    return sum;
}


std::shared_ptr<uint8_t[]> Image::sobel(uint8_t* values, bool vertical) {
    float matrix_x[] = {-1., 0., 1.,
                           -2., 0., 2.,
                           -1., 0., 1.};

    float matrix_x_rev[] = {1., 0., -1.,
                            2., 0., -2.,
                            1., 0., -1.};

    float matrix_y[] = {1., 2., 1.,
                        0., 0., 0.,
                        -1., -2., -1.};

    float matrix_y_rev[] = {-1., -2., -1.,
                        0., 0., 0.,
                        1., 2., 1.};

    if (vertical) {
        return add_uint8_arrays(convolution(values, matrix_x).get(), convolution(values, matrix_x_rev).get(), size);
    }
    return add_uint8_arrays(convolution(values, matrix_y).get(), convolution(values, matrix_y_rev).get(), size);
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


bool Image::checkEdges(int x, int y) {
    if (y < 0  || x < 0 || h <= y || w <= x) {
        return false;
    }
    return true;
}


std::shared_ptr<__m128i[]> Image::findNeighborsSimd(const uint16_t* grad_val, int x, int y) {
    std::shared_ptr<__m128i[]> neighbors(new __m128i[NEIGHBORS]);

    int pix_ptr = w * y + x;

    if (y == 0) {
        neighbors[0] = _mm_set1_epi16(0);
        neighbors[1] = _mm_set1_epi16(0);
        neighbors[2] = _mm_set1_epi16(0);
    }
    else {
        neighbors[0] = _mm_loadu_si128((__m128i*)(grad_val + pix_ptr - w - 1));
        neighbors[1] = _mm_loadu_si128((__m128i*)(grad_val + pix_ptr - w));
        neighbors[2] = _mm_loadu_si128((__m128i*)(grad_val + pix_ptr - w + 1));
    }

    if (x == 0) {
        neighbors[0] = _mm_set1_epi16(0);
        neighbors[3] = _mm_set1_epi16(0);
        neighbors[5] = _mm_set1_epi16(0);
    }
    else {
        neighbors[0] = _mm_loadu_si128((__m128i*)(grad_val + pix_ptr - w - 1));
        neighbors[3] = _mm_loadu_si128((__m128i*)(grad_val + pix_ptr - 1));
        neighbors[5] = _mm_loadu_si128((__m128i*)(grad_val + pix_ptr + w - 1));
    }

    if (y == h - 1) {
        neighbors[5] = _mm_set1_epi16(0);
        neighbors[6] = _mm_set1_epi16(0);
        neighbors[7] = _mm_set1_epi16(0);
    }
    else {
        neighbors[5] = _mm_loadu_si128((__m128i*)(grad_val + pix_ptr + w - 1));
        neighbors[6] = _mm_loadu_si128((__m128i*)(grad_val + pix_ptr + w));
        neighbors[7] = _mm_loadu_si128((__m128i*)(grad_val + pix_ptr + w + 1));
    }

    neighbors[2] = _mm_loadu_si128((__m128i*)(grad_val + pix_ptr - w + 1));
    neighbors[4] = _mm_loadu_si128((__m128i*)(grad_val + pix_ptr + 1));
    neighbors[7] = _mm_loadu_si128((__m128i*)(grad_val + pix_ptr + w + 1));

    return neighbors;
}


std::shared_ptr<uint16_t[]> Image::findNeighbors(const uint16_t* img, int x, int y) {
    std::shared_ptr<uint16_t[]> neighbors(new uint16_t[NEIGHBORS]);

    int cell = 0;
    for (int i = y - 1; i < y + 2; i++) {
        for (int j = x - 1; j < x + 2; j++) {
            if (i == y && j == x) {
                continue;
            }

            neighbors[cell] = checkEdges(j, i) ? img[i*w+j] : LOW;
            cell++;
        }
    }
    return neighbors;
}


bool Image::checkNeighbors(const uint8_t* img, int x, int y, uint8_t key) {
    for (int i = y - 1; i < y + 2; i++) {
        for (int j = x - 1; j < x + 2; j++) {
            if (i == y && j == x) {
                continue;
            }

            uint8_t pixel = checkEdges(j, i) ? img[i*w+j] : LOW;

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
            if (checkNeighbors(bin.get(), x, y, HIGH)) {
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
            if (checkNeighbors(bin.get(), x, y, HIGH)) {
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
            if (checkNeighbors(bin.get(), x, y, LOW)) {
                erode[y*w+x] = LOW;
            }
        }
    }
    return Image(erode, w, h, FORMAT_BIN);
}


bool Image::cmpltPixelNeighbors(const uint8_t* img, int x, int y, double k) {
    uint8_t pixel = int(k * img[y*w+x]);
    uint8_t nb;

    for (int i = y - 1; i < y + 2; i++) {
        for (int j = x - 1; j < x + 2; j++) {
            if (i == y && j == x) {
                continue;
            }

            nb = checkEdges(j, i) ? img[i*w+j] : LOW;
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
            if (cmpltPixelNeighbors(gray.get(), x, y, k)) {
                fd[y*w+x] = HIGH;
            }
        }
    }
    return Image(fd, w, h, FORMAT_BIN);
}


std::shared_ptr<uint16_t[]> Image::getGradientValues(const uint8_t* sobel_x, const uint8_t* sobel_y) {
    std::shared_ptr<uint16_t[]> grad_val(new uint16_t[size]);

    for (int i = 0; i < size; i += 4) {
        __m128i reg_sobel_x_int32 = _mm_cvtepu8_epi32(_mm_loadu_si128((__m128i*)(sobel_x + i)));
        __m128i reg_sobel_y_int32 = _mm_cvtepu8_epi32(_mm_loadu_si128((__m128i*)(sobel_y + i)));

        __m128 reg_sobel_x_float = _mm_cvtepi32_ps(reg_sobel_x_int32);
        __m128 reg_sobel_y_float = _mm_cvtepi32_ps(reg_sobel_y_int32);

        __m128 reg_squared_sobel_x_float = _mm_mul_ps(reg_sobel_x_float, reg_sobel_x_float);
        __m128 reg_squared_sobel_y_float = _mm_mul_ps(reg_sobel_y_float, reg_sobel_y_float);

        __m128 reg_grad_val_float = _mm_sqrt_ps(_mm_add_ps(reg_squared_sobel_x_float, reg_squared_sobel_y_float));
        __m128i reg_grad_val_int32 = _mm_cvtps_epi32(reg_grad_val_float);

        __m128i reg_grad_val_int16 = _mm_packus_epi32(reg_grad_val_int32, reg_grad_val_int32);

        _mm_storeu_si128((__m128i*)(grad_val.get() + i), reg_grad_val_int16);
    }

    return grad_val;
}


std::shared_ptr<uint16_t[]> Image::getGradientDirection(const uint8_t* sobel_x, const uint8_t* sobel_y) {
    std::shared_ptr<uint16_t[]> grad_dir(new uint16_t[size]);
    float rad[8];

    __m128 rad2deg = _mm_set1_ps(180 / M_PI);

    __m128i angle_m_67 = _mm_set1_epi32(-67);
    __m128i angle_m_23 = _mm_set1_epi32(-23);
    __m128i angle_p_23 = _mm_set1_epi32(+23);
    __m128i angle_p_67 = _mm_set1_epi32(+67);

    __m128i dir_1 = _mm_set1_epi32(1);
    __m128i dir_2 = _mm_set1_epi32(2);
    __m128i dir_3 = _mm_set1_epi32(3);

    for (int i = 0; i < size; i += 8) {
        for (int j = 0; j < 8; j++) {
            rad[j] = std::atan(sobel_y[i+j] / float(sobel_x[i+j]));
        }

        __m128 reg_rad_first = _mm_loadu_ps(rad);
        __m128 reg_rad_last = _mm_loadu_ps(rad + 4);

        __m128 angle_float_first = _mm_mul_ps(reg_rad_first, rad2deg);
        __m128 angle_float_last = _mm_mul_ps(reg_rad_last, rad2deg);

        __m128i angle_f = _mm_cvtps_epi32(angle_float_first);
        __m128i angle_l = _mm_cvtps_epi32(angle_float_last);

        __m128i dir_f = _mm_set1_epi32(0);
        __m128i dir_l = _mm_set1_epi32(0);

        __m128i mask_dir_1_f = _mm_andnot_si128(_mm_cmpgt_epi32(angle_m_67, angle_f), _mm_cmplt_epi32(angle_f, angle_m_23));
        __m128i mask_dir_2_f = _mm_andnot_si128(_mm_cmpgt_epi32(angle_m_23, angle_f), _mm_cmplt_epi32(angle_f, angle_p_23));
        __m128i mask_dir_3_f = _mm_andnot_si128(_mm_cmpgt_epi32(angle_p_23, angle_f), _mm_cmplt_epi32(angle_f, angle_p_67));

        __m128i mask_dir_1_l = _mm_andnot_si128(_mm_cmpgt_epi32(angle_m_67, angle_l), _mm_cmplt_epi32(angle_l, angle_m_23));
        __m128i mask_dir_2_l = _mm_andnot_si128(_mm_cmpgt_epi32(angle_m_23, angle_l), _mm_cmplt_epi32(angle_l, angle_p_23));
        __m128i mask_dir_3_l = _mm_andnot_si128(_mm_cmpgt_epi32(angle_p_23, angle_l), _mm_cmplt_epi32(angle_l, angle_p_67));

        dir_f = _mm_blendv_epi8(dir_f, dir_1, mask_dir_1_f);
        dir_f = _mm_blendv_epi8(dir_f, dir_2, mask_dir_2_f);
        dir_f = _mm_blendv_epi8(dir_f, dir_3, mask_dir_3_f);

        dir_l = _mm_blendv_epi8(dir_l, dir_1, mask_dir_1_l);
        dir_l = _mm_blendv_epi8(dir_l, dir_2, mask_dir_2_l);
        dir_l = _mm_blendv_epi8(dir_l, dir_3, mask_dir_3_l);

        __m128i dir_int16_f = _mm_packus_epi32(dir_f, dir_f);
        __m128i dir_int16_l = _mm_packus_epi32(dir_l, dir_l);

        _mm_storeu_si128((__m128i*)(grad_dir.get() + i), dir_int16_f);
        _mm_storeu_si128((__m128i*)(grad_dir.get() + i + 4), dir_int16_l);

        /*
        float rad = std::atan(sobel_y[i] / float(sobel_x[i]));
        int angle = rad * 180 / M_PI;

        if (-67 <= angle && angle < -23) {
            grad_dir[i] = 1;
            continue;
        }
        if (-23 <= angle && angle < 23) {
            grad_dir[i] = 2;
            continue;
        }
        if (23 <= angle && angle < 67) {
            grad_dir[i] = 3;
            continue;
        }
        grad_dir[i] = 0;*/
    }

    return grad_dir;
}


std::shared_ptr<uint16_t[]> Image::suppressNonMax(const uint16_t* grad_val, const uint16_t* grad_dir) {
    std::shared_ptr<uint16_t[]> max(new uint16_t[size]);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w - 8; x += 8) {
            int idx = w * y + x;
            std::shared_ptr<__m128i[]> neighbors = findNeighborsSimd(grad_val, x, y);

            __m128i nb1 = _mm_set1_epi16(LOW);
            __m128i nb2 = _mm_set1_epi16(LOW);

            __m128i values = _mm_loadu_si128((__m128i*)(grad_val + idx));
            __m128i dirs = _mm_loadu_si128((__m128i*)(grad_dir + idx));

            __m128i dir_0 = _mm_set1_epi16(0);
            __m128i dir_1 = _mm_set1_epi16(1);
            __m128i dir_2 = _mm_set1_epi16(2);
            __m128i dir_3 = _mm_set1_epi16(3);

            __m128i low = _mm_set1_epi16(LOW);

            __m128i dir_0_mask = _mm_cmpeq_epi16(dirs, dir_0);
            __m128i dir_1_mask = _mm_cmpeq_epi16(dirs, dir_1);
            __m128i dir_2_mask = _mm_cmpeq_epi16(dirs, dir_2);
            __m128i dir_3_mask = _mm_cmpeq_epi16(dirs, dir_3);

            nb1 = _mm_blendv_epi8(nb1, neighbors[1], dir_0_mask);
            nb2 = _mm_blendv_epi8(nb2, neighbors[6], dir_0_mask);

            nb1 = _mm_blendv_epi8(nb1, neighbors[0], dir_1_mask);
            nb2 = _mm_blendv_epi8(nb2, neighbors[7], dir_1_mask);

            nb1 = _mm_blendv_epi8(nb1, neighbors[3], dir_2_mask);
            nb2 = _mm_blendv_epi8(nb2, neighbors[4], dir_2_mask);

            nb1 = _mm_blendv_epi8(nb1, neighbors[2], dir_3_mask);
            nb2 = _mm_blendv_epi8(nb2, neighbors[5], dir_3_mask);

            __m128i cmp = _mm_or_si128(_mm_cmpgt_epi16(nb1, values), _mm_cmpgt_epi16(nb2, values));
            __m128i reg_max = _mm_blendv_epi8(values, low, cmp);

            _mm_storeu_si128((__m128i*)(max.get() + idx), reg_max);
        }

        for (int x = w - 8; x < w; x++) {
            std::shared_ptr<uint16_t[]> neighbors = findNeighbors(grad_val, x, y);

            uint16_t nb1 = LOW;
            uint16_t nb2 = LOW;

            uint16_t value = grad_val[y*w+x];
            uint16_t dir = grad_dir[y*w+x];

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
                max[y*w+x] = LOW;
            }

            max[y*w+x] = value < nb1 || value < nb2 ? LOW : value;
        }
    }

    return max;
}


std::shared_ptr<uint8_t[]> Image::doubleThreshold(const uint16_t* img, uint16_t lower_threshold, uint16_t upper_threshold) {
    if (lower_threshold >= upper_threshold) {
        throw Exception("wrong thresholds");
    }

    std::shared_ptr<uint8_t[]> empty(new uint8_t[size]);

    __m128i reg_lower_threshold = _mm_set1_epi16((int16_t)lower_threshold);
    __m128i reg_upper_threshold = _mm_set1_epi16((int16_t)upper_threshold);

    __m128i reg_value_0 = _mm_set1_epi16(0);
    __m128i reg_value_1 = _mm_set1_epi16(1);

    for (int i = 0; i < size; i += 8) {
        __m128i reg_image_values = _mm_loadu_si128((__m128i*)(img + i));

        __m128i mask_lower_threshold = _mm_cmpgt_epi16(reg_lower_threshold, reg_image_values);
        __m128i reg_lower_threshold_values = _mm_blendv_epi8(reg_value_1, reg_value_0, mask_lower_threshold);

        __m128i mask_upper_threshold = _mm_cmpgt_epi16(reg_image_values, reg_upper_threshold);
        __m128i reg_upper_threshold_values = _mm_blendv_epi8(reg_value_0, reg_value_1, mask_upper_threshold);

        __m128i reg_db_threshold_values_int16 = _mm_add_epi16(reg_lower_threshold_values, reg_upper_threshold_values);
        __m128i reg_db_threshold_values_int8 = _mm_packus_epi16(reg_db_threshold_values_int16, reg_db_threshold_values_int16);

        _mm_storeu_si128((__m128i*)(empty.get() + i), reg_db_threshold_values_int8);
    }

    /*for (int i = 0; i < size; i++) {
        if (img[i] < lower_threshold) {
            empty[i] = 0;
            continue;
        }
        if (img[i] > upper_threshold) {
            empty[i] = 2;
            continue;
        }
        empty[i] = 1;
    }*/

    return empty;
}


std::shared_ptr<uint8_t[]> Image::checkStrongIds(const uint8_t* db_threshold) {
    std::shared_ptr<uint8_t[]> empty(new uint8_t[size]);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (db_threshold[y*w+x] == 1 && checkNeighbors(db_threshold, x, y, 2) || db_threshold[y*w+x] == 2) {
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


    auto start_timer_gauss = std::chrono::system_clock::now();

    std::shared_ptr<uint8_t[]> gauss(gaussianBlur(FORMAT_GRAY));

    auto stop_timer_gauss = std::chrono::system_clock::now();


    auto start_timer_sobel = std::chrono::system_clock::now();

    std::shared_ptr<uint8_t[]> sobel_x(sobel(gauss.get(), true));
    std::shared_ptr<uint8_t[]> sobel_y(sobel(gauss.get(), false));

    auto stop_timer_sobel = std::chrono::system_clock::now();


    auto start_timer_grad = std::chrono::system_clock::now();

    std::shared_ptr<uint16_t[]> grad_val = getGradientValues(sobel_x.get(), sobel_y.get());
    std::shared_ptr<uint16_t[]> grad_dir = getGradientDirection(sobel_x.get(), sobel_y.get());

    auto stop_timer_grad = std::chrono::system_clock::now();


    auto start_timer_max = std::chrono::system_clock::now();

    std::shared_ptr<uint16_t[]> max = suppressNonMax(grad_val.get(), grad_dir.get());

    auto stop_timer_max = std::chrono::system_clock::now();


    auto start_timer_thresh = std::chrono::system_clock::now();

    std::shared_ptr<uint8_t[]> db_threshold = doubleThreshold(max.get(), lower_threshold, upper_threshold);

    std::shared_ptr<uint8_t[]> canny = checkStrongIds(db_threshold.get());

    auto stop_timer_thresh = std::chrono::system_clock::now();


    auto time_gauss = std::chrono::duration_cast<std::chrono::milliseconds>(stop_timer_gauss - start_timer_gauss).count();
    auto time_sobel = std::chrono::duration_cast<std::chrono::milliseconds>(stop_timer_sobel - start_timer_sobel).count();
    auto time_grad = std::chrono::duration_cast<std::chrono::milliseconds>(stop_timer_grad - start_timer_grad).count();
    auto time_max = std::chrono::duration_cast<std::chrono::milliseconds>(stop_timer_max - start_timer_max).count();
    auto time_thresh = std::chrono::duration_cast<std::chrono::milliseconds>(stop_timer_thresh - start_timer_thresh).count();

    std::string tg = std::to_string(time_gauss);
    std::string ts = std::to_string(time_sobel);
    std::string tgr = std::to_string(time_grad);
    std::string tm = std::to_string(time_max);
    std::string tt = std::to_string(time_thresh);

    alpha = tg + " " + ts + " " + tgr + " " + tm + " " + tt;

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
