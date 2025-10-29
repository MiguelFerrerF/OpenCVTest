#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "videocapturehandler.h"
#include <QMainWindow>
#include <QPixmap>
#include <QResizeEvent>
#include <QSize> // --- SIN CAMBIOS ---

class QCheckBox;
class QSlider;
class QLabel;
class QGroupBox;
class QComboBox; // --- SIN CAMBIOS ---

namespace Ui {
class MainWindow;
}

class VideoCaptureHandler;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

protected:
  void resizeEvent(QResizeEvent *event) override;

private slots:
  void on_startButton_clicked();
  void on_resetButton_clicked();

  // Slots de Foco
  void on_checkBoxFocoAuto_toggled(bool checked);
  void on_horizontalSliderFocoAuto_sliderMoved(int value);

  // Slot para recibir la info de soporte
  void on_propertiesSupported(CameraPropertiesSupport support);

  // --- NUEVO: Slots para rangos y errores ---
  void on_rangesSupported(const CameraPropertyRanges &ranges);
  void on_cameraOpenFailed(int cameraId, const QString &errorMsg);
  // --- FIN NUEVO ---

  // Slots para los nuevos controles
  void on_horizontalSliderBrillo_sliderMoved(int value);
  void on_horizontalSliderContraste_sliderMoved(int value);
  void on_horizontalSliderSaturacion_sliderMoved(int value);
  void on_horizontalSliderNitidez_sliderMoved(int value);
  void on_checkBoxExposicionAuto_toggled(bool checked);
  void on_horizontalSliderExposicionAuto_sliderMoved(int value);

private:
  Ui::MainWindow *ui;
  VideoCaptureHandler *m_videoCaptureHandler;

  QPixmap m_currentPixmap;

  CameraPropertiesSupport m_support;
  CameraPropertyRanges m_ranges;

  void updateVideoLabel();

  void setAllControlsEnabled(bool enabled);

  // Helper para convertir el texto a QSize
  QSize parseResolution(const QString &text);

  int mapSliderToOpenCV(int sliderValue, const PropertyRange &range);
  int mapOpenCVToSlider(double openCVValue, const PropertyRange &range);
};
#endif // MAINWINDOW_H