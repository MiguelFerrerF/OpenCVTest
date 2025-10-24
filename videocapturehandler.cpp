#include "videocapturehandler.h"
#include <QDebug>

VideoCaptureHandler::VideoCaptureHandler(QObject *parent) : QThread(parent) {
  qDebug()
      << "VideoCaptureHandler::VideoCaptureHandler() - Constructor called.";
}

VideoCaptureHandler::~VideoCaptureHandler() {}

void VideoCaptureHandler::setCamera(const int cameraId) {
  m_requestedCamera = cameraId;
}

// --- NUEVO ---
void VideoCaptureHandler::setManualFocus(bool manual) {
  m_requestedManualFocus = manual;
}

void VideoCaptureHandler::setFocusValue(int value) {
  m_requestedFocusValue = value;
}
// --- FIN NUEVO ---

// --- MODIFICADO ---
void VideoCaptureHandler::run() {
  while (!isInterruptionRequested()) {

    // --- Lógica de cambio de cámara (sin modificar) ---
    if (m_requestedCamera != -1) {
      int requestedCamId = m_requestedCamera.load();
      m_VideoCapture.release();

      if (requestedCamId != -1) {
        if (!m_VideoCapture.open(requestedCamId, cv::CAP_DSHOW)) {
          qWarning() << "No se pudo abrir la cámara" << requestedCamId;
        } else {
          // --- NUEVO: Resetear foco al abrir cámara ---
          // Por defecto, poner la cámara en auto-foco
          m_VideoCapture.set(cv::CAP_PROP_AUTOFOCUS, 1);
          m_isManualFocus = false;
          m_requestedManualFocus = false;
          m_requestedFocusValue = -1;
          // --- FIN NUEVO ---
        }
        m_currentCameraId = requestedCamId;
      } else {
        m_currentCameraId = -1;
      }
      m_requestedCamera = -1;
    }

    if (m_VideoCapture.isOpened()) {

      // --- NUEVO: Lógica de control de foco ---

      // 1. Comprobar si ha cambiado el modo (Auto/Manual)
      bool requestedManual = m_requestedManualFocus.load();
      if (requestedManual != m_isManualFocus) {
        // 0 = Manual, 1 = Auto
        m_VideoCapture.set(cv::CAP_PROP_AUTOFOCUS, requestedManual ? 0 : 1);
        m_isManualFocus = requestedManual;
      }

      // 2. Comprobar si ha cambiado el valor del foco (y estamos en modo
      // manual) Usamos exchange para coger el valor y resetearlo atómicamente
      int requestedValue = m_requestedFocusValue.exchange(-1);
      if (m_isManualFocus && requestedValue != -1) {
        // Mapeamos el valor 0-100 a un rango típico de OpenCV (ej: 0-255)
        // Nota: Este rango puede variar entre cámaras. 255 es un valor común.
        int focusVal = static_cast<int>(requestedValue * 2.55);
        m_VideoCapture.set(cv::CAP_PROP_FOCUS, focusVal);
      }
      // --- FIN NUEVO ---

      // --- Captura de frame (sin modificar) ---
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

// ... (resto del fichero cvMatToQImage y cvMatToQPixmap sin cambios) ...
// ... (asegúrate de que el resto del fichero está presente aquí) ...
QImage VideoCaptureHandler::cvMatToQImage(const cv::Mat &inMat) {
  switch (inMat.type()) {
  // 8-bit, 4 channel
  case CV_8UC4: {
    QImage image(inMat.data, inMat.cols, inMat.rows,
                 static_cast<int>(inMat.step), QImage::Format_ARGB32);

    return image;
  }

  // 8-bit, 3 channel
  case CV_8UC3: {
    QImage image(inMat.data, inMat.cols, inMat.rows,
                 static_cast<int>(inMat.step), QImage::Format_RGB888);

    return image.rgbSwapped();
  }

  // 8-bit, 1 channel
  case CV_8UC1: {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    QImage image(inMat.data, inMat.cols, inMat.rows,
                 static_cast<int>(inMat.step), QImage::Format_Grayscale8);
#else
    static QVector<QRgb> sColorTable;

    // only create our color table the first time
    if (sColorTable.isEmpty()) {
      sColorTable.resize(256);

      for (int i = 0; i < 256; ++i) {
        sColorTable[i] = qRgb(i, i, i);
      }
    }

    QImage image(inMat.data, inMat.cols, inMat.rows,
                 static_cast<int>(inMat.step), QImage::Format_Indexed8);

    image.setColorTable(sColorTable);
#endif

    return image;
  }

  default:
    qWarning()
        << "ASM::cvMatToQImage() - cv::Mat image type not handled in switch:";
    break;
  }

  return QImage();
}

QPixmap VideoCaptureHandler::cvMatToQPixmap(const cv::Mat &inMat) {
  return QPixmap::fromImage(cvMatToQImage(inMat));
}
