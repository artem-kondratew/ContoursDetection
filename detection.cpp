#include "detection.h"


Detection::Detection() {

}


void Detection::setImg(QImage* image) {
    img = image;
}


QImage* Detection::getImg() {
    return img;
}
