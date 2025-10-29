#include "videocapturehandler.h"
#include <QDebug>
#include <QtMath> // Necesario para qFuzzyIsNull en mainwindow.cpp, pero no en este .cpp

VideoCaptureHandler::VideoCaptureHandler(QObject *parent) : QThread(parent) {
  qDebug() << "VideoCaptureHandler::VideoCaptureHandler() - Constructor called.";
  qRegisterMetaType<CameraPropertiesSupport>();
  // --- NUEVO: Registrar metatipo para los rangos ---
  qRegisterMetaType<CameraPropertyRanges>();
  // --- FIN NUEVO ---
}

VideoCaptureHandler::~VideoCaptureHandler() {}

void VideoCaptureHandler::requestCameraChange(int cameraId, const QSize &resolution) {
  m_requestedWidth = resolution.width();
  m_requestedHeight = resolution.height();
  // Se asigna al final, ya que actúa como 'trigger' en el bucle run()
  m_requestedCamera = cameraId;
}

// ... (Setters de foco y propiedades sin cambios) ...
void VideoCaptureHandler::setAutoFocus(bool manual) {
  m_requestedAutoFocus = manual ? true : false;
}
void VideoCaptureHandler::setAutoExposure(bool manual) {
  m_requestedAutoExposure = manual ? true : false;
}
void VideoCaptureHandler::setFocus(int value) { m_requestedFocus = value; }
void VideoCaptureHandler::setBrightness(int value) { m_requestedBrightness = value; }
void VideoCaptureHandler::setContrast(int value) { m_requestedContrast = value; }
void VideoCaptureHandler::setSaturation(int value) { m_requestedSaturation = value; }
void VideoCaptureHandler::setSharpness(int value) { m_requestedSharpness = value; }

void VideoCaptureHandler::setExposure(int value) { m_requestedExposure = value; }

// --- MODIFICADO: Implementación corregida del helper para obtener rangos ---
PropertyRange VideoCaptureHandler::getPropertyRange(int propId) {
  PropertyRange range;

  // Si la cámara no está abierta, no podemos obtener propiedades.
  if (!m_VideoCapture.isOpened()) {
    return range;
  }

  // 1. Obtener el valor actual de la propiedad, si existe.
  double currentValue = m_VideoCapture.get(propId);
  range.current = currentValue;
  // 2. Intentar obtener los valores mínimo y máximo.
  range.min = 0;
  range.max = 255;
  if (qFuzzyIsNull(range.current)) { // Si el valor actual es esencialmente 0
    range.current = 126;             // Asumir valor central o por defecto si no se pudo leer
  }

  return range;
}
// --- FIN MODIFICADO ---

// --- MODIFICADO: Lógica de run() con portabilidad, error y rangos ---
void VideoCaptureHandler::run() {
  while (!isInterruptionRequested()) {

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
          emit cameraOpenFailed(
              requestedCamId, "Error al abrir la cámara con CAP_ANY y CAP_DSHOW.");
        }

        // --- FIN MODIFICADO ---

        if (m_VideoCapture.isOpened()) {

          // Aplicar la resolución solicitada
          if (reqWidth > 0 && reqHeight > 0) {
            m_VideoCapture.set(cv::CAP_PROP_FRAME_WIDTH, reqWidth);
            m_VideoCapture.set(cv::CAP_PROP_FRAME_HEIGHT, reqHeight);
            qDebug() << "Solicitando resolución:" << reqWidth << "x" << reqHeight;
          }

          // 1. Comprobación de propiedades y emite qué propiedades son soportadas
          CameraPropertiesSupport support;
          support.brightness = (m_VideoCapture.get(cv::CAP_PROP_BRIGHTNESS) != 0);
          support.contrast = (m_VideoCapture.get(cv::CAP_PROP_CONTRAST) != 0);
          support.saturation = (m_VideoCapture.get(cv::CAP_PROP_SATURATION) != 0);
          support.sharpness = (m_VideoCapture.get(cv::CAP_PROP_SHARPNESS) != 0);
          support.autoExposure = (m_VideoCapture.get(cv::CAP_PROP_AUTO_EXPOSURE) != 0);
          support.exposure = (m_VideoCapture.get(cv::CAP_PROP_EXPOSURE) != 0);
          support.autoFocus = (m_VideoCapture.get(cv::CAP_PROP_AUTOFOCUS) != 0);
          support.focus = (m_VideoCapture.get(cv::CAP_PROP_FOCUS) == 0);
          // --- FIN CORRECCIÓN ---

          qDebug() << "CameraPropertiesSupport - autoFocus:" << support.autoFocus
                   << "\nfocus:" << support.focus << "\nautoExposure:" << support.autoExposure
                   << "\nexposure:" << support.exposure << "\nbrightness:" << support.brightness
                   << "\ncontrast:" << support.contrast << "\nsaturation:" << support.saturation
                   << "\nsharpness:" << support.sharpness;

          emit propertiesSupported(support);

          // 2. Recopilar y emitir rangos de propiedades
          CameraPropertyRanges ranges;
          ranges.brightness = getPropertyRange(cv::CAP_PROP_BRIGHTNESS);
          ranges.contrast = getPropertyRange(cv::CAP_PROP_CONTRAST);
          ranges.saturation = getPropertyRange(cv::CAP_PROP_SATURATION);
          ranges.sharpness = getPropertyRange(cv::CAP_PROP_SHARPNESS);
          ranges.exposure = getPropertyRange(cv::CAP_PROP_EXPOSURE);
          ranges.focus = getPropertyRange(cv::CAP_PROP_FOCUS);
          emit rangesSupported(ranges);

          m_requestedAutoFocus = -1;
          m_requestedFocus = -1;
          m_requestedBrightness = -1;
          m_requestedContrast = -1;
          m_requestedSaturation = -1;
          m_requestedSharpness = -1;
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
      reqValue = m_requestedSharpness.exchange(-1);
      if (reqValue != -1)
        m_VideoCapture.set(cv::CAP_PROP_SHARPNESS, reqValue);
      reqValue = m_requestedAutoExposure.exchange(-1);
      if (reqValue != -1)
        m_VideoCapture.set(cv::CAP_PROP_AUTO_EXPOSURE, reqValue);
      reqValue = m_requestedExposure.exchange(-1);
      if (reqValue != -1)
        m_VideoCapture.set(cv::CAP_PROP_EXPOSURE, reqValue);
      reqValue = m_requestedAutoFocus.exchange(-1);
      if (reqValue != -1)
        m_VideoCapture.set(cv::CAP_PROP_AUTOFOCUS, reqValue);
      reqValue = m_requestedFocus.exchange(-1);
      if (reqValue != -1)
        m_VideoCapture.set(cv::CAP_PROP_FOCUS, reqValue);

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
        inMat.data, inMat.cols, inMat.rows, static_cast<int>(inMat.step), QImage::Format_ARGB32);
    return image;
  }
  case CV_8UC3: {
    QImage image(
        inMat.data, inMat.cols, inMat.rows, static_cast<int>(inMat.step), QImage::Format_RGB888);
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
        inMat.data, inMat.cols, inMat.rows, static_cast<int>(inMat.step), QImage::Format_Indexed8);
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