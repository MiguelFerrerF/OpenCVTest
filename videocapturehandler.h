#ifndef VIDEOCAPTUREHANDLER_H
#define VIDEOCAPTUREHANDLER_H

#include <QImage>
#include <QPixmap>
#include <QThread>
#include <atomic>
#include <opencv2/opencv.hpp>

#define ID_CAMERA_DEFAULT 0

class VideoCaptureHandler : public QThread {
  Q_OBJECT
public:
  VideoCaptureHandler(QObject *parent = nullptr);
  ~VideoCaptureHandler();

  void setCamera(const int cameraId);

signals:
  // Se ha modificado la señal para pasar el QPixmap de forma segura
  void newPixmapCaptured(const QPixmap &pixmap);

protected:
  void run() override;

private:
  QPixmap m_pixmap;
  cv::Mat m_frame;
  cv::VideoCapture m_VideoCapture;

  int m_currentCameraId{ID_CAMERA_DEFAULT};
  std::atomic<int> m_requestedCamera{-1};

  QImage cvMatToQImage(const cv::Mat &inMat);
  QPixmap cvMatToQPixmap(const cv::Mat &inMat);
};

#endif // VIDEOCAPTUREHANDLER_H