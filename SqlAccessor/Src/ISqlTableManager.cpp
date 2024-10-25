#include "ISqlTableManager.h"
#include <QUuid>
#include <QSqlQuery>
#include <QDebug>
#include <QMetaClassInfo>
#include <QJsonObject>


namespace
{
    QByteArray Title = QByteArrayLiteral("[ISqlTableManager] :");
}


ISqlTableManager::ISqlTableManager(SqlDatabaseConnector *connector, const QString &tableScheme, const QString &tableName, QObject * parent) :
    QObject(parent)
{
    _connector = connector;
    setTableName(tableName);
    setTableScheme(tableScheme);

    connect(this, &ISqlTableManager::execQuerySignal,
            _connector, &SqlDatabaseConnector::sendQuery);

    connect(_connector, &SqlDatabaseConnector::queryFinishedSignal,
            this, &ISqlTableManager::onQueryFinished);

    connect(_connector, &SqlDatabaseConnector::dbNotification,
            this, &ISqlTableManager::onDBNotification);

    connect(this, &ISqlTableManager::updated,
            this, &ISqlTableManager::updateModel);
}

int ISqlTableManager::insert(ISqlTableItem::ptr row)
{
    int validCode = checkItemValid(row);
    if(validCode == 0)
        sendQuery(insertQuery(row));

    return validCode;
}

int ISqlTableManager::update(ISqlTableItem::ptr row)
{
    int validCode = checkItemValid(row);
    if(validCode == 0)
        sendQuery(updateQuery(row));

    return validCode;
}

int ISqlTableManager::remove(ISqlTableItem::ptr row)
{
//    if(checkItemValid(row))
    sendQuery(deleteQuery(row));
    return 0;
}

int ISqlTableManager::checkItemValid(ISqlTableItem::ptr item)
{
    Q_UNUSED(item)
    return 0;
}

QString ISqlTableManager::lastItemCheckString()
{
    return _lastItemCheckString;
}

const QList<ISqlTableItem::ptr> ISqlTableManager::items()
{
    QList<ISqlTableItem::ptr> out;
    for(auto key: _items.keys())
        out << _items[key];
    return out;
}

ISqlTableItem::ptr ISqlTableManager::item(const QString &uuid)
{
    if(_items.contains(uuid))
        return _items[uuid];
    else
        return ISqlTableItem::ptr(nullptr);
}

QString ISqlTableManager::selectQuery()
{
    return QString("SELECT * FROM %1.%2;").arg(tableScheme(), tableName());
}

QString ISqlTableManager::insertQuery(ISqlTableItem::ptr item)
{
    return QString("INSERT INTO %1.%2 (%3, _uuid) VALUES (%4, %5);").
            arg(tableScheme(), tableName(),
                item->sqlFieldNames().join(", "),
                item->sqlValues().join(", "),
                QString("'%1'").arg(item->uuid()));

}

QString ISqlTableManager::updateQuery(ISqlTableItem::ptr item)
{
    auto fieldNames = item->sqlFieldNames();
    auto fieldValues = item->sqlValues();
    QStringList fieldNameValue;
    for(int i = 0; i < fieldNames.size(); i++)
    {
        fieldNameValue << QString("%1=%2").arg(fieldNames[i], fieldValues[i]);
    }

    return QString("UPDATE %1.%2 SET %3 WHERE _uuid=%4").
            arg(tableScheme(), tableName(),
                fieldNameValue.join(", "),
                QString("'%1'").arg(item->uuid()));
}

QString ISqlTableManager::deleteQuery(ISqlTableItem::ptr item)
{
    return QString("DELETE FROM %1.%2 WHERE _uuid=%3;").
            arg(tableScheme(), tableName(),
                QString("'%1'").arg(item->uuid()));
}

bool ISqlTableManager::autoParseQuery(ISqlTableItem::ptr item, const QJsonObject & record)
{
    bool ok = true;
    QStringList fields = item->sqlFieldNames();
    if(!record.contains("_uuid"))
    {
        qCritical () << Title << "can't auto parse the query - '_uuid' does not exist!";
        return false;
    }
    item->setUuid(record.value("_uuid").toString());

    for(auto field: fields)
    {
        if(!record.contains(field))
        {
            qWarning().noquote() << Title << "failed to auto parse query - record doesn't contain field" << field;
            ok = false;
        }
        item->_private->setProperty(field.toStdString().c_str(), record.value(field));
    }
    return ok;
}

