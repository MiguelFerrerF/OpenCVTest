#ifndef VIDEOCAPTUREHANDLER_H
#define VIDEOCAPTUREHANDLER_H

#include <QImage>
#include <QMetaType>
#include <QPixmap>
#include <QSize> // --- NUEVO ---
#include <QThread>
#include <atomic>
#include <opencv2/opencv.hpp>

#define ID_CAMERA_DEFAULT 0

// ... (Struct CameraPropertiesSupport sin cambios) ...
struct CameraPropertiesSupport {
  bool brightness;
  bool contrast;
  bool saturation;
  bool hue;
  bool gain;
  bool autoExposure;
  bool exposure;
};
Q_DECLARE_METATYPE(CameraPropertiesSupport)

class VideoCaptureHandler : public QThread {
  Q_OBJECT
public:
  VideoCaptureHandler(QObject *parent = nullptr);
  ~VideoCaptureHandler();

  // --- MODIFICADO ---
  void requestCameraChange(int cameraId, const QSize &resolution);
  // --- FIN MODIFICADO ---

  // ... (Setters de foco y propiedades sin cambios) ...
  void setManualFocus(bool manual);
  void setFocusValue(int value);
  void setBrightness(int value);
  void setContrast(int value);
  void setSaturation(int value);
  void setHue(int value);
  void setGain(int value);
  void setAutoExposure(bool manual);
  void setExposure(int value);

signals:
  void newPixmapCaptured(const QPixmap &pixmap);
  void propertiesSupported(CameraPropertiesSupport support);

protected:
  void run() override;

private:
  QPixmap m_pixmap;
  cv::Mat m_frame;
  cv::VideoCapture m_VideoCapture;

  int m_currentCameraId{ID_CAMERA_DEFAULT};

  // --- MODIFICADO ---
  // -2 = No-Op, -1 = Stop, >= 0 = Start
  std::atomic<int> m_requestedCamera{-2};
  std::atomic<int> m_requestedWidth{0};
  std::atomic<int> m_requestedHeight{0};
  // --- FIN MODIFICADO ---

  // ... (Atómicas de foco y propiedades sin cambios) ...
  std::atomic<bool> m_requestedManualFocus{false};
  std::atomic<int> m_requestedFocusValue{-1};
  bool m_isManualFocus{false};
  std::atomic<int> m_requestedBrightness{-1};
  std::atomic<int> m_requestedContrast{-1};
  std::atomic<int> m_requestedSaturation{-1};
  std::atomic<int> m_requestedHue{-1};
  std::atomic<int> m_requestedGain{-1};
  std::atomic<int> m_requestedAutoExposure{-1};
  std::atomic<int> m_requestedExposure{-1};

  QImage cvMatToQImage(const cv::Mat &inMat);
  QPixmap cvMatToQPixmap(const cv::Mat &inMat);
};

#endif // VIDEOCAPTUREHANDLER_H