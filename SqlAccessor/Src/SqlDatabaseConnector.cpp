#include "SqlDatabaseConnector.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlResult>
#include <QSqlRecord>
#include <QSqlDriver>
#include <QJsonDocument>
#include <QJsonObject>

namespace
{
    QByteArray Title = QByteArrayLiteral("[SqlDatabaseConnector] :");
}

inline QJsonObject recordToJson(const QSqlRecord & record, QTextCodec * codec)
{
    QJsonObject out;
//    qDebug().noquote() << "[recordToJson] : handling record : " << record;
    for(int i = 0; i < record.count(); i++)
    {
        QJsonValue value;
        if(codec)
            value = QJsonValue::fromVariant(codec->toUnicode(record.value(i).toByteArray()));
        else
            value = QJsonValue::fromVariant(record.value(i));

        out.insert(record.fieldName(i), value);
    }
//    qDebug().noquote() << "[recordToJson] : created json : " << out;
    return out;
}


SqlDatabaseConnector::SqlDatabaseConnector(QObject * parent):
    QObject(parent),
    _database {QSqlDatabase::addDatabase("QPSQL")}
{
//    _mutex = new QMutex();
//    moveToThread(&_thread);
//    _thread.setPriority(QThread::HighPriority);
//    _thread.start();

    connect(this, &SqlDatabaseConnector::sendQuerySignal,
            this, &SqlDatabaseConnector::onSendQuery);
    connect(this, &SqlDatabaseConnector::queryFinishedSignal,
            this, &SqlDatabaseConnector::onQueryFinished);
}

SqlDatabaseConnector::SqlDatabaseConnector(const QString baseHost, int port, const QString baseName, QObject *parent) :
    SqlDatabaseConnector(parent)
{
    _database.setHostName(baseHost);
    _database.setPort(port);
    _database.setDatabaseName(baseName);
    m_connectionName = QString("%1-%2").arg(baseName, QUuid::createUuid().toString().mid(1, 36));
}

SqlDatabaseConnector::~SqlDatabaseConnector()
{
//    _thread.exit(0);
//    delete _mutex;
    _database.close();
    qDebug() << Title << connectionName() << "closed connection";
}

const QString SqlDatabaseConnector::databaseName() const
{
    QMutexLocker locker(_mutex);
    return _database.databaseName();
}

const QString SqlDatabaseConnector::hostName() const
{
    QMutexLocker locker(_mutex);
    return _database.hostName();
}

int SqlDatabaseConnector::port() const
{
    QMutexLocker locker(_mutex);
    return _database.port();
}

const QString SqlDatabaseConnector::username() const
{
    QMutexLocker locker(_mutex);
    return _database.userName();
}

const QString SqlDatabaseConnector::password() const
{
    QMutexLocker locker(_mutex);
    return _database.password();
}

const QString SqlDatabaseConnector::connectionName() const
{
    return m_connectionName;
}

void SqlDatabaseConnector::sendQuery(const QUuid &uuid, const QString &query)
{
    if(debug) qDebug() << m_state;
    if(_queue.isEmpty() && m_state == Idle)
        emit sendQuerySignal(uuid, query);
    else
    {
        qDebug().noquote() << Title << "Queuing query" << uuid.toString().mid(1, 36);
        _queue.push_back({uuid, query});
    }
}

bool SqlDatabaseConnector::connectToBase(const QString &username, const QString &password)
{
    if(hostName().isEmpty())
    {
        qWarning() << Title << "can't connect to base with an unknown host!";
        return false;
    }
    if(databaseName().isEmpty())
    {
        qWarning() << Title << "can't connect to base with an unknown name!";
        return false;
    }
    return connectToBase(hostName(), port(), databaseName(), username, password);
}

bool SqlDatabaseConnector::connectToBase(const QString &host, int port, const QString &baseName, const QString &username, const QString &password)
{
    if(_database.isOpen())
    {
        qWarning().noquote() << Title << "can't connect to base, because already connected. Use reconnectToBase() instead";
        return false;
    }
    _database.setHostName(host);
    _database.setPort(port);
    _database.setDatabaseName(baseName);
    _database.setUserName(username);
    _database.setPassword(password);

    bool ok = _database.open();
    if(!ok)
    {
        qWarning().noquote() << Title << "could not connect to database!";
        qWarning().noquote() << _database.lastError().text();
        m_state = Disconnected;
        return false;
    }
    qDebug().noquote() << Title << "connected to database" << databaseName() << "as user" << this->username();
    m_state = Idle;
    emit connected();

    if(!_database.driver()->subscribeToNotification(IDSqlChangedEvent))
        qDebug().noquote() << Title << _database.driver()->lastError().databaseText();
    else
        qDebug().noquote().nospace() << Title << "subscribed for notification \"" << IDSqlChangedEvent << "\"";

    connect(_database.driver(), SIGNAL(notification(const QString &, QSqlDriver::NotificationSource, const QVariant &)),
            this, SLOT(onDBNotify(const QString &, QSqlDriver::NotificationSource, const QVariant &)));

    return true;
}

