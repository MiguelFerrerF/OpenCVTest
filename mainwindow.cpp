#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QVBoxLayout>
#include <QImage>
#include <QPixmap>

// Función auxiliar: convierte cv::Mat -> QImage
static QImage matToQImage(const cv::Mat &mat) {
    switch (mat.type()) {
    case CV_8UC1: {
        QImage img(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
        return img.copy();
    }
    case CV_8UC3: {
        QImage img(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return img.rgbSwapped(); // OpenCV usa BGR
    }
    case CV_8UC4: {
        QImage img(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return img.copy();
    }
    default:
        return QImage();
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setMinimumSize(640, 480);

    setCentralWidget(imageLabel);

    // Cargar imagen con OpenCV
    cv::Mat img = cv::imread("img.jpg"); // asegúrate que esté en la carpeta del ejecutable
    if (!img.empty()) {
        showImage(img);
    } else {
        imageLabel->setText("No se pudo cargar la imagen.");
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showImage(const cv::Mat &mat) {
    QImage qimg = matToQImage(mat);
    imageLabel->setPixmap(QPixmap::fromImage(qimg).scaled(imageLabel->size(),
                                                          Qt::KeepAspectRatio,
                                                          Qt::SmoothTransformation));
}
