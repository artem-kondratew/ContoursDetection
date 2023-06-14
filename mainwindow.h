#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include "detection.h"
#include "image.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    void centerWidgets();
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    void locateImage(Image img);

private slots:
    void on_imgUploadButton_clicked();

    void on_actionClose_triggered();

private:
    Ui::MainWindow *ui;
};


#endif // MAINWINDOW_H
