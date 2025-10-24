#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "videocapturehandler.h"
#include <QCameraDevice>
#include <QMediaDevices>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  // --- ¡NUEVO! ---
  // 1. Centra el contenido del QLabel (tanto imagen como texto)
  ui->videoLabel->setAlignment(Qt::AlignCenter);

  m_videoCaptureHandler = new VideoCaptureHandler(this);

  // --- MODIFICADO ---
  // La conexión ahora guarda el pixmap y llama a la función de dibujado
  connect(
      m_videoCaptureHandler, &VideoCaptureHandler::newPixmapCaptured, this,
      [=](const QPixmap &pixmap) {
        // 1. Guarda el frame original
        m_currentPixmap = pixmap;
        // 2. Llama a la función que lo escala y lo muestra
        updateVideoLabel();
      });

  // Rellenar el comboBox con las cámaras disponibles
  QStringList cameraNames;
  for (const QCameraDevice &camera : QMediaDevices::videoInputs()) {
    cameraNames << camera.description();
  }
  ui->comboBoxCameras->addItems(cameraNames);

  // Si no hay cámaras, mostrar aviso
  if (cameraNames.isEmpty()) {
    ui->startButton->setEnabled(false);
    ui->videoLabel->setText("No se han detectado cámaras.");
  }

  // Iniciar el hilo UNA SOLA VEZ al crear la ventana
  m_videoCaptureHandler->start(QThread::HighestPriority);
}

MainWindow::~MainWindow() {
  // Cierre seguro del hilo
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
  } else {
    // Estado: OFF (Detener)
    m_videoCaptureHandler->setCamera(-1);
    ui->startButton->setText("Start OpenCV");
    ui->comboBoxCameras->setEnabled(true);

    // --- ¡NUEVO! ---
    // 1. Borra la imagen guardada
    m_currentPixmap = QPixmap();
    // 2. Borra la imagen del label (esto cumple tu petición)
    ui->videoLabel->clear();
    // 3. Muestra el texto de cámara detenida
    ui->videoLabel->setText("Cámara detenida.");
  }
}

// --- ¡NUEVO! ---
// Este evento se llama automáticamente cada vez que la ventana cambia de tamaño
void MainWindow::resizeEvent(QResizeEvent *event) {
  QMainWindow::resizeEvent(event); // Pasa el evento a la clase base
  updateVideoLabel(); // Llama a nuestra función para reescalar la imagen
}

// --- ¡NUEVO! ---
// Esta función centraliza la lógica de escalado y dibujado
void MainWindow::updateVideoLabel() {
  // Si no hay imagen (porque se pulsó Stop), no hagas nada
  if (m_currentPixmap.isNull()) {
    return;
  }

  // Escala la imagen guardada (m_currentPixmap) al tamaño actual del label,
  // manteniendo la relación de aspecto y con escalado suave.
  ui->videoLabel->setPixmap(m_currentPixmap.scaled(
      ui->videoLabel->size(), // Escala al tamaño actual del widget
      Qt::KeepAspectRatio, Qt::SmoothTransformation));
}