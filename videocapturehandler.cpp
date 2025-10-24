#include "videocapturehandler.h"
#include <QDebug>

VideoCaptureHandler::VideoCaptureHandler(QObject *parent) : QThread(parent) {
  qDebug()
      << "VideoCaptureHandler::VideoCaptureHandler() - Constructor called.";
}

VideoCaptureHandler::~VideoCaptureHandler() {
  // La liberación principal se hace al final de run()
}

void VideoCaptureHandler::setCamera(const int cameraId) {
  m_requestedCamera = cameraId;
}

// Se ha eliminado la función getPixmap()

void VideoCaptureHandler::run() {
  while (!isInterruptionRequested()) {

    if (m_requestedCamera != -1) { // Hay una nueva solicitud de cámara
      int requestedCamId = m_requestedCamera.load();
      m_VideoCapture.release(); // Cierra la cámara anterior (si la hay)

      if (requestedCamId != -1) { // Si el ID no es -1, intenta abrirla
        if (!m_VideoCapture.open(requestedCamId, cv::CAP_DSHOW)) {
          qWarning() << "No se pudo abrir la cámara" << requestedCamId;
        }
        m_currentCameraId = requestedCamId;
      } else {
        m_currentCameraId = -1; // Se solicitó parar (ID -1)
      }
      m_requestedCamera = -1; // Marcamos la solicitud como procesada
    }

    if (m_VideoCapture.isOpened()) {
      m_VideoCapture >> m_frame;
      if (!m_frame.empty()) {
        m_pixmap = cvMatToQPixmap(m_frame);
        // Se emite la señal con el pixmap como argumento
        emit newPixmapCaptured(m_pixmap);
      }
      QThread::msleep(10); // Pausa breve si la cámara funciona
    } else {
      // Si no hay cámara abierta, dormimos un poco más
      // para no consumir CPU inútilmente.
      QThread::msleep(100);
    }
  }

  // Limpieza final al salir del hilo
  m_VideoCapture.release();
  qDebug() << "VideoCaptureHandler::run() - Hilo terminado y cámara liberada.";
}

QImage VideoCaptureHandler::cvMatToQImage(const cv::Mat &inMat) {
  switch (inMat.type()) {
  // 8-bit, 4 channel
  case CV_8UC4: {
    QImage image(
        inMat.data, inMat.cols, inMat.rows, static_cast<int>(inMat.step),
        QImage::Format_ARGB32);

    return image;
  }

  // 8-bit, 3 channel
  case CV_8UC3: {
    QImage image(
        inMat.data, inMat.cols, inMat.rows, static_cast<int>(inMat.step),
        QImage::Format_RGB888);

    return image.rgbSwapped();
  }

  // 8-bit, 1 channel
  case CV_8UC1: {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    QImage image(
        inMat.data, inMat.cols, inMat.rows, static_cast<int>(inMat.step),
        QImage::Format_Grayscale8);
#else
    static QVector<QRgb> sColorTable;

    // only create our color table the first time
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
    qWarning()
        << "ASM::cvMatToQImage() - cv::Mat image type not handled in switch:"
        << inMat.type();
    break;
  }

  return QImage();
}

QPixmap VideoCaptureHandler::cvMatToQPixmap(const cv::Mat &inMat) {
  return QPixmap::fromImage(cvMatToQImage(inMat));
}