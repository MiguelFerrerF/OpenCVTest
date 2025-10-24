#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "videocapturehandler.h"
#include <QCameraDevice>
#include <QMediaDevices>

// --- NUEVO ---
#include <QCheckBox>
#include <QHBoxLayout> // Para añadir widgets al layout del .ui
#include <QSlider>

// --- FIN NUEVO ---

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  ui->videoLabel->setAlignment(Qt::AlignCenter);

  m_videoCaptureHandler = new VideoCaptureHandler(this);

  connect(
      m_videoCaptureHandler, &VideoCaptureHandler::newPixmapCaptured, this,
      [=](const QPixmap &pixmap) {
        m_currentPixmap = pixmap;
        updateVideoLabel();
      });

  // Rellenar el comboBox con las cámaras disponibles
  QStringList cameraNames;
  for (const QCameraDevice &camera : QMediaDevices::videoInputs()) {
    cameraNames << camera.description();
  }
  ui->comboBoxCameras->addItems(cameraNames);

  if (cameraNames.isEmpty()) {
    ui->startButton->setEnabled(false);
    ui->videoLabel->setText("No se han detectado cámaras.");
  }

  // --- NUEVO: Crear y añadir controles de foco ---

  // 1. Crear los widgets
  m_manualFocusCheckBox = new QCheckBox("Foco Manual");
  m_manualFocusCheckBox->setEnabled(false); // Deshabilitado hasta pulsar Start

  m_focusSlider = new QSlider(Qt::Horizontal);
  m_focusSlider->setRange(0, 100);  // Rango 0-100 como pediste
  m_focusSlider->setEnabled(false); // Deshabilitado por defecto

  // 2. Añadirlos al layout horizontal existente en el .ui
  //    (Asumiendo que el layout en el QGroupBox 'botones' se llama
  //    'horizontalLayout')
  ui->botones->layout()->addWidget(m_manualFocusCheckBox);
  ui->botones->layout()->addWidget(m_focusSlider);

  // 3. Conectar las señales
  connect(
      m_manualFocusCheckBox, &QCheckBox::toggled, this,
      &MainWindow::on_manualFocus_toggled);
  connect(
      m_focusSlider, &QSlider::valueChanged, this,
      &MainWindow::on_focusSlider_valueChanged);
  // --- FIN NUEVO ---

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
    m_videoCaptureHandler->setCamera(cameraId);
    ui->startButton->setText("Stop");
    ui->comboBoxCameras->setEnabled(false);

    // --- NUEVO ---
    // Habilitar controles de foco
    m_manualFocusCheckBox->setEnabled(true);
    // El slider solo se activa si el check ya estaba (o está) marcado
    m_focusSlider->setEnabled(m_manualFocusCheckBox->isChecked());
    // --- FIN NUEVO ---

  } else {
    // Estado: OFF (Detener)
    m_videoCaptureHandler->setCamera(-1);
    ui->startButton->setText("Start OpenCV");
    ui->comboBoxCameras->setEnabled(true);

    m_currentPixmap = QPixmap();
    ui->videoLabel->clear();
    ui->videoLabel->setText("Cámara detenida.");

    // --- NUEVO ---
    // Deshabilitar y resetear controles de foco
    m_videoCaptureHandler->setManualFocus(false); // Volver a auto-foco
    m_manualFocusCheckBox->setChecked(false);
    m_manualFocusCheckBox->setEnabled(false);
    m_focusSlider->setEnabled(false);
    // --- FIN NUEVO ---
  }
}

// --- NUEVO ---
void MainWindow::on_manualFocus_toggled(bool checked) {
  // checked == true -> Modo Manual
  // checked == false -> Modo Automático

  // Habilita el slider SÓLO si el modo es manual
  m_focusSlider->setEnabled(checked);

  // Informa al hilo de captura sobre el cambio
  m_videoCaptureHandler->setManualFocus(checked);

  // Si volvemos a automático, reseteamos el valor del slider
  if (!checked) {
    m_focusSlider->setValue(0);
  } else {
    // Si pasamos a manual, enviamos el valor actual del slider
    m_videoCaptureHandler->setFocusValue(m_focusSlider->value());
  }
}

void MainWindow::on_focusSlider_valueChanged(int value) {
  // Informa al hilo de captura sobre el nuevo valor de foco
  // No es necesario comprobar si está en modo manual,
  // el hilo se encargará de ignorar el valor si está en automático.
  m_videoCaptureHandler->setFocusValue(value);
}
// --- FIN NUEVO ---

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