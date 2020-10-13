#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QUdpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

static const int EXIT_CODE_REBOOT = -123456789;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    int getout_flag = 0;
    int stop_flag = 0;
    QApplication *app;
    QUdpSocket *socket;
    int wait_quit = 0;

    void setApp(QApplication *app);
    void usleep(unsigned int msec);
    void Delay_MSec(unsigned int msec);
    void keyPressEvent(QKeyEvent *ev);
    void setSocket(QUdpSocket *socket);

private:
    Ui::MainWindow *ui;
    QPushButton * actionReboot;
};
#endif // MAINWINDOW_H
