#include "mainwindow.h"


//QMediaPlayer *mediaPlayer;
//QVideoWidget *videoWidget;


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
    ui->originalLabel->setGeometry(0, 0, geometry.width() / 2, geometry.height());
    ui->convertedLabel->setGeometry(geometry.width() / 2, 0, geometry.width() / 2, geometry.height());
}


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setWindowTitle("Contours detection");

    centerWidgets();

    ui->imgOpenErrorLabel->hide();
    ui->originalLabel->hide();
    ui->convertedLabel->hide();

    ui->imgPathLineEdit->setText("/home/user/Pictures/sage.jpg");

    DoubleSpinBoxArray[0] = ui->convDoubleSpinBox_00; DoubleSpinBoxArray[1] = ui->convDoubleSpinBox_01; DoubleSpinBoxArray[2] = ui->convDoubleSpinBox_02;
    DoubleSpinBoxArray[3] = ui->convDoubleSpinBox_10; DoubleSpinBoxArray[4] = ui->convDoubleSpinBox_11; DoubleSpinBoxArray[5] = ui->convDoubleSpinBox_12;
    DoubleSpinBoxArray[6] = ui->convDoubleSpinBox_20; DoubleSpinBoxArray[7] = ui->convDoubleSpinBox_21; DoubleSpinBoxArray[8] = ui->convDoubleSpinBox_22;

    ui->convDoubleSpinBox_00->setValue(0); ui->convDoubleSpinBox_01->setValue(0); ui->convDoubleSpinBox_02->setValue(0);
    ui->convDoubleSpinBox_10->setValue(0); ui->convDoubleSpinBox_11->setValue(1); ui->convDoubleSpinBox_12->setValue(0);
    ui->convDoubleSpinBox_20->setValue(0); ui->convDoubleSpinBox_21->setValue(0); ui->convDoubleSpinBox_22->setValue(0);

    ui->convDoubleSpinBox_k->setValue(1);

    ui->convCheckBox->hide();
    ui->convPushButton->hide();
    ui->convDoubleSpinBox_k->hide();
    for (int i = 0; i < KERNEL_SIZE; i++) {
        DoubleSpinBoxArray[i]->hide();
    }

    ui->convDoubleSpinBox_00->hide(); ui->convDoubleSpinBox_01->hide(); ui->convDoubleSpinBox_02->hide();
    ui->convDoubleSpinBox_10->hide(); ui->convDoubleSpinBox_11->hide(); ui->convDoubleSpinBox_12->hide();
    ui->convDoubleSpinBox_20->hide(); ui->convDoubleSpinBox_21->hide(); ui->convDoubleSpinBox_22->hide();



    /*videoWidget = new QVideoWidget(ui->centralwidget);
    videoWidget->setObjectName(QString::fromUtf8("videoWidget"));
    videoWidget->setGeometry((1920-640)/2, (1080-480)/2, 640, 480);
    videoWidget->show();

    QCamera* camera = new QCamera(this);
    camera->setViewfinder(videoWidget);
    camera->start();

    //mediaPlayer = new QMediaPlayer();
    //mediaPlayer->setVideoOutput(videoWidget);
    //mediaPlayer->setMedia(QUrl::fromLocalFile("/home/user/Videos/video.mp4"));
    //mediaPlayer->setMedia(QUrl::fromLocalFile("/home/user/Videos/video.mp4"));

    //mediaPlayer->play();*/
}


MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::saveQImage(QImage qimage) {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save as..."), "img.jpg", QString());
    if (fileName.isEmpty() || fileName.isNull()) {
        return;
    }
    qimage.save(fileName, "JPG");
}


void MainWindow::locateOriginal() {
    if (original.isNull()) {
        return;
    }

    QRect geometry = this->geometry();

    QImage qimg = original.Image2QImage(original.Format());
    QPixmap pixmap = QPixmap::fromImage(qimg);
    pixmap = pixmap.scaled(geometry.width(), geometry.height(), Qt::KeepAspectRatio);

    int x = (geometry.width() - pixmap.width()) / 2;
    int y = (geometry.height() - pixmap.height()) / 2;

    ui->originalLabel->setGeometry(x, y, geometry.width(), geometry.height());
    ui->originalLabel->setPixmap(pixmap);
}


void MainWindow::locateConverted() {
    if (converted.isNull()) {
        return;
    }

    QRect geometry = this->geometry();

    QImage qimg = converted.Image2QImage(converted.Format());
    QPixmap pixmap = QPixmap::fromImage(qimg);
    pixmap = pixmap.scaled(geometry.width(), geometry.height(), Qt::KeepAspectRatio);

    int x = (geometry.width() - pixmap.width()) / 2;
    int y = (geometry.height() - pixmap.height()) / 2;

    ui->convertedLabel->setGeometry(x, y, pixmap.width(), pixmap.height());
    ui->convertedLabel->setPixmap(pixmap);
}


void MainWindow::resizeEvent(QResizeEvent *event) {
    centerWidgets();
    //locateImage(&image, , &gray);
}


void MainWindow::on_imgUploadButton_clicked() {
    QString path = ui->imgPathLineEdit->text();
    std::shared_ptr<QImage> qimg = std::make_shared<QImage>(QImage(path).convertToFormat(QImage::Format_RGB888));

    if (qimg->isNull()) {
        ui->imgOpenErrorLabel->show();
        return;
    }

    original = Image(qimg);
    locateOriginal();
    /*Image gray = original.Gray();
    Image bin = gray.Bin();
    Image cont = bin.ExternalContouring();

    auto start_time = std::chrono::steady_clock::now();

    auto end_time = std::chrono::steady_clock::now();
    //auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    int elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    ui->timeBox->setValue(elapsed_time);*/

    ui->imgOpenErrorLabel->hide();
    ui->imgPathLineEdit->hide();
    ui->imgUploadButton->hide();
    ui->imgPromtLabel->hide();

    ui->originalLabel->show();
}


