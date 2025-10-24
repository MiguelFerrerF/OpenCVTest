#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QResizeEvent>

// --- NUEVO ---
// Declaraciones anticipadas de las nuevas clases de UI
class QCheckBox;
class QSlider;
// --- FIN NUEVO ---

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

  // --- NUEVO ---
  // Slots para los nuevos controles de foco
  void on_manualFocus_toggled(bool checked);
  void on_focusSlider_valueChanged(int value);
  // --- FIN NUEVO ---

private:
  Ui::MainWindow *ui;
  VideoCaptureHandler *m_videoCaptureHandler;

  QPixmap m_currentPixmap;

  // --- NUEVO ---
  // Punteros a los nuevos widgets
  QCheckBox *m_manualFocusCheckBox;
  QSlider *m_focusSlider;
  // --- FIN NUEVO ---

  void updateVideoLabel();
};
#endif // MAINWINDOW_H