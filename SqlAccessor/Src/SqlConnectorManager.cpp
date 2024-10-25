#include "SqlConnectorManager.h"
#include <QDebug>

namespace
{
    QByteArray Title = QByteArrayLiteral("[SqlConnectorManager] :");
}

SqlConnectorManager * SqlConnectorManager::_instance { nullptr };



SqlConnectorManager::SqlConnectorManager()
{
    // qRegisterMetaType<QSqlQuery> ();
    // qRegisterMetaType<QSqlField> ();
//    qDebug() << "[SqlConnectorManager][constructor] : Registering meta types now";
    qRegisterMetaType<QueryResult> ();
    qRegisterMetaType<SqlNotification> ();
    qRegisterMetaType<QSqlDriver::NotificationSource> ();
}

SqlConnectorManager::~SqlConnectorManager()
{
    for(auto conn: _connectors)
        delete conn;
}

SqlConnectorManager &SqlConnectorManager::instance()
{
    if(!_instance)
        _instance = new SqlConnectorManager;
    return *_instance;
}

void SqlConnectorManager::freeInstance()
{
    if(_instance)
        delete _instance;
}

SqlDatabaseConnector *SqlConnectorManager::getConnector(const QString &connectionName)
{
    if(_connectors.contains(connectionName))
        return _connectors[connectionName];
    else
        return nullptr;
}

bool SqlConnectorManager::addConnection(const QString &baseName, const QString &host, int port, QString connectionName)
{
    if(connectionName.isEmpty())
        connectionName = QString("%1-%2").arg(baseName, QUuid::createUuid().toString().mid(1, 36));

    if(_connectors.contains(connectionName))
    {
        qWarning().noquote() << Title << "can't add connection, connection name already exists!";
        return false;
    }

    auto newConnection = new SqlDatabaseConnector(host, port, baseName);
    newConnection->setConnectionName(connectionName);
    _connectors[connectionName] = newConnection;
    return true;
}

void SqlConnectorManager::removeConnection(const QString connectionName)
{
    if(_connectors.contains(connectionName))
    {
        delete _connectors[connectionName];
        _connectors.remove(connectionName);
    }
}

bool SqlConnectorManager::openConnection(const QString connectionName, const QString &username, const QString &password)
{
    if(!_connectors.contains(connectionName))
    {
        qWarning() << Title << "don't have connection called" << connectionName;
        return false;
    }
    return _connectors[connectionName]->connectToBase(username, password);
}

bool SqlConnectorManager::closeConnection(const QString &connectionName)
{
    if(!_connectors.contains(connectionName))
    {
        qWarning() << Title << "don't have connection named" << connectionName;
        return false;
    }
    return _connectors[connectionName]->disconnectFromBase();
}
