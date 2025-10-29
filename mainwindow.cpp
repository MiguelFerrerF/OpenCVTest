#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "videocapturehandler.h"
#include <QCameraDevice>
#include <QMediaDevices>
#include <QMessageBox>

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

  // --- NUEVO: Conectar señales de rangos y errores ---
  connect(
      m_videoCaptureHandler, &VideoCaptureHandler::propertiesSupported, this,
      &MainWindow::on_propertiesSupported);
  connect(
      m_videoCaptureHandler, &VideoCaptureHandler::rangesSupported, this,
      &MainWindow::on_rangesSupported);
  connect(
      m_videoCaptureHandler, &VideoCaptureHandler::cameraOpenFailed, this,
      &MainWindow::on_cameraOpenFailed);
  // --- FIN NUEVO ---

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

void MainWindow::on_startButton_clicked() {
  if (ui->startButton->isChecked()) {
    // Estado: ON (Iniciar)
    int cameraId = ui->comboBoxCameras->currentIndex();

    QString resText = ui->comboBoxResolution->currentText();
    QSize resolution = parseResolution(resText);

    m_videoCaptureHandler->requestCameraChange(cameraId, resolution);

    ui->startButton->setText("Stop");
    ui->comboBoxCameras->setEnabled(false);
    ui->comboBoxResolution->setEnabled(false);
  } else {
    // Estado: OFF (Detener)
    m_videoCaptureHandler->requestCameraChange(-1, QSize());

    ui->startButton->setText("Start OpenCV");
    ui->comboBoxCameras->setEnabled(true);
    ui->comboBoxResolution->setEnabled(true);

    m_currentPixmap = QPixmap();
    ui->videoLabel->clear();
    ui->videoLabel->setText("Cámara detenida.");
  }
}

void MainWindow::on_resetButton_clicked() {
  // Reiniciar todos los controles a sus valores predeterminados
  ui->checkBoxFocoAuto->setChecked(true);
  on_checkBoxFocoAuto_toggled(true);
  ui->checkBoxExposicionAuto->setChecked(true);
  on_checkBoxExposicionAuto_toggled(true);
  ui->horizontalSliderBrillo->setValue(50);
  on_horizontalSliderBrillo_sliderMoved(50);
  ui->horizontalSliderContraste->setValue(50);
  on_horizontalSliderContraste_sliderMoved(50);
  ui->horizontalSliderSaturacion->setValue(50);
  on_horizontalSliderSaturacion_sliderMoved(50);
  ui->horizontalSliderNitidez->setValue(50);
  on_horizontalSliderNitidez_sliderMoved(50);
}

void MainWindow::on_cameraOpenFailed(int cameraId, const QString &errorMsg) {
  Q_UNUSED(cameraId);
  QMessageBox::critical(
      this, "Error de Cámara",
      tr("No se pudo iniciar la cámara seleccionada. Detalle: %1").arg(errorMsg));

  // Resetear el botón de inicio/parada y habilitar la selección de cámara
  ui->startButton->setChecked(false);
  ui->startButton->setText("Start OpenCV");
  ui->comboBoxCameras->setEnabled(true);
  ui->comboBoxResolution->setEnabled(true);
}

void MainWindow::on_rangesSupported(const CameraPropertyRanges &ranges) {
  m_ranges = ranges;

  // Configurar los Sliders (escala de 0 a 100 para la GUI)
  // ... (Lógica de configuración de rangos y valores sin cambios) ...

  // Brillo
  ui->horizontalSliderBrillo->setRange(0, 100);
  ui->horizontalSliderBrillo->setValue(
      qBound(0, mapOpenCVToSlider(ranges.brightness.current, ranges.brightness), 100));

  // Contraste
  ui->horizontalSliderContraste->setRange(0, 100);
  ui->horizontalSliderContraste->setValue(
      qBound(0, mapOpenCVToSlider(ranges.contrast.current, ranges.contrast), 100));

  // Saturación
  ui->horizontalSliderSaturacion->setRange(0, 100);
  ui->horizontalSliderSaturacion->setValue(
      qBound(0, mapOpenCVToSlider(ranges.saturation.current, ranges.saturation), 100));

  // Nitidez
  ui->horizontalSliderNitidez->setRange(0, 100);
  ui->horizontalSliderNitidez->setValue(
      qBound(0, mapOpenCVToSlider(ranges.sharpness.current, ranges.sharpness), 100));

  // Exposición
  ui->horizontalSliderExposicionAuto->setRange(0, 100);
  ui->horizontalSliderExposicionAuto->setValue(
      qBound(0, mapOpenCVToSlider(ranges.exposure.current, ranges.exposure), 100));

  // Foco
  ui->horizontalSliderFocoAuto->setRange(0, 100);
  ui->horizontalSliderFocoAuto->setValue(
      qBound(0, mapOpenCVToSlider(ranges.focus.current, ranges.focus), 100));

  // --- NUEVA LÓGICA DE HABILITACIÓN AQUÍ ---
  // 1. Habilitar/Deshabilitar todos los controles según el soporte real de la cámara.
  // Usamos m_support, que ya se actualizó en on_propertiesSupported, y que es la información de
  // si la prop. está disponible.
  ui->checkBoxFocoAuto->setEnabled(m_support.autoFocus);
  ui->horizontalSliderBrillo->setEnabled(m_support.brightness);
  ui->horizontalSliderContraste->setEnabled(m_support.contrast);
  ui->horizontalSliderSaturacion->setEnabled(m_support.saturation);
  ui->horizontalSliderNitidez->setEnabled(m_support.sharpness);
  ui->checkBoxExposicionAuto->setEnabled(m_support.autoExposure);

  ui->horizontalSliderFocoAuto->setEnabled(m_support.focus && !ui->checkBoxFocoAuto->isChecked());
  ui->horizontalSliderExposicionAuto->setEnabled(
      m_support.exposure && !ui->checkBoxExposicionAuto->isChecked());
}
// nuevo slot)
void MainWindow::on_propertiesSupported(CameraPropertiesSupport support) { m_support = support; }

