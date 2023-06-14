#include "image.h"


Pixel::Pixel(int red, int green, int blue) {
    r = red;
    g = green;
    b = blue;
}


int** Image::createDoubleArray(int h, int w) {
    int** array = new int*[h];
    for (int y = 0; y < h; y++) {
        array[y] = new int[w];
    }
    return array;
}


void Image::createGray() {
    gray = createDoubleArray(h, w);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            gray[y][x] = int(GRAY_R * r[y][x] + GRAY_G * g[y][x] + GRAY_B*b[y][x]);
        }
    }
}


Image::Image(QImage* img) {
    w = img->width();
    h = img->height();
    r = createDoubleArray(h, w);
    g = createDoubleArray(h, w);
    b = createDoubleArray(h, w);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            QRgb rgb = img->pixel(x, y);
            r[y][x] = qRed(rgb);
            g[y][x] = qGreen(rgb);
            b[y][x] = qBlue(rgb);
        }
    }

    createGray();
}


Pixel Image::rgb_pixel(int y, int x) {
    return Pixel(r[y][x], g[y][x], b[y][x]);
}


QImage Image::Image2QImage() {
    QImage img(w, h, QImage::Format_RGB888);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            img.setPixel(x, y, qRgb(r[y][x], g[y][x], b[y][x]));
        }
    }
    return img;
}


QImage Image::Gray2QImage() {
    QImage img(w, h, QImage::Format_RGB888);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            img.setPixel(x, y, qRgb(gray[y][x], gray[y][x], gray[y][x]));
        }
    }
    return img;
}


int Image::width() {
    return w;
}


int Image::height() {
    return h;
}
