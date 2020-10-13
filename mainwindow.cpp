#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qdatetime.h"
#include <QDebug>
#include <QTimer>
#include "qevent.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    this->getout_flag = 1;
    this->app->quit();
    delete ui;
}

void MainWindow::usleep(unsigned int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);

    while (QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }

}

void MainWindow::Delay_MSec(unsigned int msec)
{
   QEventLoop loop;//定义一个新的事件循环
   QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
   loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_P)
    {
       this->stop_flag = !this->stop_flag;
       return;
    }
    if(ev->key() == Qt::Key_R)
    {
        this->socket->abort();
       this->getout_flag = !this->getout_flag;
       qApp->exit( EXIT_CODE_REBOOT );
       return;
    }
    if(ev->key() == Qt::Key_C)
    {
       this->getout_flag = 1;
        if(this->wait_quit == 1)
        {
            this->socket->abort();
            this->close();
            this->app->quit();
        }
    }
}

void MainWindow::setApp(QApplication *appl)
{
    this->app = appl;
}

void MainWindow::setSocket(QUdpSocket *qsocket)
{
    this->socket = qsocket;
}
