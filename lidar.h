#ifndef LIDAR_H
#define LIDAR_H

#include <QMainWindow>
#include "qcustomplot.h"
#include "comport.h"
#include "infowindow.h"
#include "mshare.h"
#include "tcpsocket.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Lidar; }
QT_END_NAMESPACE

class Lidar : public QMainWindow
{
    Q_OBJECT

public:
    Lidar(QWidget *parent = nullptr);
    ~Lidar();
    // метод для выделения из строки считанной с устройства данных
    // о времени и расстоянии до распознанных целей и добавляет запись в контейнер graph_value
    void parse_received_data(const QByteArray &data);

    void info_bottom_enable();

    void update_data(QByteArray &read_data); // слот обновляет массив данных прочитанных с устройства

    void update_shared_memory_data(QVector<double> vector);

    // Методы графика
    void plot_settings();

    void real_plot();


    // datagram
    // Методы графика
//    void plot_settings();

    void ether_plot(QByteArray received_data);

    void get_numbers_from_raw_data(QByteArray received_data);

signals:
    // сигналы для работы с com портом

    void sent_data_to_com_port(const QByteArray &data);

    void recive_data_from_com_port(QByteArray &data);

    void open_serial_port(const QString &text);

    void close_serial_port();

    // сигналы для работы с распределяемой памятью

    void write_to_shared_memory(QVector<double> data);

    void read_from_shared_memory();

    void do_tcp_connect();

    void do_tcp_disconnect();

private slots:
    void on_lon_clicked();

    void on_lof_clicked();

    void on_ver_clicked();

    void on_vlt_clicked();

    void on_css_clicked();

    void on_tm1_clicked();

    void on_rst_clicked();

    void on_syn1_clicked();

    void on_syn2_clicked();

    void on_srr_clicked();

    void on_nim_clicked();

    void on_connected_clicked();

    void on_nim1_clicked();

    void on_info_clicked();

    void on_tcp_connect_clicked();

    void on_tcp_disconnect_clicked();

private:
    Ui::Lidar *ui;

    InfoWindow *window;

    void  keyPressEvent(QKeyEvent *event);

    double lazer_start = 0;// при старте программы в эту переменную записывается текущее unix-time

    double min_L = 99999; // минимальное расстоние до цели (для масштабирования графика)

    double max_L = 0; // максимальное расстояние до цели (для масштабирования графика)

    bool key_pressed = false;

    bool is_connect = false; // для отображения кнопик соединения с портом

    bool lazer_on = false;
/////////////////////
    QByteArray data = ""; // при первом запуске кнопки connect контейнер data не обновляется
/////////////////////
    ComPort *com_port;

    MShare *shared_memory;
//    QSharedMemory shared_memory;

//    контейнер для хранения данных о целях
//    (время обнаружения - массив расстояний до обнаруженных объектов)
    std::map<double, QVector<double>> graph_value;

    QVector<double> result; // массив для записи команды распознавания в распределяемую память

    QVector<double> q_x; // массив для записи коодинат

    QVector<double> values; // вектор(массив) расстояний до обнаруженных объектов за 1 выстрел

    TcpSocket *socket = nullptr;

    QVector<int16_t> plot_points;

    int legacy_plot_points_size;

    QVector<int16_t> temp;


    QVector<int16_t> lidar_data;

    int16_t lidar_data_value;

};
#endif // LIDAR_H
