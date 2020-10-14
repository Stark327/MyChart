#include "mainwindow.h"
#include <QApplication>

#include <QDebug>
#include "qlogging.h"
#include <iostream>
#include <QUdpSocket>
#include <QtCharts>

#define SERIES_SIZE         3000

#define SAMPLE_RATE         1000.0f

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
    for(int i=0; i < SERIES_SIZE/5; i++)
    {
        filterdSeries->append(i/SAMPLE_RATE*5, (double)i*0.0001);
    }

    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->setTitle("电源管理");
    QFont font;
    font.setPointSize(16);
    chart->setTitleFont(font);


    // X轴属性
    QValueAxis *axisX = new QValueAxis;
    axisX->setTickCount((int)(SERIES_SIZE/SAMPLE_RATE)+1);
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
    QByteArray analog_1;
    QByteArray analog_2;
    QByteArray analog;
    int marked_flag = 0;
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
            float analog1 = *(float *)analog.data();
            analog[0] = array[17];//14-17
            analog[1] = array[16];
            analog[2] = array[15];
            analog[3] = array[14];
            float analog2 = *(float *)analog.data();
            analog[0] = array[21];//18-21
            analog[1] = array[20];
            analog[2] = array[19];
            analog[3] = array[18];
            float analog3 = *(float *)analog.data();
            analog[0] = array[25];//22-25
            analog[1] = array[24];
            analog[2] = array[23];
            analog[3] = array[22];
            float analog4 = *(float *)analog.data();
            analog[0] = array[29];//26-29
            analog[1] = array[28];
            analog[2] = array[27];
            analog[3] = array[26];
            float analog5 = *(float *)analog.data();

            float temp = (float)(array[8]*0xff + array[9]);
            float Ubat = *(float *)analog_1.data();
            float Imean = *(float *)analog_2.data();
            // 更新点集
            QVector<QPointF> oldPoints = series->pointsVector();
            QVector<QPointF> points;
            QVector<QPointF> oldPoints2 = filterdSeries->pointsVector();
            QVector<QPointF> points2;
            int length = oldPoints.count();
            int length2 = oldPoints2.count();

            for(int i=1; i <= length -5; ++i){
                points.append(QPointF((i-1)/SAMPLE_RATE ,oldPoints.at(i+4).y()));
            }
            points.append(QPointF((length-5)/SAMPLE_RATE,(double)analog1)); // 注入数据
            points.append(QPointF((length-4)/SAMPLE_RATE,(double)analog2));
            points.append(QPointF((length-3)/SAMPLE_RATE,(double)analog3));
            points.append(QPointF((length-2)/SAMPLE_RATE,(double)analog4));
            points.append(QPointF((length-1)/SAMPLE_RATE,(double)analog5));
            series->replace(points);

            for(int i=1; i < length2; i++){
                points2.append(QPointF((i-1)/SAMPLE_RATE*5 ,oldPoints2.at(i).y()));
            }
            if (marked_flag == 0 && temp >= 38)
            {
                temp = 0;
                marked_flag = 1;
            }
            points2.append(QPointF((length-1)/SAMPLE_RATE*5,(double)temp)); // 注入数据
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
