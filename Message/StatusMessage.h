#ifndef _DISCONNECTTOSERVER_H_
#define _DISCONNECTTOSERVER_H_

#include <utility>

#include "EventBus/Event.hpp"
#include "include/CommonStrcut.h"

// tcp断链
class DisconnectToServer : public Event { };

// 验证失败消息
class AuthFailed : public Event
{
public:
    std::string  message;
};

// 触发重连消息
class RetryConnectToServerEvt : public Event
{
public:
    bool result = false;
};

// 重连结果消息
//class RetryConnectRet : public Event{ };

// 登录出错提示信息
class LoginErrMessage : public Event
{
public:
    std::string errorMessage;
};

// 重新获取导航信息
class RefreshNavEvt : public Event {};

// 回到登录窗口
class GoBackLoginWndEvt : public Event
{
public:
    std::string reason; // message
};

// 系统退出
class SystemQuitEvt : public Event {};

//
class GetHistoryError : public Event {};

//
class UserOnlineState : public Event
{
public:
    UserOnlineState(const QInt64 &login_t, const QInt64 &logout_t, std::string ip)
        : login_t(login_t), logout_t(logout_t), ip(std::move(ip)) {}

public:
    QInt64 login_t {0};
    QInt64 logout_t{0};
    std::string ip;
};

//
class OperatorStatistics : public Event
{
public:
    explicit OperatorStatistics(std::string ip, std::vector<st::StActLog> operators)
        : ip(std::move(ip)), operators(std::move(operators)) {}

public:
    const std::string ip;
    const std::vector<st::StActLog> operators;

};

//
class ExceptCpuEvt : public Event
{
public:
    double cpu;
    long long time;
    std::string stack;
};
#endif
