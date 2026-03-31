#ifndef ARDUINO_H
#define ARDUINO_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

class Arduino : public QObject
{
    Q_OBJECT

public:
    Arduino();
    ~Arduino();

    // OLD TEAM METHODS (kept for compatibility)
    int connect_arduino();
    int close_arduino();
    int write_to_arduino(QByteArray data);
    QByteArray read_from_arduino();
    QSerialPort* getSerialPort();
    QString getPortName();
    QString getarduino_port_name();

    // NEW METHODS (required by AutoEcole.cpp)
    bool isConnected() const;
    void sendToArduino(const QString &message);
    void processClientID(int idClient);
    bool checkClientExists(int idClient, QString &nom, QString &prenom);
    int getClientSessionsCount(int idClient);

signals:
    void dataReceived(const QString &data);
    void keyPressed(const QString &key);
    void clientIdEntered(const QString &id);

    // REQUIRED BY AutoEcole.cpp
    void clientFound(int id, const QString &nomComplet, int sessions);
    void clientNotFound(int id);
    void clientWithoutSessions(int id, const QString &nomComplet);
    void errorOccurred(const QString &error);
    void connectionChanged(bool connected);

private:
    QSerialPort *serial;
    static const quint16 arduino_vendor_id = 9025;
    static const quint16 arduino_product_id = 67;

    QString arduino_port_name;
    bool arduino_is_available;
    QByteArray data;
};

#endif // ARDUINO_H
