#pragma once
#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QDate>

static QByteArray IDSqlChangedEvent = QByteArrayLiteral("data_change_event");

//!
//! \brief The SqlNotification struct
//! \author Ivanov GD
//!
//! Структура для хранения информации, полученной из базы
//! в виде уведомления
//!
struct SqlNotification
{
    enum ActionType
    {
        UPDATE,
        INSERT,
        DELETE,
    };
    int iSource;

    QString itemUuid;
    QString table;
    QString schema;
    ActionType actionType;
    QJsonObject data;
    QJsonObject oldData;
};

Q_DECLARE_METATYPE(SqlNotification)
