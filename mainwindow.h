#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <chrono>
#include <QMainWindow>
#include <QCamera>
#include <QVideoWidget>
#include <QMediaPlayer>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include "./ui_mainwindow.h"
#include "image.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    Image original;
    Image converted;

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
    void locateOriginal();
    void locateConverted();

private slots:
    void on_imgUploadButton_clicked();

    void on_actionClose_triggered();

    void on_actionShow_triggered(bool checked);

    void on_convPushButton_clicked();

    void on_actionShow_original_triggered();

    void on_actionShow_converted_triggered();

    void on_actionGray_triggered();

    void on_actionBin_triggered();

    void on_actionGauss_triggered();

    void on_actionSet_as_original_triggered();

    void on_actionSobel_x_triggered();

    void on_actionSobel_y_triggered();

    void on_actionExternal_Contouring_triggered();

    void on_actionDilate_triggered();

    void on_actionErode_triggered();

    void on_actionFind_Dark_triggered();

    void on_actionCanny_triggered();

    void on_actionFlip_vertically_triggered();

    void on_actionFlip_horizontally_triggered();

    void on_actionSave_original_triggered();

    void on_actionSave_converted_triggered();

private:
    Ui::MainWindow *ui;
};




#endif // MAINWINDOW_H
