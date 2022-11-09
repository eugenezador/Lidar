#include "lidar.h"
#include "ui_lidar.h"

Lidar::Lidar(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Lidar)
    , window(new InfoWindow(this))
    , com_port(new ComPort(this))
    , shared_memory(new MShare(this))
    , socket(new TcpSocket(this))
    {
        ui->setupUi(this);

    //чтение доступных портов при запуске
        foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
            ui->portName->addItem(serialPortInfo.portName());
        }

        connect(this, &Lidar::open_serial_port, com_port, &ComPort::open_serial_port);
        connect(this, &Lidar::sent_data_to_com_port, com_port, &ComPort::writeData);
        connect(this, &Lidar::close_serial_port, com_port, &ComPort::close_serial_port);
        connect(com_port, &ComPort::received_data, this, &Lidar::parse_received_data);
        connect(com_port, &ComPort::received_data, this, &Lidar::update_data);

    //    connect(window, &InfoWindow::info_enable, this, &Ocounter::info_bottom_enable);

        connect(this, &Lidar::write_to_shared_memory, shared_memory, &MShare::write_to_shared_memory);
        connect(this, &Lidar::read_from_shared_memory, shared_memory, &MShare::read_from_shared_memory);
    //    connect(shared_memory, &MShare::read_data_from_shared_memory, this, &Ocounter::update_shared_memory_data, Qt::QueuedConnection);


    //    device_start = QDateTime::currentDateTimeUtc().toTime_t();

        plot_settings();


        connect(this, &Lidar::do_tcp_connect, socket, &TcpSocket::do_connect);
        connect(this, &Lidar::do_tcp_disconnect, socket, &TcpSocket::do_disconnect);
        connect(socket, &TcpSocket::data_from_lidar, this, &Lidar::ether_plot);
    }

Lidar::~Lidar()
{
    delete socket;
    delete shared_memory;
    delete com_port;
    delete window;
    delete ui;
}

void Lidar::parse_received_data(const QByteArray &data)
{
    result.clear();
        QString str = QString(data);
    QVector<double> values;
    double time = 0;
    QString tmp;
    int k = 0;
    int flag = 0;

    // если нет целей сразу выодим соответствующее сообщение
    if(strstr(data.constData(),"none")) {
        ui->read_log->append("No targets detected");
    }

    else if(strstr(data.constData(),"time")) {

    for(int i = 0; i < str.size() ; i++) {

        //выходим из цикла если последний элемент запятая
        if(str[i]  == ',' && i == str.size() - 1) break;

        // ловим начало числа авремени
        if(str[i] == 'e') {
            qDebug() << "flag 1 e";
            flag = 1;
            i++;
        }

        // ловим конец числа времени
        if (str[i] == ' ' && str.size() != 0) {
            flag = 0;
            qDebug() << "time : "<< tmp;
            time = lazer_start + tmp.toDouble() * 0.001;
//            result.push_back(tmp.toDouble());
            k =0;

            tmp.clear();
        }

        // ловим начало первой цели
        if(str[i] == ':' /*&& tmp.size() != 0*/) {
            qDebug() << "flag 1 :";
            flag = 1;
            i++;
        }

        // ловим начало второй цели
        if(str[i] == ',' /*&& tmp.size() != 0*/ && i != data.size() - 1 ) {
            qDebug() << "flag 1 ,";
            flag = 1;
            i++;
        }

        // ловим конец числа
        if (str[i] == '(' && data.size() != 0) {
            qDebug() << "point: " << tmp;
            flag = 0;
            values.append(tmp.toDouble());
//            result.push_back(tmp.toDouble());
            k =0;

            tmp.clear();
        }

        // если мы внутри нужного числа, то записываем i-й элемент в буферный массив
        if(flag) {
//            qDebug() << "inside need data: "<< str.at(i);
            tmp[k] = str[i];
//            qDebug() << "inside need tmp: "<< tmp.at(k);
            k++;
        }
    }

    graph_value[time] = values;

    real_plot();

//    emit write_to_shared_memory(result);

    ui->read_log->append(QString(data));

    qDebug() << "parse : " << graph_value;
    }
}

void Lidar::info_bottom_enable()
{
    ui->info->setEnabled(true);
}

void Lidar::update_data(QByteArray &read_data)
{
    data.clear();
    for(int i = 0; i < read_data.size(); i++) {
        data[i] = read_data[i];
    }
    qDebug() << "update data" << data;

}

