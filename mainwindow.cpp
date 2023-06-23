#include "mainwindow.h"


QImage qimg;
Image image, gray, bin;


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
    ui->Sobelx->hide();

    ui->imgPathLineEdit->setText("/home/user/Pictures/aruco.jpg");

    DoubleSpinBoxArray[0] = ui->convDoubleSpinBox_00; DoubleSpinBoxArray[1] = ui->convDoubleSpinBox_01; DoubleSpinBoxArray[2] = ui->convDoubleSpinBox_02;
    DoubleSpinBoxArray[3] = ui->convDoubleSpinBox_10; DoubleSpinBoxArray[4] = ui->convDoubleSpinBox_11; DoubleSpinBoxArray[5] = ui->convDoubleSpinBox_12;
    DoubleSpinBoxArray[6] = ui->convDoubleSpinBox_20; DoubleSpinBoxArray[7] = ui->convDoubleSpinBox_21; DoubleSpinBoxArray[8] = ui->convDoubleSpinBox_22;

    ui->convDoubleSpinBox_00->setValue(0); ui->convDoubleSpinBox_01->setValue(0); ui->convDoubleSpinBox_02->setValue(0);
    ui->convDoubleSpinBox_10->setValue(0); ui->convDoubleSpinBox_11->setValue(1); ui->convDoubleSpinBox_12->setValue(0);
    ui->convDoubleSpinBox_20->setValue(0); ui->convDoubleSpinBox_21->setValue(0); ui->convDoubleSpinBox_22->setValue(0);

    ui->convCheckBox->hide();
    ui->convPushButton->hide();
    ui->convDoubleSpinBox_N->hide();
    for (int i = 0; i < KERNEL_SIZE; i++) {
        DoubleSpinBoxArray[i]->hide();
    }
}


MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::saveQImage(QImage qimage) {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save as..."), "img.jpg", QString());
    if (fileName.isEmpty() || fileName.isNull())
        return;
    QPixmap::fromImage(qimage).save(fileName);
}


void MainWindow::locateImage(Image* left_img, int left_format, Image* right_img, int right_format) {
    if (left_img == nullptr || right_img == nullptr) {
        return;
    }

    QImage qimg = left_img->Image2QImage(left_format);
    QImage qgray = right_img->Image2QImage(right_format);

    QPixmap img_pixmap = QPixmap::fromImage(qimg);
    QPixmap gray_pixmap = QPixmap::fromImage(qgray);

    QRect geometry = this->geometry();

    if (left_img->width() > geometry.width() / 2 || left_img->height() > geometry.height()) {
        img_pixmap = img_pixmap.scaled(geometry.width() / 2, geometry.height(), Qt::KeepAspectRatio);
        gray_pixmap = gray_pixmap.scaled(geometry.width() / 2, geometry.height(), Qt::KeepAspectRatio);
        ui->rgbLabel->setGeometry(0, 0, geometry.width() / 2, geometry.height());
        ui->grayLabel->setGeometry(geometry.width() / 2, 0, geometry.width() / 2, geometry.height());
    }
    else {
        int x = (geometry.width() - left_img->width() * 2) / 3;
        int y = (geometry.height() - left_img->height()) / 2;
        ui->rgbLabel->setGeometry(x, y, left_img->width(), left_img->height());
        ui->grayLabel->setGeometry(left_img->width() + 2 * x, y, left_img->width(), left_img->height());
    }

    ui->rgbLabel->setPixmap(img_pixmap);
    ui->grayLabel->setPixmap(gray_pixmap);
}


void MainWindow::resizeEvent(QResizeEvent *event) {
    centerWidgets();
    //locateImage(&image, , &gray);
}


void MainWindow::on_imgUploadButton_clicked() {
    QString path = ui->imgPathLineEdit->text();
    qimg = QImage(path).convertToFormat(QImage::Format_RGB888);

    if (qimg.isNull()) {
        ui->imgOpenErrorLabel->show();
        return;
    }

    image = Image(&qimg);
    gray = image.Gray();
    bin = image.Bin();

    Image sobel = image.sobel(true);

    Image cont = bin.externalContouring();
    Image dilate = cont.dilate();
    Image erode = dilate.erode();

    Image fd = gray.fd().dilate();

    locateImage(&gray, FORMAT_GRAY, &fd, FORMAT_BIN);

    ui->imgOpenErrorLabel->hide();
    ui->imgPathLineEdit->hide();
    ui->imgUploadButton->hide();
    ui->imgPromtLabel->hide();

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
        ui->Sobelx->show();
        for (int i = 0; i < KERNEL_SIZE; i++) {
            DoubleSpinBoxArray[i]->show();
            ui->convDoubleSpinBox_N->show();
        }
    }
    else {
        ui->convCheckBox->hide();
        ui->convPushButton->hide();
        ui->Sobelx->hide();
        for (int i = 0; i < KERNEL_SIZE; i++) {
            DoubleSpinBoxArray[i]->hide();
            ui->convDoubleSpinBox_N->hide();
        }
    }
}


Image MainWindow::useConv() {
    if (ui->Sobelx->isChecked()) {
        return gray.sobel(true  );
    }

    double kernel[KERNEL_SIZE];
    for (int i = 0; i < KERNEL_SIZE; i++) {
        kernel[i] = DoubleSpinBoxArray[i]->value();
    }

    double k = ui->convDoubleSpinBox_N->value();

    return gray.conv(kernel, k);
}


void MainWindow::on_convPushButton_clicked() {
    if (!ui->convCheckBox->isChecked()) {
        return;
    }

    Image conv = useConv();

    locateImage(&gray, FORMAT_GRAY, &conv, FORMAT_GRAY);

    //saveQImage(conv_img.Image2QImage());
}
