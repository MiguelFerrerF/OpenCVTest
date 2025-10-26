#include "videocapturehandler.h"
#include <QDebug>

VideoCaptureHandler::VideoCaptureHandler(QObject *parent) : QThread(parent) {
  qDebug()
      << "VideoCaptureHandler::VideoCaptureHandler() - Constructor called.";
  qRegisterMetaType<CameraPropertiesSupport>();
}

VideoCaptureHandler::~VideoCaptureHandler() {}

// --- MODIFICADO ---
void VideoCaptureHandler::requestCameraChange(
    int cameraId, const QSize &resolution) {
  m_requestedWidth = resolution.width();
  m_requestedHeight = resolution.height();
  // Se asigna al final, ya que actúa como 'trigger' en el bucle run()
  m_requestedCamera = cameraId;
}
// --- FIN MODIFICADO ---

// ... (Setters de foco y propiedades sin cambios) ...
void VideoCaptureHandler::setManualFocus(bool manual) {
  m_requestedManualFocus = manual;
}
void VideoCaptureHandler::setFocusValue(int value) {
  m_requestedFocusValue = value;
}
void VideoCaptureHandler::setBrightness(int value) {
  m_requestedBrightness = value;
}
void VideoCaptureHandler::setContrast(int value) {
  m_requestedContrast = value;
}
void VideoCaptureHandler::setSaturation(int value) {
  m_requestedSaturation = value;
}
void VideoCaptureHandler::setHue(int value) { m_requestedHue = value; }
void VideoCaptureHandler::setGain(int value) { m_requestedGain = value; }
void VideoCaptureHandler::setAutoExposure(bool manual) {
  m_requestedAutoExposure = manual ? 1 : 0;
}
void VideoCaptureHandler::setExposure(int value) {
  m_requestedExposure = value;
}

