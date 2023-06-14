#include "mainwindow.h"
#include "./ui_mainwindow.h"


Image* img_ptr;


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
}


MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::resizeEvent(QResizeEvent *event) {
    centerWidgets();
}


void MainWindow::locateImage(Image img) {
    QRect geometry = this->geometry();

    QImage gray = img.Gray2QImage();

    QPixmap img_pixmap = QPixmap::fromImage(img.Image2QImage());
    QPixmap gray_pixmap = QPixmap::fromImage(gray);

    if (img.width() > geometry.width() / 2 || img.height() > geometry.height()) {
        img_pixmap = img_pixmap.scaled(geometry.width() / 2, geometry.height(), Qt::KeepAspectRatio);
        gray_pixmap = gray_pixmap.scaled(geometry.width() / 2, geometry.height(), Qt::KeepAspectRatio);
        ui->rgbLabel->setGeometry(0, 0, geometry.width() / 2, geometry.height());
        ui->grayLabel->setGeometry(geometry.width() / 2, 0, geometry.width() / 2, geometry.height());
    }
    else {
        int x = (geometry.width() - img.width() * 2) / 3;
        int y = (geometry.height() - img.height()) / 2;
        ui->rgbLabel->setGeometry(x, y, img.width(), img.height());
        ui->grayLabel->setGeometry(img.width() + 2 * x, y, img.width(), img.height());
    }

    ui->rgbLabel->setPixmap(img_pixmap);
    ui->grayLabel->setPixmap(gray_pixmap);
}


void MainWindow::on_imgUploadButton_clicked() {
    QString path = ui->imgPathLineEdit->text();
    QImage qimg(path);

    if (qimg.isNull()) {
        ui->imgOpenErrorLabel->show();
        return;
    }

    Image img(&qimg);
    img_ptr = &img;

    ui->imgOpenErrorLabel->hide();
    ui->imgPathLineEdit->hide();
    ui->imgUploadButton->hide();
    ui->imgPromtLabel->hide();

    locateImage(img);
    ui->rgbLabel->show();
    ui->grayLabel->show();
}


void MainWindow::on_actionClose_triggered() {
    img_ptr = nullptr;

    ui->imgOpenErrorLabel->hide();
    ui->imgPathLineEdit->show();
    ui->imgUploadButton->show();
    ui->imgPromtLabel->show();
    ui->rgbLabel->hide();
    ui->grayLabel->hide();
}
