//
// Created by cc on 2019/11/02.
//

#ifndef STALK_V2_LOCALSERVER_H
#define STALK_V2_LOCALSERVER_H

#include <QObject>
#include <QLocalServer>

enum package{
    package_invalid = 0,
    package_say_hi,
    package_say_hi_ack,
    package_say_bye,
    package_say_bye_ack, // 暂时无用 发出say bye 之后直接释放
    package_request,
    package_response
};

enum request {
    request_invalid = 0,
    request_base_info = 0x0001,
    request_user_info  = 0x0010,
    request_wakeup_window  = 0x0100,
};

/**
* @description: LocalServer
* @author: cc
* @create: 2019-11-02 11:00
**/
class LocalServer : public QObject {
    Q_OBJECT
public:
    LocalServer();
    ~LocalServer() override;

public:
    void runServer(const QString& listenName);

Q_SIGNALS:
    void sgWakeupWindow();

private slots:
    void onNewClient();
    void socketReadyReadHandler();

private:
    void dealMessage(QLocalSocket *socket, const QByteArray &arrayData);

private:
    QLocalServer *_pServer;

    std::map<QLocalSocket*, QByteArray> _datas;
};


#endif //STALK_V2_LOCALSERVER_H
