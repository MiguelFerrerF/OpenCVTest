#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "videocapturehandler.h"
#include <QCameraDevice>
#include <QMediaDevices>

// --- Includes nuevos (QComboBox ya estaba) ---
#include <QCheckBox>
#include <QComboBox> // Asegurarse de que está
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  ui->videoLabel->setAlignment(Qt::AlignCenter);

  m_videoCaptureHandler = new VideoCaptureHandler(this);

  // ... (Conexión de newPixmapCaptured sin cambios) ...
  connect(
      m_videoCaptureHandler, &VideoCaptureHandler::newPixmapCaptured, this,
      [=](const QPixmap &pixmap) {
        m_currentPixmap = pixmap;
        updateVideoLabel();
      });

  // --- MODIFICADO: Rellenar ComboBox y añadir el de Resolución ---

  // Rellenar ComboBox de cámaras
  QStringList cameraNames;
  for (const QCameraDevice &camera : QMediaDevices::videoInputs()) {
    cameraNames << camera.description();
  }
  ui->comboBoxCameras->addItems(cameraNames);

  // --- NUEVO: Crear ComboBox de Resolución ---
  m_resolutionComboBox = new QComboBox();
  m_resolutionComboBox->addItems(
      {"Default", "640x480", "1280x720", "1920x1080"});
  // Insertarlo en el layout 'botones' entre el selector de cámara y el de foco
  // El layout 'horizontalLayout' dentro de 'botones'
  QHBoxLayout *buttonLayout =
      qobject_cast<QHBoxLayout *>(ui->botones->layout());
  if (buttonLayout) {
    // Insertar en la posición 1 (después del comboBoxCameras que está en la 0)
    buttonLayout->insertWidget(1, m_resolutionComboBox);
  }
  // --- FIN NUEVO ---

  if (cameraNames.isEmpty()) {
    ui->startButton->setEnabled(false);
    ui->videoLabel->setText("No se han detectado cámaras.");
  }

  // --- Controles de Foco (modificado para insertarlos) ---
  m_manualFocusCheckBox = new QCheckBox("Foco Manual");
  m_focusSlider = new QSlider(Qt::Horizontal);
  m_focusSlider->setRange(0, 100);
  if (buttonLayout) {
    // Insertar después del ComboBox de resolución
    buttonLayout->insertWidget(2, m_manualFocusCheckBox);
    buttonLayout->insertWidget(3, m_focusSlider);
  }
  connect(
      m_manualFocusCheckBox, &QCheckBox::toggled, this,
      &MainWindow::on_manualFocus_toggled);
  connect(
      m_focusSlider, &QSlider::valueChanged, this,
      &MainWindow::on_focusSlider_valueChanged);

  // ... (Creación del QGroupBox 'm_settingsGroup' y todos los sliders sin
  // cambios) ... (El código que crea m_settingsGroup, settingsLayout y añade
  // los sliders va aquí)
  m_settingsGroup = new QGroupBox("Ajustes de Cámara");
  QGridLayout *settingsLayout = new QGridLayout();
  m_brightnessLabel = new QLabel("Brillo");
  m_brightnessSlider = new QSlider(Qt::Horizontal);
  m_brightnessSlider->setRange(0, 255);
  m_contrastLabel = new QLabel("Contraste");
  m_contrastSlider = new QSlider(Qt::Horizontal);
  m_contrastSlider->setRange(0, 255);
  m_saturationLabel = new QLabel("Saturación");
  m_saturationSlider = new QSlider(Qt::Horizontal);
  m_saturationSlider->setRange(0, 255);
  m_hueLabel = new QLabel("Tono (Hue)");
  m_hueSlider = new QSlider(Qt::Horizontal);
  m_hueSlider->setRange(0, 255);
  m_gainLabel = new QLabel("Ganancia");
  m_gainSlider = new QSlider(Qt::Horizontal);
  m_gainSlider->setRange(0, 255);
  m_autoExposureCheckBox = new QCheckBox("Exposición Auto");
  m_exposureLabel = new QLabel("Exposición");
  m_exposureSlider = new QSlider(Qt::Horizontal);
  m_exposureSlider->setRange(0, 255);
  settingsLayout->addWidget(m_brightnessLabel, 0, 0);
  settingsLayout->addWidget(m_brightnessSlider, 0, 1);
  settingsLayout->addWidget(m_contrastLabel, 1, 0);
  settingsLayout->addWidget(m_contrastSlider, 1, 1);
  settingsLayout->addWidget(m_saturationLabel, 2, 0);
  settingsLayout->addWidget(m_saturationSlider, 2, 1);
  settingsLayout->addWidget(m_hueLabel, 3, 0);
  settingsLayout->addWidget(m_hueSlider, 3, 1);
  settingsLayout->addWidget(m_gainLabel, 4, 0);
  settingsLayout->addWidget(m_gainSlider, 4, 1);
  settingsLayout->addWidget(m_autoExposureCheckBox, 5, 0);
  settingsLayout->addWidget(m_exposureLabel, 6, 0);
  settingsLayout->addWidget(m_exposureSlider, 6, 1);
  m_settingsGroup->setLayout(settingsLayout);
  ui->verticalLayout->insertWidget(
      ui->verticalLayout->count() - 1, m_settingsGroup);
  connect(
      m_brightnessSlider, &QSlider::valueChanged, this,
      &MainWindow::on_brightness_changed);
  connect(
      m_contrastSlider, &QSlider::valueChanged, this,
      &MainWindow::on_contrast_changed);
  connect(
      m_saturationSlider, &QSlider::valueChanged, this,
      &MainWindow::on_saturation_changed);
  connect(
      m_hueSlider, &QSlider::valueChanged, this, &MainWindow::on_hue_changed);
  connect(
      m_gainSlider, &QSlider::valueChanged, this, &MainWindow::on_gain_changed);
  connect(
      m_autoExposureCheckBox, &QCheckBox::toggled, this,
      &MainWindow::on_autoExposure_toggled);
  connect(
      m_exposureSlider, &QSlider::valueChanged, this,
      &MainWindow::on_exposure_changed);
  connect(
      m_videoCaptureHandler, &VideoCaptureHandler::propertiesSupported, this,
      &MainWindow::on_propertiesSupported);
  setAllControlsEnabled(false);
  // --- FIN ZONA SIN CAMBIOS ---

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
    QString resText = m_resolutionComboBox->currentText();
    QSize resolution = parseResolution(resText);
    // --- FIN NUEVO ---

    // --- MODIFICADO: Enviar ID y Resolución ---
    m_videoCaptureHandler->requestCameraChange(cameraId, resolution);

    ui->startButton->setText("Stop");
    ui->comboBoxCameras->setEnabled(false);
    m_resolutionComboBox->setEnabled(false); // --- NUEVO ---

  } else {
    // Estado: OFF (Detener)
    // --- MODIFICADO: Enviar -1 y QSize() vacío ---
    m_videoCaptureHandler->requestCameraChange(-1, QSize());

    ui->startButton->setText("Start OpenCV");
    ui->comboBoxCameras->setEnabled(true);
    m_resolutionComboBox->setEnabled(true); // --- NUEVO ---

    m_currentPixmap = QPixmap();
    ui->videoLabel->clear();
    ui->videoLabel->setText("Cámara detenida.");

    setAllControlsEnabled(false);
  }
}