void MainWindow::on_actionClose_triggered() {
    ui->imgOpenErrorLabel->hide();
    ui->imgPathLineEdit->show();
    ui->imgUploadButton->show();
    ui->imgPromtLabel->show();
    ui->originalLabel->hide();
    ui->convertedLabel->hide();
    original = Image();
    converted = Image();
}


void MainWindow::on_actionShow_triggered(bool checked) {
    if (checked) {
        ui->convCheckBox->show();
        ui->convPushButton->show();
        for (int i = 0; i < KERNEL_SIZE; i++) {
            DoubleSpinBoxArray[i]->show();
            ui->convDoubleSpinBox_k->show();
        }
    }
    else {
        ui->convCheckBox->hide();
        ui->convPushButton->hide();
        for (int i = 0; i < KERNEL_SIZE; i++) {
            DoubleSpinBoxArray[i]->hide();
            ui->convDoubleSpinBox_k->hide();
        }
    }
}


void MainWindow::on_convPushButton_clicked() {
    if (!ui->convCheckBox->isChecked()) {
        return;
    }
    if (original.Format() != FORMAT_GRAY && original.Format() != FORMAT_BIN) {
        return;
    }

    double k = ui->convDoubleSpinBox_k->value();
    double kernel[KERNEL_SIZE];

    for (int i = 0; i < KERNEL_SIZE; i++) {
        kernel[i] = DoubleSpinBoxArray[i]->value();
    }

    converted = original.Convolution(kernel, k);
    locateConverted();
    on_actionShow_converted_triggered();
}


void MainWindow::on_actionShow_original_triggered() {
    ui->originalLabel->show();
    ui->convertedLabel->hide();
}


void MainWindow::on_actionShow_converted_triggered() {
    ui->originalLabel->hide();
    ui->convertedLabel->show();
}


void MainWindow::on_actionSet_as_original_triggered() {
    original = converted;
    converted = Image();
    locateOriginal();
}


void MainWindow::on_actionGray_triggered() {
    if (original.Format() != FORMAT_RGB) {
        return;
    }
    converted = original.Gray();
    locateConverted();
    on_actionShow_converted_triggered();
}


void MainWindow::on_actionBin_triggered() {
    if (original.Format() != FORMAT_GRAY) {
        return;
    }
    converted = original.Bin();
    locateConverted();
    on_actionShow_converted_triggered();
}


void MainWindow::on_actionGauss_triggered() {
    if (original.Format() != FORMAT_GRAY && original.Format() != FORMAT_BIN) {
        return;
    }
    converted = original.GaussianBlur(original.Format());
    locateConverted();
    on_actionShow_converted_triggered();
}


void MainWindow::on_actionSobel_x_triggered() {
    if (original.Format() != FORMAT_GRAY && original.Format() != FORMAT_BIN) {
        return;
    }
    converted = original.Sobel(true, original.Format());
    locateConverted();
    on_actionShow_converted_triggered();
}

void MainWindow::on_actionSobel_y_triggered() {
    if (original.Format() != FORMAT_GRAY && original.Format() != FORMAT_BIN) {
        return;
    }
    converted = original.Sobel(false, original.Format());
    locateConverted();
    on_actionShow_converted_triggered();
}


void MainWindow::on_actionExternal_Contouring_triggered() {
    if (original.Format() != FORMAT_BIN) {
        return;
    }
    converted = original.ExternalContouring();
    locateConverted();
    on_actionShow_converted_triggered();
}


void MainWindow::on_actionDilate_triggered() {
    if (original.Format() != FORMAT_BIN) {
        return;
    }
    converted = original.Dilate();
    locateConverted();
    on_actionShow_converted_triggered();
}


void MainWindow::on_actionErode_triggered() {
    if (original.Format() != FORMAT_BIN) {
        return;
    }
    converted = original.Erode();
    locateConverted();
    on_actionShow_converted_triggered();
}


void MainWindow::on_actionFind_Dark_triggered() {
    if (original.Format() != FORMAT_GRAY) {
        return;
    }
    converted = original.FindDark();
    locateConverted();
    on_actionShow_converted_triggered();
}


void MainWindow::on_actionCanny_triggered() {
    if (original.Format() != FORMAT_GRAY) {
        return;
    }
    converted = original.Canny();
    locateConverted();
    on_actionShow_converted_triggered();
}

void MainWindow::on_actionFlip_vertically_triggered() {
    if (original.Format() != FORMAT_GRAY && original.Format() != FORMAT_BIN) {
        return;
    }
    converted = original.Flip(true);
    locateConverted();
    on_actionShow_converted_triggered();
}


void MainWindow::on_actionFlip_horizontally_triggered() {
    if (original.Format() != FORMAT_GRAY && original.Format() != FORMAT_BIN) {
        return;
    }
    converted = original.Flip(false);
    locateConverted();
    on_actionShow_converted_triggered();
}


void MainWindow::on_actionSave_original_triggered() {
    if (!original.isNull()) {
        saveQImage(original.Image2QImage(original.Format()));
    }
}


void MainWindow::on_actionSave_converted_triggered() {
    if (!converted.isNull()) {
        saveQImage(converted.Image2QImage(converted.Format()));
    }
}