bool SqlDatabaseConnector::disconnectFromBase()
{
    if(isOpen()){
        _database.close();
        emit disconnected();
    }
    return true;
}

void SqlDatabaseConnector::onSendQuery(const QUuid &uuid, const QString query_str)
{
    QString query_str_coded = query_str;

    if(!_database.isOpen())
    {
        qWarning() << Title << "base is not open, can't send a query!";
        return;
    }


    if(!_query)
    {
        if(debug) qDebug().noquote() << Title << "constructing a query. Thread id:" << QThread::currentThreadId();
        _query = new QSqlQuery(_database);
        _query->setForwardOnly(true);
    }

    if(!_queue.isEmpty() || m_state != Idle)
    {
        if(debug) qDebug() << Title << "Putting query in queue";
        _queue.push_back({uuid, query_str});
        return;
    }


    if (debug) qDebug().noquote() << "[SqlDatabaseConnector] : executing query:" << query_str;
    m_state = Busy;
    _query->finish();

    if(_codec)
        query_str_coded = _codec->fromUnicode(query_str);
    // qDebug() << query_str_coded;

    bool ok = _query->exec(query_str_coded);

    if(!ok)
    {
        qWarning().noquote() << Title << "query error" << _query->lastError().text();
        qWarning().noquote() << _query->lastQuery();
        emit queryErrorSignal(uuid, _query->lastError());
    }
    else
    {
        if(debug) qDebug() << Title << "Executed query" << query_str;
    }

    QueryResult out;
    out.error = _query->lastError();
    out.isSelect = _query->isSelect();
    if(out.isSelect)
        while(_query->next()){
            out.records << recordToJson(_query->record(), _codec);
        }

    _query->finish();


    emit queryFinishedSignal(uuid, out);
    m_state = Idle;
}

void SqlDatabaseConnector::onQueryFinished(const QUuid &uuid, QueryResult res)
{
    Q_UNUSED(res)

    if(!_queue.isEmpty())
    {
        auto q = _queue.dequeue();
        if (debug) qDebug().noquote() << Title << "Dequeuing query" << uuid.toString().mid(1, 36);
        emit sendQuerySignal(q.first, q.second);
    }
}

void SqlDatabaseConnector::onDBNotify(const QString &name, QSqlDriver::NotificationSource source, const QVariant &payload)
{
    Q_UNUSED(name)

    if(debug) qDebug() << Title << "Notification from DB!";
    // qDebug() << "Name: " << name;
    // qDebug() << "Source: " << source;
    // qDebug() << "Payload: " << payload;

    SqlNotification notif;

    QByteArray bytes = payload.toByteArray();
    if(_codec)
        bytes = _codec->toUnicode(bytes).toUtf8();

    QJsonObject obj = QJsonDocument::fromJson(bytes).object();

    if (debug) qDebug().noquote() << obj;

    notif.iSource = source;
    notif.data = obj.value("data").toObject();
    notif.oldData = obj.value("data_old").toObject();
    notif.table = obj.value("table").toString();
    notif.schema = obj.value("schema").toString();
    notif.itemUuid = notif.data.value("_uuid").toString();

    notif.actionType = obj.value("action") == "UPDATE" ? SqlNotification::UPDATE :
                       obj.value("action") == "INSERT" ? SqlNotification::INSERT :
                                                         SqlNotification::DELETE;

    if(debug) {
        qDebug().noquote() << "-------Source: " << source;
        qDebug().noquote() << "-------Item uuid: " << notif.itemUuid;
        qDebug().noquote() << "-------Scheme: " << notif.schema;
        qDebug().noquote() << "-------Table: " << notif.table;
        qDebug().noquote() << "-------Action" << notif.actionType;
        qDebug().noquote() << "-------Data: " << notif.data;
        qDebug().noquote() << "-------Old data: " << notif.oldData;
        qDebug() << "";
    }
    emit dbNotification(notif);
}

const SqlDatabaseConnector::State &SqlDatabaseConnector::state() const
{
    return m_state;
}

QTextCodec * const SqlDatabaseConnector::codec() const
{
    return _codec;
}

void SqlDatabaseConnector::setCodec(QTextCodec *codec)
{
    _codec = codec;
    qDebug().noquote().nospace() << "[SqlDatabaseConnector] : using codec : '" << codec->name() << "'";
}

void SqlDatabaseConnector::setConnectionName(const QString &newConnectionName)
{
    m_connectionName = newConnectionName;
}

bool SqlDatabaseConnector::isOpen()
{
    return _database.isOpen();
}