// ... (Slots de foco y propiedades sin cambios) ...
void MainWindow::on_manualFocus_toggled(bool checked) {
  m_videoCaptureHandler->setManualFocus(checked);
  m_focusSlider->setEnabled(m_support.brightness && checked);
}
void MainWindow::on_focusSlider_valueChanged(int value) {
  m_videoCaptureHandler->setFocusValue(value);
}
void MainWindow::on_propertiesSupported(CameraPropertiesSupport support) {
  m_support = support;
  m_manualFocusCheckBox->setEnabled(support.brightness);
  m_focusSlider->setEnabled(
      support.brightness && m_manualFocusCheckBox->isChecked());
  m_brightnessLabel->setEnabled(support.brightness);
  m_brightnessSlider->setEnabled(support.brightness);
  m_contrastLabel->setEnabled(support.contrast);
  m_contrastSlider->setEnabled(support.contrast);
  m_saturationLabel->setEnabled(support.saturation);
  m_saturationSlider->setEnabled(support.saturation);
  m_hueLabel->setEnabled(support.hue);
  m_hueSlider->setEnabled(support.hue);
  m_gainLabel->setEnabled(support.gain);
  m_gainSlider->setEnabled(support.gain);
  m_autoExposureCheckBox->setEnabled(support.autoExposure);
  m_exposureLabel->setEnabled(support.exposure);
  m_exposureSlider->setEnabled(
      support.exposure && !m_autoExposureCheckBox->isChecked());
}
void MainWindow::on_brightness_changed(int value) {
  m_videoCaptureHandler->setBrightness(value);
}
void MainWindow::on_contrast_changed(int value) {
  m_videoCaptureHandler->setContrast(value);
}
void MainWindow::on_saturation_changed(int value) {
  m_videoCaptureHandler->setSaturation(value);
}
void MainWindow::on_hue_changed(int value) {
  m_videoCaptureHandler->setHue(value);
}
void MainWindow::on_gain_changed(int value) {
  m_videoCaptureHandler->setGain(value);
}
void MainWindow::on_autoExposure_toggled(bool checked) {
  m_videoCaptureHandler->setAutoExposure(checked);
  m_exposureSlider->setEnabled(m_support.exposure && !checked);
}
void MainWindow::on_exposure_changed(int value) {
  m_videoCaptureHandler->setExposure(value);
}
void MainWindow::setAllControlsEnabled(bool enabled) {
  m_manualFocusCheckBox->setEnabled(enabled);
  m_focusSlider->setEnabled(enabled);
  m_settingsGroup->setEnabled(enabled);
  m_brightnessLabel->setEnabled(enabled);
  m_brightnessSlider->setEnabled(enabled);
  m_contrastLabel->setEnabled(enabled);
  m_contrastSlider->setEnabled(enabled);
  m_saturationLabel->setEnabled(enabled);
  m_saturationSlider->setEnabled(enabled);
  m_hueLabel->setEnabled(enabled);
  m_hueSlider->setEnabled(enabled);
  m_gainLabel->setEnabled(enabled);
  m_gainSlider->setEnabled(enabled);
  m_autoExposureCheckBox->setEnabled(enabled);
  m_exposureLabel->setEnabled(enabled);
  m_exposureSlider->setEnabled(enabled);

  if (!enabled) {
    m_autoExposureCheckBox->setChecked(false);
    m_manualFocusCheckBox->setChecked(false);
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