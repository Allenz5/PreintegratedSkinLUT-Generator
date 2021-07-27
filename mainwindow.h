#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "lutgenerator.h"
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void horizontalSlider_slot();
    void intensity_slot();
    void horizontalSlider_2_slot();
    void radius_slot();

private:
    Ui::MainWindow *ui;
    LutGenerator *myLut;
    QImage *tempImage;
};

#endif // MAINWINDOW_H
