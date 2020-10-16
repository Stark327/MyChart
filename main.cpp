#include "mainwindow.h"
#include <QApplication>

#include <QDebug>
#include "qlogging.h"
#include <iostream>
#include <QUdpSocket>
#include <QtCharts>
#include <qchart.h>

#define SERIES_SIZE         3000

#define SAMPLE_RATE         1000.0f

#define LINE1  1
#define LINE2  1
#define LINE3  1

using namespace std;
using namespace QtCharts;
int main(int argc, char *argv[])
{

    int currentExitCode = 0;
  do{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle(QString("Oscilloscope1"));
    w.setApp(&a);
    QChart *chart = new QChart();

    QUdpSocket *socket = new QUdpSocket();
    w.setSocket(socket);
    socket->bind(QHostAddress(QHostAddress("192.168.1.48")),50001);
    socket->writeDatagram("hello",QHostAddress(QHostAddress::Broadcast),50000);//先向外广播请求网络权限

    QLineSeries *series = new QLineSeries();
    QLineSeries *filterdSeries = new QLineSeries();
    QLineSeries *series3 = new QLineSeries();
    QByteArray array;

    QValueAxis *axisX = new QValueAxis;
    axisX->setTickCount(3);
    axisX->setLabelFormat("%f");
    axisX->setTitleText("T/s");
    axisX->setRange(0,SERIES_SIZE/SAMPLE_RATE);
    chart->addAxis(axisX, Qt::AlignBottom);

    // Y轴属性
    QValueAxis *axisY = new QValueAxis;
    axisY->setTickCount(13);
    axisY->setLabelFormat("%f");
    axisY->setRange(0,60);
    axisY->setTitleText("Current/A");
    chart->addAxis(axisY, Qt::AlignLeft);

#if LINE1
    QString name("母线电流");
    series->setName(name);
    for(int i=0; i < SERIES_SIZE; i++)
    {
        series->append(i/SAMPLE_RATE, (double)i*0.0001);
    }
    chart->addSeries(series);
    chart->setAxisX(axisX,series);
    chart->setAxisY(axisY,series);
#endif
#if LINE2
    QString name2("刹车电阻温度");
    filterdSeries->setName(name2);
    for(int i=0; i < SERIES_SIZE/5; i++)
    {
        filterdSeries->append(i/SAMPLE_RATE*5, (double)i*0.0001);
    }
    chart->addSeries(filterdSeries);
    chart->setAxisX(axisX,filterdSeries);
    chart->setAxisY(axisY,filterdSeries);
#endif
#if LINE3
    QString name3("pwm值\r\n");
    series3->setName(name3);
    for(int i=0; i < SERIES_SIZE/5; i++)
    {
        series3->append(i/SAMPLE_RATE*5.f, (double)i*0.0001);
    }
    chart->addSeries(series3);
    chart->setAxisX(axisX,series3);
    chart->setAxisY(axisY,series3);
#endif

    chart->setTitle("电源管理");
    QFont font;
    font.setPointSize(16);
    chart->setTitleFont(font);


    // 添加点集，坐标轴,配置窗体显示

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    w.setCentralWidget(chartView);
    w.show();

    // 主循环

    float analog1, analog2, analog3, analog4, analog5;
    int length, length2, length3;

    float temp, pwm, Ubat, Imean;
    QByteArray analog_1;
    QByteArray analog_2;
    QByteArray analog;
    int marked_flag = 0;

    QVector<QPointF> oldPoints;
    QVector<QPointF> points;
    QVector<QPointF> oldPoints2;
    QVector<QPointF> points2;
    QVector<QPointF> oldPoints3;
    QVector<QPointF> points3;

    while(!w.getout_flag)
    {
        // 等待数据
        if ((!w.stop_flag) && socket->hasPendingDatagrams())
        {
            array.resize(socket->pendingDatagramSize());
            socket->readDatagram(array.data(), array.size());

            analog_1[0] = array[3];
            analog_1[1] = array[2];
            analog_1[2] = array[1];
            analog_1[3] = array[0];

            analog_2[0] = array[7];
            analog_2[1] = array[6];
            analog_2[2] = array[5];
            analog_2[3] = array[4];

            analog[0] = array[13];//10-13
            analog[1] = array[12];
            analog[2] = array[11];
            analog[3] = array[10];
            analog1 = *(float *)analog.data();
            analog[0] = array[17];//14-17
            analog[1] = array[16];
            analog[2] = array[15];
            analog[3] = array[14];
            analog2 = *(float *)analog.data();
            analog[0] = array[21];//18-21
            analog[1] = array[20];
            analog[2] = array[19];
            analog[3] = array[18];
            analog3 = *(float *)analog.data();
            analog[0] = array[25];//22-25
            analog[1] = array[24];
            analog[2] = array[23];
            analog[3] = array[22];
            analog4 = *(float *)analog.data();
            analog[0] = array[29];//26-29
            analog[1] = array[28];
            analog[2] = array[27];
            analog[3] = array[26];
            analog5 = *(float *)analog.data();

            temp = (float)(array[8]*0xff + array[9]);
            pwm = (float)(array[30]*0xff + array[31]);
            Ubat = *(float *)analog_1.data();
            Imean = *(float *)analog_2.data();
            // 更新点集

#if LINE1
            oldPoints = series->pointsVector();
            length = oldPoints.count();
            for(int i=1; i <= length -5; ++i){
                points.append(QPointF((i-1)/SAMPLE_RATE ,oldPoints.at(i+4).y()));
            }
            points.append(QPointF((length-5)/SAMPLE_RATE,(double)analog1)); // 注入数据
            points.append(QPointF((length-4)/SAMPLE_RATE,(double)analog2));
            points.append(QPointF((length-3)/SAMPLE_RATE,(double)analog3));
            points.append(QPointF((length-2)/SAMPLE_RATE,(double)analog4));
            points.append(QPointF((length-1)/SAMPLE_RATE,(double)analog5));
            series->replace(points);
            oldPoints.clear();
            points.clear();
            QVector<QPointF>(oldPoints).swap(oldPoints);
            QVector<QPointF>(points).swap(points);
#endif
#if LINE2
            oldPoints2 = filterdSeries->pointsVector();
            length2 = oldPoints2.count();
            for(int i=1; i < length2; i++){
                points2.append(QPointF((i-1)/SAMPLE_RATE*5 ,oldPoints2.at(i).y()));
            }
            points2.append(QPointF((length2-1)/SAMPLE_RATE*5,(double)temp)); // 注入数据
            filterdSeries->replace(points2);
            oldPoints2.clear();
            points2.clear();
            QVector<QPointF>(oldPoints2).swap(oldPoints2);
            QVector<QPointF>(points2).swap(points2);
#endif
#if LINE3
            oldPoints3 = series3->pointsVector();
            length3 = oldPoints3.count();
            for(int i=1; i < length3; i++){
                points3.append(QPointF((i-1)/SAMPLE_RATE*5 ,oldPoints3.at(i).y()));
            }
            points3.append(QPointF((length3-1)/SAMPLE_RATE*5,(double)pwm)); // 注入数据
            series3->replace(points3);
            oldPoints3.clear();
            points3.clear();
            QVector<QPointF>(oldPoints3).swap(oldPoints3);
            QVector<QPointF>(points3).swap(points3);
#endif
        }
        else
        {
            w.usleep(1);
        }
    }
    w.wait_quit = 1;
        currentExitCode = a.exec();
    } while( currentExitCode == EXIT_CODE_REBOOT );
    cout<<"finish"<<endl;

    return 0;
}
