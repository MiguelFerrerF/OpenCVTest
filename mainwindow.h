#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "videocapturehandler.h"
#include <QMainWindow>
#include <QPixmap>
#include <QResizeEvent>
#include <QSize> // --- NUEVO ---

class QCheckBox;
class QSlider;
class QLabel;
class QGroupBox;
class QComboBox; // --- NUEVO ---

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

  // Slots de Foco
  void on_manualFocus_toggled(bool checked);
  void on_focusSlider_valueChanged(int value);

  // Slot para recibir la info de soporte
  void on_propertiesSupported(CameraPropertiesSupport support);

  // Slots para los nuevos controles
  void on_brightness_changed(int value);
  void on_contrast_changed(int value);
  void on_saturation_changed(int value);
  void on_hue_changed(int value);
  void on_gain_changed(int value);
  void on_autoExposure_toggled(bool checked);
  void on_exposure_changed(int value);

private:
  Ui::MainWindow *ui;
  VideoCaptureHandler *m_videoCaptureHandler;

  QPixmap m_currentPixmap;

  CameraPropertiesSupport m_support;

  // --- NUEVO ---
  QComboBox *m_resolutionComboBox;
  // --- FIN NUEVO ---

  // Controles de Foco
  QCheckBox *m_manualFocusCheckBox;
  QSlider *m_focusSlider;

  // Contenedor para nuevos controles
  QGroupBox *m_settingsGroup;

  // ... (Punteros a los demás sliders y labels sin cambios) ...
  QLabel *m_brightnessLabel;
  QSlider *m_brightnessSlider;
  QLabel *m_contrastLabel;
  QSlider *m_contrastSlider;
  QLabel *m_saturationLabel;
  QSlider *m_saturationSlider;
  QLabel *m_hueLabel;
  QSlider *m_hueSlider;
  QLabel *m_gainLabel;
  QSlider *m_gainSlider;
  QCheckBox *m_autoExposureCheckBox;
  QLabel *m_exposureLabel;
  QSlider *m_exposureSlider;

  void updateVideoLabel();

  void setAllControlsEnabled(bool enabled);

  // --- NUEVO ---
  // Helper para convertir el texto a QSize
  QSize parseResolution(const QString &text);
  // --- FIN NUEVO ---
};
#endif // MAINWINDOW_H