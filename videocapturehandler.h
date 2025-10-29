#ifndef VIDEOCAPTUREHANDLER_H
#define VIDEOCAPTUREHANDLER_H

#include <QImage>
#include <QMetaType>
#include <QPixmap>
#include <QSize> // --- SIN CAMBIOS ---
#include <QThread>
#include <atomic>
#include <opencv2/opencv.hpp>

#define ID_CAMERA_DEFAULT 0

// ... (Struct CameraPropertiesSupport sin cambios, excepto por el comentario) ...
// Estructura para informar qué propiedades de cámara son soportadas (bool)
struct CameraPropertiesSupport {
  bool autoFocus = false;
  bool focus = true;
  bool autoExposure = false;
  bool exposure = false;
  bool brightness = false;
  bool contrast = false;
  bool saturation = false;
  bool sharpness = false;
};
Q_DECLARE_METATYPE(CameraPropertiesSupport)

// --- NUEVO: Estructuras para rangos dinámicos de propiedades ---
struct PropertyRange {
  double min = 0;
  double max = 255;
  double current = 0; // Valor actual o por defecto
};

struct CameraPropertyRanges {
  PropertyRange brightness;
  PropertyRange contrast;
  PropertyRange saturation;
  PropertyRange sharpness;
  PropertyRange focus;
  PropertyRange exposure;
};
Q_DECLARE_METATYPE(CameraPropertyRanges)
// --- FIN NUEVO ---

class VideoCaptureHandler : public QThread {
  Q_OBJECT
public:
  VideoCaptureHandler(QObject *parent = nullptr);
  ~VideoCaptureHandler();

  // --- MODIFICADO (SIN CAMBIOS REALES) ---
  void requestCameraChange(int cameraId, const QSize &resolution);

  // ... (Setters de foco y propiedades sin cambios) ...
  void setAutoFocus(bool manual);
  void setAutoExposure(bool manual);
  void setBrightness(int value);
  void setContrast(int value);
  void setSaturation(int value);
  void setSharpness(int value);
  void setFocus(int value);
  void setExposure(int value);

signals:
  void newPixmapCaptured(const QPixmap &pixmap);
  void propertiesSupported(CameraPropertiesSupport support);

  // --- NUEVO: Señales para rangos y errores ---
  void rangesSupported(const CameraPropertyRanges &ranges);
  void cameraOpenFailed(int cameraId, const QString &errorMsg);
  // --- FIN NUEVO ---

protected:
  void run() override;

private:
  QPixmap m_pixmap;
  cv::Mat m_frame;
  cv::VideoCapture m_VideoCapture;

  int m_currentCameraId{ID_CAMERA_DEFAULT};

  // -2 = No-Op, -1 = Stop, >= 0 = Start
  std::atomic<int> m_requestedCamera{-2};
  std::atomic<int> m_requestedWidth{0};
  std::atomic<int> m_requestedHeight{0};

  // ... (Atómicas de foco y propiedades sin cambios) ...
  std::atomic<int> m_requestedAutoFocus{-1};
  std::atomic<int> m_requestedFocus{-1};
  std::atomic<int> m_requestedAutoExposure{-1};
  std::atomic<int> m_requestedExposure{-1};
  std::atomic<int> m_requestedBrightness{-1};
  std::atomic<int> m_requestedContrast{-1};
  std::atomic<int> m_requestedSaturation{-1};
  std::atomic<int> m_requestedSharpness{-1};

  QImage cvMatToQImage(const cv::Mat &inMat);
  QPixmap cvMatToQPixmap(const cv::Mat &inMat);

  // --- NUEVO: Helper para obtener rangos de propiedades de OpenCV ---
  PropertyRange getPropertyRange(int propId);
  // --- FIN NUEVO ---
};

#endif // VIDEOCAPTUREHANDLER_H