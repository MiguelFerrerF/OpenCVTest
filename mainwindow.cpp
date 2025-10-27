#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "videocapturehandler.h"
#include <QCameraDevice>
#include <QMediaDevices>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      m_videoCaptureHandler(new VideoCaptureHandler(this)) {
  ui->setupUi(this);

  // Conexión para recibir nuevos pixmaps capturados
  connect(
      m_videoCaptureHandler, &VideoCaptureHandler::newPixmapCaptured, this,
      [=](const QPixmap &pixmap) {
        m_currentPixmap = pixmap;
        updateVideoLabel();
      });

  // Rellenar ComboBox de cámaras
  QStringList cameraNames;
  for (const QCameraDevice &camera : QMediaDevices::videoInputs()) {
    cameraNames << camera.description();
  }
  ui->comboBoxCameras->addItems(cameraNames);

  if (cameraNames.isEmpty()) {
    ui->startButton->setEnabled(false);
    ui->videoLabel->setText("No se han detectado cámaras.");
  }

  setAllControlsEnabled(false);

  m_videoCaptureHandler->start(QThread::HighestPriority);
}

MainWindow::~MainWindow() {
  m_videoCaptureHandler->requestInterruption();
  m_videoCaptureHandler->wait();
  delete ui;
}

// --- MODIFICADO ---
void MainWindow::on_startButton_clicked() {
  if (ui->startButton->isChecked()) {
    // Estado: ON (Iniciar)
    int cameraId = ui->comboBoxCameras->currentIndex();

    // --- NUEVO: Obtener resolución ---
    QString resText = ui->comboBoxResolution->currentText();
    QSize resolution = parseResolution(resText);
    // --- FIN NUEVO ---

    // --- MODIFICADO: Enviar ID y Resolución ---
    m_videoCaptureHandler->requestCameraChange(cameraId, resolution);

    ui->startButton->setText("Stop");
    ui->comboBoxCameras->setEnabled(false);
    ui->comboBoxResolution->setEnabled(false);

  } else {
    // Estado: OFF (Detener)
    // --- MODIFICADO: Enviar -1 y QSize() vacío ---
    m_videoCaptureHandler->requestCameraChange(-1, QSize());

    ui->startButton->setText("Start OpenCV");
    ui->comboBoxCameras->setEnabled(true);
    ui->comboBoxResolution->setEnabled(true);

    m_currentPixmap = QPixmap();
    ui->videoLabel->clear();
    ui->videoLabel->setText("Cámara detenida.");

    setAllControlsEnabled(true);
  }
}

// ... (Slots de foco y propiedades sin cambios) ...
void MainWindow::on_checkBoxFocoAuto_toggled(bool checked) {
  m_videoCaptureHandler->setManualFocus(checked);
  ui->horizontalSliderFocoAuto->setEnabled(m_support.brightness && checked);
}
void MainWindow::on_horizontalSliderFocoAuto_sliderMoved(int value) {
  m_videoCaptureHandler->setFocusValue(value);
}
void MainWindow::on_propertiesSupported(CameraPropertiesSupport support) {
  m_support = support;
  ui->horizontalSliderBrillo->setEnabled(support.brightness);
  ui->horizontalSliderFocoAuto->setEnabled(support.brightness && ui->checkBoxFocoAuto->isChecked());
  ui->horizontalSliderBrillo->setEnabled(support.brightness);
  ui->horizontalSliderContraste->setEnabled(support.contrast);
  ui->horizontalSliderSaturacion->setEnabled(support.saturation);
  ui->horizontalSliderNitidez->setEnabled(support.sharpness);
  ui->checkBoxExposicionAuto->setEnabled(support.autoExposure);
  ui->horizontalSliderExposicionAuto->setEnabled(
      support.exposure && !ui->checkBoxExposicionAuto->isChecked());
}
void MainWindow::on_horizontalSliderBrillo_sliderMoved(int value) {
  m_videoCaptureHandler->setBrightness(value);
}
void MainWindow::on_horizontalSliderContraste_sliderMoved(int value) {
  m_videoCaptureHandler->setContrast(value);
}
void MainWindow::on_horizontalSliderSaturacion_sliderMoved(int value) {
  m_videoCaptureHandler->setSaturation(value);
}
void MainWindow::on_horizontalSliderNitidez_sliderMoved(int value) {
  m_videoCaptureHandler->setSharpness(value);
}
void MainWindow::on_checkBoxExposicionAuto_toggled(bool checked) {
  m_videoCaptureHandler->setAutoExposure(checked);
  ui->horizontalSliderExposicionAuto->setEnabled(m_support.exposure && !checked);
}
void MainWindow::on_horizontalSliderExposicionAuto_sliderMoved(int value) {
  m_videoCaptureHandler->setExposure(value);
}
void MainWindow::setAllControlsEnabled(bool enabled) {
  ui->checkBoxFocoAuto->setEnabled(enabled);
  ui->horizontalSliderFocoAuto->setEnabled(enabled);
  ui->horizontalSliderBrillo->setEnabled(enabled);
  ui->horizontalSliderContraste->setEnabled(enabled);
  ui->horizontalSliderSaturacion->setEnabled(enabled);
  ui->horizontalSliderNitidez->setEnabled(enabled);
  ui->checkBoxExposicionAuto->setEnabled(enabled);
  ui->horizontalSliderExposicionAuto->setEnabled(enabled);

  if (!enabled) {
    ui->checkBoxExposicionAuto->setChecked(false);
    ui->checkBoxFocoAuto->setChecked(false);
  }
}

// ... (resizeEvent y updateVideoLabel sin cambios) ...
void MainWindow::resizeEvent(QResizeEvent *event) {
  QMainWindow::resizeEvent(event);
  updateVideoLabel();
}
void MainWindow::updateVideoLabel() {
  if (m_currentPixmap.isNull()) {
    return;
  }
  ui->videoLabel->setPixmap(m_currentPixmap.scaled(
      ui->videoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

// --- NUEVO: Helper para parsear la resolución ---
QSize MainWindow::parseResolution(const QString &text) {
  if (text == "Default") {
    // 0,0 significará "default" para el hilo
    return QSize(0, 0);
  }
  QStringList parts = text.split('x');
  if (parts.size() == 2) {
    bool ok1, ok2;
    int w = parts[0].toInt(&ok1);
    int h = parts[1].toInt(&ok2);
    if (ok1 && ok2) {
      return QSize(w, h);
    }
  }
  return QSize(0, 0); // Fallback a default
}