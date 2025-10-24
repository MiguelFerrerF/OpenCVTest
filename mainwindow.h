#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>      // Añadir
#include <QResizeEvent> // Añadir

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
  // Detecta cuándo se redimensiona la ventana
  void resizeEvent(QResizeEvent *event) override;

private slots:
  void on_startButton_clicked();

private:
  Ui::MainWindow *ui;
  VideoCaptureHandler *m_videoCaptureHandler;

  // Variable para guardar el frame original sin escalar
  QPixmap m_currentPixmap;

  // Función helper para dibujar/redibujar la imagen
  void updateVideoLabel();
};
#endif // MAINWINDOW_H