// --- MODIFICADO ---
void VideoCaptureHandler::run() {
  while (!isInterruptionRequested()) {

    // --- MODIFICADO: Lógica de petición de cámara ---
    // Atómicamente coge el valor de 'requestedCamera' y lo resetea a -2 (No-Op)
    int requestedCamId = m_requestedCamera.exchange(-2);

    // Si no hay petición (es -2), saltar esta parte
    if (requestedCamId != -2) {

      m_VideoCapture.release(); // Cerrar cámara anterior

      // requestedCamId >= 0 es una petición de START
      if (requestedCamId >= 0) {

        // Cargar la resolución solicitada
        int reqWidth = m_requestedWidth.load();
        int reqHeight = m_requestedHeight.load();

        if (!m_VideoCapture.open(requestedCamId, cv::CAP_DSHOW)) {
          qWarning() << "No se pudo abrir la cámara" << requestedCamId;
        } else {

          // --- NUEVO: Aplicar resolución ANTES de chequear propiedades ---
          if (reqWidth > 0 && reqHeight > 0) {
            m_VideoCapture.set(cv::CAP_PROP_FRAME_WIDTH, reqWidth);
            m_VideoCapture.set(cv::CAP_PROP_FRAME_HEIGHT, reqHeight);
            qDebug() << "Solicitando resolución:" << reqWidth << "x"
                     << reqHeight;
          }
          // --- FIN NUEVO ---

          // ... (Comprobación de propiedades y reseteo de estados sin cambios)
          // ...
          CameraPropertiesSupport support;
          support.brightness =
              (m_VideoCapture.get(cv::CAP_PROP_BRIGHTNESS) != -1);
          support.contrast = (m_VideoCapture.get(cv::CAP_PROP_CONTRAST) != -1);
          support.saturation =
              (m_VideoCapture.get(cv::CAP_PROP_SATURATION) != -1);
          support.hue = (m_VideoCapture.get(cv::CAP_PROP_HUE) != -1);
          support.gain = (m_VideoCapture.get(cv::CAP_PROP_GAIN) != -1);
          support.autoExposure =
              (m_VideoCapture.get(cv::CAP_PROP_AUTO_EXPOSURE) != -1);
          support.exposure = (m_VideoCapture.get(cv::CAP_PROP_EXPOSURE) != -1);
          emit propertiesSupported(support);

          m_VideoCapture.set(cv::CAP_PROP_AUTOFOCUS, 1);
          m_isManualFocus = false;
          m_requestedManualFocus = false;
          m_requestedFocusValue = -1;
          m_requestedBrightness = -1;
          m_requestedContrast = -1;
          m_requestedSaturation = -1;
          m_requestedHue = -1;
          m_requestedGain = -1;
          m_requestedAutoExposure = -1;
          m_requestedExposure = -1;
        }
        m_currentCameraId = requestedCamId;
      }
      // requestedCamId == -1 es una petición de STOP
      else if (requestedCamId == -1) {
        m_currentCameraId = -1;
      }
    }
    // --- FIN MODIFICADO ---

    if (m_VideoCapture.isOpened()) {

      // ... (Aplicación de propiedades de foco y ajuste sin cambios) ...
      int reqValue = -1;
      reqValue = m_requestedBrightness.exchange(-1);
      if (reqValue != -1)
        m_VideoCapture.set(cv::CAP_PROP_BRIGHTNESS, reqValue);
      reqValue = m_requestedContrast.exchange(-1);
      if (reqValue != -1)
        m_VideoCapture.set(cv::CAP_PROP_CONTRAST, reqValue);
      reqValue = m_requestedSaturation.exchange(-1);
      if (reqValue != -1)
        m_VideoCapture.set(cv::CAP_PROP_SATURATION, reqValue);
      reqValue = m_requestedHue.exchange(-1);
      if (reqValue != -1)
        m_VideoCapture.set(cv::CAP_PROP_HUE, reqValue);
      reqValue = m_requestedGain.exchange(-1);
      if (reqValue != -1)
        m_VideoCapture.set(cv::CAP_PROP_GAIN, reqValue);
      reqValue = m_requestedAutoExposure.exchange(-1);
      if (reqValue != -1)
        m_VideoCapture.set(cv::CAP_PROP_AUTO_EXPOSURE, reqValue);
      reqValue = m_requestedExposure.exchange(-1);
      if (reqValue != -1)
        m_VideoCapture.set(cv::CAP_PROP_EXPOSURE, reqValue);
      bool requestedManual = m_requestedManualFocus.load();
      if (requestedManual != m_isManualFocus) {
        m_VideoCapture.set(cv::CAP_PROP_AUTOFOCUS, requestedManual ? 0 : 1);
        m_isManualFocus = requestedManual;
      }
      int requestedFocusValue = m_requestedFocusValue.exchange(-1);
      if (m_isManualFocus && requestedFocusValue != -1) {
        int focusVal = static_cast<int>(requestedFocusValue * 2.55);
        m_VideoCapture.set(cv::CAP_PROP_FOCUS, focusVal);
      }
      // --- FIN ZONA SIN CAMBIOS ---

      m_VideoCapture >> m_frame;
      if (!m_frame.empty()) {
        m_pixmap = cvMatToQPixmap(m_frame);
        emit newPixmapCaptured(m_pixmap);
      }
      QThread::msleep(10);
    } else {
      QThread::msleep(100);
    }
  }

  m_VideoCapture.release();
  qDebug() << "VideoCaptureHandler::run() - Hilo terminado y cámara liberada.";
}

// ... (cvMatToQImage y cvMatToQPixmap sin cambios) ...
QImage VideoCaptureHandler::cvMatToQImage(const cv::Mat &inMat) {
  switch (inMat.type()) {
  case CV_8UC4: {
    QImage image(
        inMat.data, inMat.cols, inMat.rows, static_cast<int>(inMat.step),
        QImage::Format_ARGB32);
    return image;
  }
  case CV_8UC3: {
    QImage image(
        inMat.data, inMat.cols, inMat.rows, static_cast<int>(inMat.step),
        QImage::Format_RGB888);
    return image.rgbSwapped();
  }
  case CV_8UC1: {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    QImage image(
        inMat.data, inMat.cols, inMat.rows, static_cast<int>(inMat.step),
        QImage::Format_Grayscale8);
#else
    static QVector<QRgb> sColorTable;
    if (sColorTable.isEmpty()) {
      sColorTable.resize(256);
      for (int i = 0; i < 256; ++i) {
        sColorTable[i] = qRgb(i, i, i);
      }
    }
    QImage image(
        inMat.data, inMat.cols, inMat.rows, static_cast<int>(inMat.step),
        QImage::Format_Indexed8);
    image.setColorTable(sColorTable);
#endif
    return image;
  }
  default:
    qWarning() << "ASM::cvMatToQImage() - cv::Mat image type not handled in "
                  "switch:"
               << inMat.type();
    break;
  }
  return QImage();
}

QPixmap VideoCaptureHandler::cvMatToQPixmap(const cv::Mat &inMat) {
  return QPixmap::fromImage(cvMatToQImage(inMat));
}