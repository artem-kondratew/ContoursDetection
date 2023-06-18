#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QDoubleSpinBox>
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

protected:
    void resizeEvent(QResizeEvent *event);

private:
    void locateImage(Image* img, Image* gray);

private slots:
    void on_imgUploadButton_clicked();

    void on_actionClose_triggered();

    void on_actionShow_triggered(bool checked);

    void on_convPushButton_clicked();

private:
    Ui::MainWindow *ui;
};




#endif // MAINWINDOW_H
