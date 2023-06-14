#ifndef DETECTION_H
#define DETECTION_H


#include <QMainWindow>
#include <QFile>


class Detection {
private:
    inline static QImage* img;

public:
    Detection();

    static void setImg(QImage* image);
    static QImage* getImg();
};


#endif // DETECTION_H
