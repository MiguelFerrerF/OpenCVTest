#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <opencv2/opencv.hpp>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void on_SelectButton_clicked();
  void on_EraseButton_clicked();

private:
  Ui::MainWindow *ui;
  void showImage(const cv::Mat &mat);
};
#endif // MAINWINDOW_H
