#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QScreen>
#include <QGuiApplication>
#include <QImage>
#include <QPixmap>
#include <QTextCodec>

// @brief: initial window
// @param parent:
// @ret: void
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->intensity->setMaximum(1.00);
    ui->intensity->setValue(0.1);
    ui->intensity->setMinimum(0.001);
    ui->intensity->setSingleStep(0.01);
    ui->radius->setMaximum(10.00);
    ui->radius->setValue(1);
    ui->radius->setMinimum(0.5);
    ui->radius->setSingleStep(0.5);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// @brief: show LUT when preview button be pushed
// @param: void
// @ret: void
void MainWindow::on_pushButton_clicked()
{

    //get color spcae setting "false" == "linear" // "true" == "sRGB"
    bool color = false;
    if (ui->colorSpace->currentText() == "sRGB") {
        color = true;
    }

    //get tonemap setting
    bool tonemap = true;
    if (ui->toneMapping->currentIndex() == 1) {
        tonemap = false;
    }
    //create my lut
    myLut = new LutGenerator(32, ui->intensity->value(), ui->radius->value(), color, tonemap);

    //preview lut
    tempImage = new QImage(myLut->getData(),32,32,QImage::Format_RGB888);
    ui->preview->setPixmap(QPixmap::fromImage(*tempImage).scaled(ui->preview->width(), ui->preview->height(),Qt::KeepAspectRatio,Qt::SmoothTransformation));

    //delete lut and preview
    delete myLut;
    delete tempImage;
}

// @brief: save LUT when saving button be pushed
// @param: void
// @ret: void
void MainWindow::on_pushButton_2_clicked()
{
    //get the saving address
    QString filename = QFileDialog::getSaveFileName(this,tr("Save Image"),"LUT.bmp",tr(".bmp")); //选择路径

    QTextCodec *code = QTextCodec::codecForName("GB2312");//解决中文路径问题
    std::string name = code->fromUnicode(filename).data();

    //get size of the lut
    QString pixelString = ui->comboBox->currentText();
    int pixelInt = pixelString.toInt();

    //get color spcae setting "false" == "linear" // "true" == "sRGB"
    bool color = false;
    if (ui->colorSpace->currentText() == "sRGB") {
        color = true;
    }

    //get tonemap setting
    bool tonemap = true;
    if (ui->toneMapping->currentIndex() == 1) {
        tonemap = false;
    }

    //create my lut
    myLut = new LutGenerator(pixelInt, ui->intensity->value(), ui->radius->value(), color, tonemap, name);

    //delete lut
    delete myLut;
    QMessageBox::about(this,tr("Message"),tr("Finish Generating"));
}