void Lidar::update_shared_memory_data(QVector<double> vector)
{
    result.clear();
    for(int i = 0; i < vector.size(); i++) {
        result[i] = vector[i];
    }
    qDebug() << "update memory_data" << result;
}

void Lidar::plot_settings()
{
    ui->ocounter_plot->setInteraction(QCP::iRangeDrag, true);// взаимодействие удаления/приближения графика
    ui->ocounter_plot->setInteraction(QCP::iRangeZoom, true);// взвимодействие перетаскивания графика

    ui->ocounter_plot->addGraph();
    ui->ocounter_plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::green, Qt::green, 4));
    ui->ocounter_plot->graph(0)->setLineStyle(QCPGraph::lsNone);

    ui->ocounter_plot->xAxis->setLabel("t");
    ui->ocounter_plot->yAxis->setLabel("L");


    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    ui->ocounter_plot->xAxis->setTicker(dateTicker);

    ui->ocounter_plot->xAxis->setRange(QDateTime::currentDateTimeUtc().toTime_t(), QDateTime::currentDateTimeUtc().toTime_t()  + 200 );
    dateTicker->setDateTimeFormat("h:m:s");

    //
    //
    ui->ether_plot->setInteraction(QCP::iRangeDrag, true);// взаимодействие удаления/приближения графика
    ui->ether_plot->setInteraction(QCP::iRangeZoom, true);// взвимодействие перетаскивания графика

    ui->ether_plot->addGraph();
    ui->ether_plot->graph(0)->setPen(QPen(QColor(0, 255, 127)));
    ui->ether_plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::yellow, Qt::yellow, 4));
    ui->ether_plot->graph(0)->setLineStyle(QCPGraph::lsLine);

}

void Lidar::real_plot()
{
    if(lazer_on) {
//    if( !q_x.empty() && !q_y.empty() )
//        {
//            q_x.clear();
//            q_y.clear();
//        }
//    ui->plot->graph(0)->setData(q_x, q_y);

//    ui->plot->replot();
//    ui->plot->update();


    for (const auto& item : graph_value) {
        q_x.append(item.first);
        for (const double& point : item.second) {
            if(point < min_L) {
                min_L = point;
            }
            if(point > max_L) {
                max_L = point;
            }
            ui->ocounter_plot->graph(0)->addData(item.first, point);
        }

        ui->ocounter_plot->xAxis->setRange(q_x.first() - 4, q_x.last() + 4);
        ui->ocounter_plot->yAxis->setRange(min_L - 140, max_L + 140);
    }

    ui->ocounter_plot->replot();
    ui->ocounter_plot->update();
    }
}

void Lidar::on_lon_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$LON\r");
        lazer_on = true;
        key_pressed = false;
        lazer_start = QDateTime::currentDateTimeUtc().toTime_t();
    }

//    parse_received_data("Opt ch time255405 pnts:292.5(4383),331.4(1077),");

//    result << 1 << 4 << 4 << 1;
//    emit write_to_shared_memory(result);

}


void Lidar::on_lof_clicked()
{
    if(key_pressed) {
    emit sent_data_to_com_port("$LOF\r");
        lazer_on = false;
        key_pressed = false;
    }
}

void Lidar::on_ver_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$VER\r");
        key_pressed = false;
    }

//    emit read_from_shared_memory();
}

void Lidar::on_vlt_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$VLT\r");
        key_pressed = false;
    }
}

void Lidar::on_css_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$CSS\r");
        key_pressed = false;
    }
}

void Lidar::on_tm1_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$TM1\r");
        key_pressed = false;
    }
}

void Lidar::on_rst_clicked()
{
   if(key_pressed) {
       emit sent_data_to_com_port("$RST\r");
       key_pressed = false;
   }
}

void Lidar::on_syn1_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$SYN1\r");
        key_pressed = false;
    }
}

void Lidar::on_syn2_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$SYN2\r");
        key_pressed = false;
    }
}

void Lidar::on_srr_clicked()
{
    if(key_pressed) {
        data.clear();
        data.resize(6);
        data[0] = '$';
        data[1] = 'S';
        data[2] = 'R';
        data[3] = 'R';
        data[4] = ' ';
        data[5] = ui->srr_spinBox->value();
        data[6] = '\r';

        emit sent_data_to_com_port(data);

        key_pressed = false;
    }
}

void Lidar::on_nim_clicked()
{
    if(key_pressed) {
        data.clear();
        data.resize(6);
        data[0] = '$';
        data[1] = 'N';
        data[2] = 'I';
        data[3] = 'M';
        data[4] = ' ';
        data[5] = ui->nim_spinBox->value();
        data[6] = '\r';

        emit sent_data_to_com_port(data);

        key_pressed = false;
    }
}