void ISqlTableManager::load()
{
    sendQuery(selectQuery());
}

void ISqlTableManager::unload()
{
    _items.clear();
}

void ISqlTableManager::fullReload()
{
    unload();
    load();
}

int ISqlTableManager::count() const
{
    return _items.count();
}

const QStandardItemModel *ISqlTableManager::model() const
{
    return _model;
}

void ISqlTableManager::setModel(QStandardItemModel *newModel)
{
    _model = newModel;
}

const QString &ISqlTableManager::tableName() const
{
    return m_tableName;
}

void ISqlTableManager::setTableName(const QString &newTableName)
{
    m_tableName = newTableName;
}

const QString &ISqlTableManager::tableScheme() const
{
    return m_tableScheme;
}

void ISqlTableManager::setTableScheme(const QString &newTableScheme)
{
    m_tableScheme = newTableScheme;
}

void ISqlTableManager::sendQuery(const QString &query)
{
    QUuid uuid = QUuid::createUuid();
    _awaitedQueries << uuid;
    emit execQuerySignal(uuid, query);
}

void ISqlTableManager::onQueryFinished(const QUuid &uuid, QueryResult result)
{
    if(_debug) qDebug().noquote() << Title << QString("query finished for table %1.%2!").arg(m_tableScheme, m_tableName);
//    qDebug() << _awaitedQueries << uuid;

    if(!_awaitedQueries.contains(uuid))
    {
        return;
    }
    _awaitedQueries.removeAll(uuid);

    if(result.error.type() != QSqlError::NoError)
    {
        qWarning().noquote() << QString("[%1] query error : %2").arg(this->metaObject()->className(), result.error.text());
        return;
    }

//    qDebug() << "Qeury size" << query.size();
    if(result.isSelect)
    {
        if(_debug) qDebug().noquote() << Title << "type - SELECT";
        if(_debug) qDebug().noquote() << Title << "size - " << result.records.size();
//        qDebug().noquote() << Title << "data - " << result.records;
        unload();
        for(auto record: result.records)
        {
            auto item = parseSingleQuery(record);
            if(item)
                _items[record.value("_uuid").toString()] = item;
            else
                qWarning().noquote() << Title << "not adding item to the list";
        }
        emit updated();
    }
}

void ISqlTableManager::onDBNotification(const SqlNotification notif)
{
//    qDebug().noquote() << QString("[ISqlTableManager] : notification for %1.%2").arg(notif.schema, notif.table);
    if(notif.table != tableName() || notif.schema != tableScheme())
    {
        if(_items.contains(notif.itemUuid))
            qWarning().noquote() << QString("[%1][onDBNotification] : table name (%2) or scheme (%3) didn't match the notification(%4.%5),\n"
                                            "but the manager has an item with such uuid. What went wrong?")
                                    .arg(metaObject()->className(),
                                         tableName(),
                                         tableScheme(),
                                         notif.schema,
                                         notif.table);
        return;
    }

    switch(notif.actionType)
    {
    case SqlNotification::INSERT:
    {
        if(_debug) qDebug().noquote() << Title << QString("Received INSERT for table %1.%2").arg(tableScheme(), tableName());
        auto newItem = parseSingleQuery(notif.data);
        _items.insert(notif.itemUuid, newItem);
    }
    break;
    case SqlNotification::UPDATE:
    {
        if(_debug) qDebug().noquote() << Title << QString("Received UPDATE for table %1.%2").arg(tableScheme(), tableName());
        auto item = parseSingleQuery(notif.data);
        if(_items.contains(notif.itemUuid))
            _items[notif.itemUuid] = item;
        else
            qWarning () << Title << "UPDATE for non-existing item!";
    }
    break;
    case SqlNotification::DELETE:
    {
        if(_debug) qDebug().noquote() << Title << QString("Received DELETE for table %1.%2").arg(tableScheme(), tableName());
        if(_items.contains(notif.itemUuid))
            _items.remove(notif.itemUuid);
        else
            qWarning () << Title << "REMOVE for non-existing item!";
    }
    break;
    }

    emit updated();
    emit updatedItem(item(notif.itemUuid));
}
