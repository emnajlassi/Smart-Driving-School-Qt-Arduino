#include "arduino.h"
#include <QDebug>

Arduino::Arduino()
{
    serial = new QSerialPort();
    arduino_is_available = false;
    arduino_port_name = "";
}

Arduino::~Arduino()
{
    close_arduino();
    delete serial;
}

/* ============================================================
 *   OLD TEAM METHODS (unchanged for compatibility)
 * ============================================================ */
int Arduino::connect_arduino()
{
    foreach (const QSerialPortInfo &serial_port_info, QSerialPortInfo::availablePorts())
    {
        if (serial_port_info.hasVendorIdentifier() && serial_port_info.hasProductIdentifier())
        {
            if (serial_port_info.vendorIdentifier() == arduino_vendor_id &&
                serial_port_info.productIdentifier() == arduino_product_id)
            {
                arduino_port_name = serial_port_info.portName();
                arduino_is_available = true;
                break;
            }
        }
    }

    if (arduino_is_available)
    {
        serial->setPortName(arduino_port_name);
        serial->setBaudRate(QSerialPort::Baud9600);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);

        if (serial->open(QIODevice::ReadWrite))
        {
            emit connectionChanged(true);
            return 0;
        }
        else
        {
            emit errorOccurred("Unable to open serial port");
            return 1;
        }
    }
    return -1;
}

int Arduino::close_arduino()
{
    if (serial->isOpen())
    {
        serial->close();
        emit connectionChanged(false);
        return 0;
    }
    return 1;
}

QByteArray Arduino::read_from_arduino()
{
    if (serial->isReadable())
    {
        data = serial->readAll();
        QString text = QString::fromUtf8(data).trimmed();

        if (!text.isEmpty())
        {
            emit dataReceived(text);

            if (text.startsWith("KEY:"))
                emit keyPressed(text.mid(4));

            if (text.startsWith("ENTER:"))
                emit clientIdEntered(text.mid(6));
        }

        return data;
    }
    return QByteArray();
}

int Arduino::write_to_arduino(QByteArray d)
{
    if (serial->isWritable())
    {
        serial->write(d);
        return 0;
    }
    emit errorOccurred("Couldn't write to serial");
    return 1;
}

QSerialPort* Arduino::getSerialPort()
{
    return serial;
}

QString Arduino::getPortName()
{
    return arduino_port_name;
}

QString Arduino::getarduino_port_name()
{
    return arduino_port_name;
}

/* ============================================================
 *   NEW METHODS REQUIRED BY AutoEcole.cpp
 * ============================================================ */

bool Arduino::isConnected() const
{
    return serial->isOpen();
}

void Arduino::sendToArduino(const QString &message)
{
    if (!serial->isOpen())
    {
        emit errorOccurred("Arduino not connected");
        return;
    }

    QByteArray data = message.toUtf8();
    serial->write(data);
    serial->write("\n");
    serial->flush();
}

/* ====== DATABASE CHECKS (from your previous code) ====== */
bool Arduino::checkClientExists(int idClient, QString &nom, QString &prenom)
{
    QSqlQuery query;
    query.prepare("SELECT NOM, PRENOM FROM CLIENT WHERE ID_CLIENT = :id");
    query.bindValue(":id", idClient);

    if (!query.exec())
        return false;

    if (query.next())
    {
        nom = query.value(0).toString();
        prenom = query.value(1).toString();
        return true;
    }

    return false;
}

int Arduino::getClientSessionsCount(int idClient)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM SEANCE WHERE ID_CLIENT = :id");
    query.bindValue(":id", idClient);

    if (!query.exec() || !query.next())
        return -1;

    return query.value(0).toInt();
}

/* ====== FINAL: Process Client ID ====== */
void Arduino::processClientID(int idClient)
{
    QString nom, prenom;

    bool exists = checkClientExists(idClient, nom, prenom);

    if (!exists)
    {
        sendToArduino("RESULT:NOT_FOUND");
        emit clientNotFound(idClient);
        return;
    }

    int count = getClientSessionsCount(idClient);

    QString full = nom + " " + prenom;

    if (count < 0)
    {
        sendToArduino("RESULT:ERROR");
        emit errorOccurred("Database error");
        return;
    }

    if (count == 0)
    {
        sendToArduino("RESULT:NO_SESSIONS:" + full);
        emit clientWithoutSessions(idClient, full);
    }
    else
    {
        sendToArduino("RESULT:OK");
        emit clientFound(idClient, full, count);
    }
}
