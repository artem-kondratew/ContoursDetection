#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>

Image img, gray;


void MainWindow::centerWidgets() {
    QRect geometry = this->geometry();
    int x;

    // imgPromtLabel
    x = (geometry.width() - ui->imgPromtLabel->width()) / 2;
    ui->imgPromtLabel->setGeometry(x, ui->imgPromtLabel->y(), ui->imgPromtLabel->width(), ui->imgPromtLabel->height());

    // imgPathLineEdit
    x = (geometry.width() - ui->imgPathLineEdit->width()) / 2;
    ui->imgPathLineEdit->setGeometry(x, ui->imgPathLineEdit->y(), ui->imgPathLineEdit->width(), ui->imgPathLineEdit->height());


    // imgOpenErrorLabel
    x = (geometry.width() - ui->imgOpenErrorLabel->width()) / 2;
    ui->imgOpenErrorLabel->setGeometry(x, ui->imgOpenErrorLabel->y(), ui->imgOpenErrorLabel->width(),
                                       ui->imgOpenErrorLabel->height());

    // imgUploadButton
    x = (geometry.width() - ui->imgUploadButton->width()) / 2;
    ui->imgUploadButton->setGeometry(x, ui->imgUploadButton->y(), ui->imgUploadButton->width(), ui->imgUploadButton->height());

    // rgbLabel & grayLabel
    ui->rgbLabel->setGeometry(0, 0, geometry.width() / 2, geometry.height());
    ui->grayLabel->setGeometry(geometry.width() / 2, 0, geometry.width() / 2, geometry.height());
}


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setWindowTitle("Contours detection");

    centerWidgets();

    ui->imgOpenErrorLabel->hide();
    ui->rgbLabel->hide();
    ui->grayLabel->hide();

    ui->imgPathLineEdit->setText("/home/user/Pictures/img.jpg");

    DoubleSpinBoxArray[0] = ui->convDoubleSpinBox_00; DoubleSpinBoxArray[1] = ui->convDoubleSpinBox_01; DoubleSpinBoxArray[2] = ui->convDoubleSpinBox_02;
    DoubleSpinBoxArray[3] = ui->convDoubleSpinBox_10; DoubleSpinBoxArray[4] = ui->convDoubleSpinBox_11; DoubleSpinBoxArray[5] = ui->convDoubleSpinBox_12;
    DoubleSpinBoxArray[6] = ui->convDoubleSpinBox_20; DoubleSpinBoxArray[7] = ui->convDoubleSpinBox_21; DoubleSpinBoxArray[8] = ui->convDoubleSpinBox_22;

    ui->convDoubleSpinBox_00->setValue(1); ui->convDoubleSpinBox_01->setValue(1); ui->convDoubleSpinBox_02->setValue(1);
    ui->convDoubleSpinBox_10->setValue(1); ui->convDoubleSpinBox_11->setValue(1); ui->convDoubleSpinBox_12->setValue(1);
    ui->convDoubleSpinBox_20->setValue(1); ui->convDoubleSpinBox_21->setValue(1); ui->convDoubleSpinBox_22->setValue(1);

    ui->convCheckBox->hide();
    ui->convPushButton->hide();
    for (int i = 0; i < KERNEL_SIZE; i++) {
        DoubleSpinBoxArray[i]->hide();
    }
}


MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::resizeEvent(QResizeEvent *event) {
    centerWidgets();
    locateImage(&img, &gray);
}


void MainWindow::locateImage(Image* img, Image* gray) {
    if (img == nullptr || gray == nullptr) {
        return;
    }

    QImage qgray = gray->Image2QImage();
    QImage qimg = img->Image2QImage();

    QPixmap img_pixmap = QPixmap::fromImage(qimg);
    QPixmap gray_pixmap = QPixmap::fromImage(qgray);

    QRect geometry = this->geometry();

    if (img->width() > geometry.width() / 2 || img->height() > geometry.height()) {
        img_pixmap = img_pixmap.scaled(geometry.width() / 2, geometry.height(), Qt::KeepAspectRatio);
        gray_pixmap = gray_pixmap.scaled(geometry.width() / 2, geometry.height(), Qt::KeepAspectRatio);
        ui->rgbLabel->setGeometry(0, 0, geometry.width() / 2, geometry.height());
        ui->grayLabel->setGeometry(geometry.width() / 2, 0, geometry.width() / 2, geometry.height());
    }
    else {
        int x = (geometry.width() - img->width() * 2) / 3;
        int y = (geometry.height() - img->height()) / 2;
        ui->rgbLabel->setGeometry(x, y, img->width(), img->height());
        ui->grayLabel->setGeometry(img->width() + 2 * x, y, img->width(), img->height());
    }

    ui->rgbLabel->setPixmap(img_pixmap);
    ui->grayLabel->setPixmap(gray_pixmap);
}


void MainWindow::on_imgUploadButton_clicked() {
    QString path = ui->imgPathLineEdit->text();
    QImage qimg = QImage(path).convertToFormat(QImage::Format_RGB888);

    if (qimg.isNull()) {
        ui->imgOpenErrorLabel->show();
        return;
    }

    img = Image(&qimg);
    gray = img.Gray();

    ui->imgOpenErrorLabel->hide();
    ui->imgPathLineEdit->hide();
    ui->imgUploadButton->hide();
    ui->imgPromtLabel->hide();

    locateImage(&img, &gray);
    ui->rgbLabel->show();
    ui->grayLabel->show();
}


void MainWindow::on_actionClose_triggered() {
    ui->imgOpenErrorLabel->hide();
    ui->imgPathLineEdit->show();
    ui->imgUploadButton->show();
    ui->imgPromtLabel->show();
    ui->rgbLabel->hide();
    ui->grayLabel->hide();
}


void MainWindow::on_actionShow_triggered(bool checked) {
    if (checked) {
        ui->convCheckBox->show();
        ui->convPushButton->show();
        for (int i = 0; i < KERNEL_SIZE; i++) {
            DoubleSpinBoxArray[i]->show();
        }
    }
    else {
        ui->convCheckBox->hide();
        ui->convPushButton->hide();
        for (int i = 0; i < KERNEL_SIZE; i++) {
            DoubleSpinBoxArray[i]->hide();
        }
    }
}

void MainWindow::on_convPushButton_clicked() {
    if (!ui->convCheckBox->isChecked()) {
        return;
    }
    double array[KERNEL_SIZE];
    for (int i = 0; i < KERNEL_SIZE; i++) {
        array[i] = DoubleSpinBoxArray[i]->value();
    }
    Image conv_img = gray.conv(array);
    locateImage(&img, &conv_img);
}