// Slots de Foco
void MainWindow::on_checkBoxFocoAuto_toggled(bool checked) {
  m_videoCaptureHandler->setAutoFocus(checked);
  ui->horizontalSliderFocoAuto->setEnabled(m_support.focus && !checked);
}

void MainWindow::on_checkBoxExposicionAuto_toggled(bool checked) {
  m_videoCaptureHandler->setAutoExposure(checked);
  ui->horizontalSliderExposicionAuto->setEnabled(m_support.exposure && !checked);
}

void MainWindow::on_horizontalSliderFocoAuto_sliderMoved(int value) {
  int openCVValue = mapSliderToOpenCV(value, m_ranges.focus);
  m_videoCaptureHandler->setFocus(openCVValue);
}

void MainWindow::on_horizontalSliderBrillo_sliderMoved(int value) {
  int openCVValue = mapSliderToOpenCV(value, m_ranges.brightness);
  m_videoCaptureHandler->setBrightness(openCVValue);
}

void MainWindow::on_horizontalSliderContraste_sliderMoved(int value) {
  int openCVValue = mapSliderToOpenCV(value, m_ranges.contrast);
  m_videoCaptureHandler->setContrast(openCVValue);
}

void MainWindow::on_horizontalSliderSaturacion_sliderMoved(int value) {
  int openCVValue = mapSliderToOpenCV(value, m_ranges.saturation);
  m_videoCaptureHandler->setSaturation(openCVValue);
}

void MainWindow::on_horizontalSliderNitidez_sliderMoved(int value) {
  int openCVValue = mapSliderToOpenCV(value, m_ranges.sharpness);
  m_videoCaptureHandler->setSharpness(openCVValue);
}

void MainWindow::on_horizontalSliderExposicionAuto_sliderMoved(int value) {
  int openCVValue = mapSliderToOpenCV(value, m_ranges.exposure);
  m_videoCaptureHandler->setExposure(openCVValue);
}

void MainWindow::setAllControlsEnabled(bool enabled) {
  // Nota: Esta función ya no verifica m_support, solo el estado general 'enabled'.
  // La habilitación específica de cada control se mueve al slot on_rangesSupported.
  ui->checkBoxFocoAuto->setEnabled(enabled);
  ui->horizontalSliderFocoAuto->setEnabled(enabled);
  ui->horizontalSliderBrillo->setEnabled(enabled);
  ui->horizontalSliderContraste->setEnabled(enabled);
  ui->horizontalSliderSaturacion->setEnabled(enabled);
  ui->horizontalSliderNitidez->setEnabled(enabled);
  ui->checkBoxExposicionAuto->setEnabled(enabled);
  ui->horizontalSliderExposicionAuto->setEnabled(enabled);

  // Aseguramos que los modos automáticos estén activos si la cámara se detiene o se está
  // inicializando
  if (!enabled) {
    ui->checkBoxExposicionAuto->setChecked(true);
    ui->checkBoxFocoAuto->setChecked(true);
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

// Helper para parsear la resolución
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

// --- NUEVO: Helper para mapear valores del Slider (0-100) al rango de OpenCV ---
int MainWindow::mapSliderToOpenCV(int sliderValue, const PropertyRange &range) {
  // Escala de [0, 100] (slider) a [range.min, range.max] (OpenCV)
  double outputRange = range.max - range.min;
  double scaleFactor = outputRange / 100.0;

  // Mapeo lineal: (ValorSlider * FactorEscala) + Mínimo
  double mappedValue = (sliderValue * scaleFactor) + range.min;

  // Asegurar que el valor se mantiene dentro del rango de OpenCV
  return qBound(
      static_cast<int>(range.min), static_cast<int>(mappedValue), static_cast<int>(range.max));
}

// --- NUEVO: Helper para mapear valores de OpenCV al rango del Slider (0-100) ---
int MainWindow::mapOpenCVToSlider(double openCVValue, const PropertyRange &range) {
  // Escala de [range.min, range.max] (OpenCV) a [0, 100] (slider)
  double inputValue = openCVValue - range.min;
  double inputRange = range.max - range.min;

  if (qFuzzyIsNull(inputRange)) {
    return 50; // Evitar división por cero, devolver centro por defecto
  }

  double normalizedValue = inputValue / inputRange;

  // Mapeo al rango de 0 a 100
  int sliderValue = static_cast<int>(normalizedValue * 100.0);

  // Asegurar que el valor se mantiene dentro del rango del slider
  return qBound(0, sliderValue, 100);
}