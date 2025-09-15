#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QImage>
#include <QPixmap>
#include <QVBoxLayout>

// Función auxiliar: convierte cv::Mat -> QImage
static QImage matToQImage(const cv::Mat &mat) {
  switch (mat.type()) {
  case CV_8UC1: {
    QImage img(
        mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
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
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  ui->imageLabel->setAlignment(Qt::AlignCenter);
  ui->imageLabel->setMinimumSize(640, 480);

  qDebug("Aplicación iniciada");
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::showImage(const cv::Mat &mat) {
  QImage qimg = matToQImage(mat);
  ui->imageLabel->setPixmap(
      QPixmap::fromImage(qimg).scaled(
          ui->imageLabel->size(), Qt::KeepAspectRatio,
          Qt::SmoothTransformation));
}

void MainWindow::on_SelectButton_clicked() {
  qDebug("Botón 'Select Image' presionado.");
  // open file dialog
  QString fileName = QFileDialog::getOpenFileName(
      this, "Select Image", "", "Images (*.png *.xpm *.jpg *.bmp)");
  if (!fileName.isEmpty()) {
    cv::Mat img = cv::imread(fileName.toStdString());
    if (!img.empty()) {
      showImage(img);
      qDebug("Imagen cargada: %s", fileName.toStdString().c_str());
    } else {
      ui->imageLabel->setText("No se pudo cargar la imagen.");
      qDebug("Error al cargar la imagen: %s", fileName.toStdString().c_str());
    }
  } else {
    qDebug("No se seleccionó ninguna imagen.");
  }
}

void MainWindow::on_EraseButton_clicked() {
  ui->imageLabel->clear();
  qDebug("Imagen borrada");
}
