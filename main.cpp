#include "mainwindow.h"
#include <QApplication>

#include <QDebug>
#include "qlogging.h"
#include <iostream>
#include <QUdpSocket>
#include <QtCharts>

#define SERIES_SIZE         2000

#define SAMPLE_RATE         200.0f

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

    QUdpSocket *socket = new QUdpSocket();
    w.setSocket(socket);
    socket->bind(QHostAddress(QHostAddress("192.168.1.48")),50001);
    socket->writeDatagram("hello",QHostAddress(QHostAddress::Broadcast),50000);//先向外广播请求网络权限

    QLineSeries *series = new QLineSeries();
    QLineSeries *filterdSeries = new QLineSeries();
    QByteArray array;

    // 初始化赋值
    for(int i=0; i < SERIES_SIZE; i++)
    {
        series->append(i/SAMPLE_RATE, (double)i*0.0001);
    }
    for(int i=0; i < SERIES_SIZE; i++)
    {
        filterdSeries->append(i/SAMPLE_RATE, (double)i*0.0001);
    }

    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->setTitle("电源管理");
    QFont font;
    font.setPointSize(16);
    chart->setTitleFont(font);


    // X轴属性
    QValueAxis *axisX = new QValueAxis;
    axisX->setTickCount((int)(SERIES_SIZE/SAMPLE_RATE)+1); //四根线3s
    axisX->setLabelFormat("%f");
    axisX->setTitleText("T/s");
    axisX->setRange(0,SERIES_SIZE/SAMPLE_RATE);
    chart->addAxis(axisX, Qt::AlignBottom);

    // Y轴属性
    QValueAxis *axisY = new QValueAxis;
    axisY->setTickCount(13);
    axisY->setLabelFormat("%f");
    axisY->setRange(0,15);
    axisY->setTitleText("Current/A");
    chart->addAxis(axisY, Qt::AlignLeft);

    // 添加点集，坐标轴,配置窗体显示
    chart->addSeries(series);
    chart->addSeries(filterdSeries);

    chart->setAxisX(axisX,series);
    chart->setAxisY(axisY,series);
    chart->setAxisX(axisX,filterdSeries);
    chart->setAxisY(axisY,filterdSeries);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    w.setCentralWidget(chartView);
    w.show();

    // 主循环
    QByteArray float4;
    QByteArray float4_filter;
    while(!w.getout_flag)
    {
        // 等待数据
        if ((!w.stop_flag) && socket->hasPendingDatagrams())
        {
            array.resize(socket->pendingDatagramSize());
            socket->readDatagram(array.data(), array.size());

            float4[0] = array[3];
            float4[1] = array[2];
            float4[2] = array[1];
            float4[3] = array[0];

            float4_filter[0] = array[7];
            float4_filter[1] = array[6];
            float4_filter[2] = array[5];
            float4_filter[3] = array[4];

            float pwm = (float)(array[8]*0xff + array[9]);

            float current = *(float *)float4.data();
            float current_filter = *(float *)float4_filter.data();
            // 更新点集
            QVector<QPointF> oldPoints = series->pointsVector();
            QVector<QPointF> points;

            int length = oldPoints.count();
            for(int i=1; i < length; ++i){
                points.append(QPointF((i-1)/SAMPLE_RATE ,oldPoints.at(i).y()));
            }
            points.append(QPointF((length-1)/SAMPLE_RATE,(double)current)); // 注入数据
            series->replace(points);

            // 更新点集
            QVector<QPointF> oldPoints2 = filterdSeries->pointsVector();
            QVector<QPointF> points2;

            int length2 = oldPoints2.count();
            for(int i=1; i < length2; ++i){
                points2.append(QPointF((i-1)/SAMPLE_RATE ,oldPoints2.at(i).y()));
            }
            points2.append(QPointF((length-1)/SAMPLE_RATE,(double)current_filter/*pwm*/)); // 注入数据
            filterdSeries->replace(points2);

            // 清除点集内存
            oldPoints2.clear();
            oldPoints.clear();
            points.clear();
            points2.clear();
            QVector<QPointF>(oldPoints2).swap(oldPoints2);
            QVector<QPointF>(oldPoints).swap(oldPoints);
            QVector<QPointF>(points).swap(points);
            QVector<QPointF>(points2).swap(points2);
            //w.usleep(1);
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
