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

  // --- NUEVO ---
  // Métodos públicos para ser llamados desde MainWindow
  void setManualFocus(bool manual);
  void setFocusValue(int value);
  // --- FIN NUEVO ---

signals:
  void newPixmapCaptured(const QPixmap &pixmap);

protected:
  void run() override;

private:
  QPixmap m_pixmap;
  cv::Mat m_frame;
  cv::VideoCapture m_VideoCapture;

  int m_currentCameraId{ID_CAMERA_DEFAULT};
  std::atomic<int> m_requestedCamera{-1};

  // --- NUEVO ---
  // Variables atómicas para controlar el foco desde la UI
  std::atomic<bool> m_requestedManualFocus{
      false};                                 // false = Auto, true = Manual
  std::atomic<int> m_requestedFocusValue{-1}; // Valor de foco (0-100)

  // Variable para guardar el estado actual del foco
  bool m_isManualFocus{false};
  // --- FIN NUEVO ---

  QImage cvMatToQImage(const cv::Mat &inMat);
  QPixmap cvMatToQPixmap(const cv::Mat &inMat);
};

#endif // VIDEOCAPTUREHANDLER_H