void Lidar::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_S) {
        key_pressed = true;
    }
    else {
        key_pressed = false;
    }

    if(event->key() == Qt::Key_F1) {
        emit sent_data_to_com_port("$NIM 1\r");
    }

    if(event->key() == Qt::Key_V) {
        emit sent_data_to_com_port("$VER\r");
    }

    if(event->key() == Qt::Key_F2) {
        emit sent_data_to_com_port("$LON\r");
    }

    if(event->key() == Qt::Key_F3) {
        emit sent_data_to_com_port("$LOF\r");
    }

    if(event->key() == Qt::Key_F4) {
        emit sent_data_to_com_port("$CSS\r");
    }

    if(event->key() == Qt::Key_F5) {
        emit sent_data_to_com_port("$VLT\r");
    }

    if(event->key() == Qt::Key_T) {
        emit sent_data_to_com_port("$TM1\r");
    }

    if(event->key() == Qt::Key_R) {
        emit sent_data_to_com_port("$RST\r");
    }

    if(event->key() == Qt::Key_F6) {
        emit sent_data_to_com_port("$SYN1\r");
    }

    if(event->key() == Qt::Key_F7) {
        emit sent_data_to_com_port("$SYN2\r");
    }
}

void Lidar::on_connected_clicked()
{
//    com_port_permission();
    if(is_connect) {
        ui->connected->setText("Connect");
        ui->connected->setStyleSheet("*{ background-color: rgb(0, 153, 0); color:  rgb(255, 255, 255)}");

        emit close_serial_port();
        is_connect = false;
        qDebug() << "serial close";
    }
    else if(!is_connect) {
        emit open_serial_port(ui->portName->currentText());
        emit sent_data_to_com_port("$VER\r");

        if(com_port->serial->isOpen()) {
            ui->connected->setText("Disconnect");
            ui->connected->setStyleSheet("*{ background-color: rgb(255,0,0); color:  rgb(255, 255, 255)}");

            is_connect = true;
            qDebug() << "serial open";
            ui->read_log->append(data);
        } else {
            ui->read_log->append("Connection error");
        }
    }
}

void Lidar::on_nim1_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$NIM 1\r");
        key_pressed = false;
    }
}

void Lidar::on_info_clicked()
{
    window->show();
}


void Lidar::ether_plot(QByteArray received_data)
{
    lidar_data.clear();

    get_numbers_from_raw_data(received_data);

    qDebug() << "in plot: " << lidar_data;

    legacy_plot_points_size = plot_points.size();
    for(int i = 0; i < lidar_data.size(); i++){
        plot_points.append(lidar_data[i]);
    }

    int x_range = 0;
    for(int i = legacy_plot_points_size; i < plot_points.size(); i++){
        ui->ether_plot->graph(0)->addData(i, plot_points[i]);
        x_range = i;
    }

    ui->ether_plot->xAxis->setRange(-4,  x_range + 4);
    ui->ether_plot->yAxis->setRange(lidar_data[0], lidar_data.size());

    ui->ether_plot->replot();
    ui->ether_plot->update();
}

void Lidar::get_numbers_from_raw_data(QByteArray received_data)
{
    temp.clear();
    for(int i = 0; i < received_data.size(); i++) {
        temp.append(received_data[i]);
    }

    bool devider = false;
    for(int i = 0; i < temp.size(); i++) {
        if(i % 2 != 0){
            devider = true;
        }

        if(devider) {
            temp[i - 1] = temp[i - 1] << 10;
            temp[i - 1] = temp[i - 1] >> 10;

            temp[i] = temp[i] << 10;
            temp[i] = temp[i] >> 10;

            lidar_data_value = ( (temp[i] << 6) | (temp[i - 1]) );
            if(lidar_data_value & 800) {
                lidar_data_value = lidar_data_value ^ 800;
                lidar_data_value = lidar_data_value | 8000;
            }

            lidar_data.append (lidar_data_value);
            devider = false;
        }
    }
}

void Lidar::on_tcp_connect_clicked()
{
    emit do_tcp_connect();
    if(socket->is_connect){
        ui->tcp_connect->setEnabled(0);
        ui->tcp_disconnect->setEnabled(1);
    }
}


void Lidar::on_tcp_disconnect_clicked()
{
    emit do_tcp_disconnect();
    if(!socket->is_connect) {
        ui->tcp_connect->setEnabled(1);
        ui->tcp_disconnect->setEnabled(0);
    }
}

