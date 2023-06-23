#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include "./ui_mainwindow.h"
#include "image.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    QDoubleSpinBox* DoubleSpinBoxArray[KERNEL_SIZE];

private:
    void centerWidgets();
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void saveQImage(QImage qimage);

protected:
    void resizeEvent(QResizeEvent *event);
    Image useConv();

private:
    void locateImage(Image* left_img, int left_format, Image* right_img, int right_format);

private slots:
    void on_imgUploadButton_clicked();

    void on_actionClose_triggered();

    void on_actionShow_triggered(bool checked);

    void on_convPushButton_clicked();

private:
    Ui::MainWindow *ui;
};




#endif // MAINWINDOW_H
