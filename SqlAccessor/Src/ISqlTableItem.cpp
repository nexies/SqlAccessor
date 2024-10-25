#include "ISqlTableItem.h"
#include <QMetaProperty>
#include <QDebug>
#include <QJsonObject>

ISqlTableItemPrivate::ISqlTableItemPrivate(QObject *parent) :
    QObject(parent)
{

}

QStringList ISqlTableItemPrivate::fieldNames() const
{
    auto mobj = metaObject();
    QStringList out;
    for(int i = mobj->propertyOffset(); i < mobj->propertyCount(); i++)
    {
        out << mobj->property(i).name();
    }
    return out;
}

QVariantList ISqlTableItemPrivate::values() const
{
    auto mobj = metaObject();
    QVariantList out;
    for(int i = mobj->propertyOffset(); i < mobj->propertyCount(); i++)
    {
        out << mobj->property(i).read(this);
    }
    return out;
}

QString ISqlTableItemPrivate::sqlValue(const QString &name) const
{
    QVariant var = property(name.toStdString().c_str());

    switch(var.type())
    {
    case QVariant::Char:
    case QVariant::String:
    case QVariant::ByteArray:
    {
        return QString("'%1'").arg(var.toString());
    }
    break;
    case QVariant::Int:
    {
        return QString::number(var.toInt());
    }
    break;
    case QVariant::Double:
    {
        return QString::number(var.toDouble());
    }
    break;
    case QVariant::Bool:
    {
        return (var.toBool() ? "true" : "false");
    }
    break;
    case QVariant::Uuid:
    {
        return QString("'%1'").arg(var.toUuid().toString().mid(1, 36));
    }
    break;
    default:
    {
        qWarning().noquote() << QString("[%1] : dunno what to do with QVariant type").
                      arg(metaObject()->className())
                   << var.type();
        return "''";
    }
    break;
    }
}

QStringList ISqlTableItemPrivate::sqlValues() const
{
    auto fields = fieldNames();
    QStringList out;
    for(auto field: fields)
        out << sqlValue(field);

    return out;
}

int ISqlTableItemPrivate::count() const
{
    return metaObject()->propertyCount();
}

QVariant ISqlTableItemPrivate::value(const QString &name) const
{
    return property(name.toStdString().c_str());
}


ISqlTableItem::ISqlTableItem()
{
    _uuid = ISqlTableItem::makeUuid();
    _creationTime = QDateTime::currentDateTime();
}

ISqlTableItem::~ISqlTableItem()
{

}

QString ISqlTableItem::makeUuid()
{
    return QString(QUuid::createUuid().toString()).mid(1, 36);
}

QStringList ISqlTableItem::sqlFieldNames() const
{
    return _private->fieldNames();
}


QStringList ISqlTableItem::sqlValues() const
{
    return _private->sqlValues();
}

int ISqlTableItem::sqlValuesCount() const
{
    return _private->count();
}

QVariant ISqlTableItem::value(const QString &name)
{
    return _private->value(name);
}

const QString &ISqlTableItem::uuid() const
{
    return _uuid;
}

void ISqlTableItem::setUuid(const QString &uuid)
{
    _uuid = uuid;
}

bool ISqlTableItem::fromJsonObject(const QJsonObject &obj)
{
    bool ok = true;
    for(auto field : sqlFieldNames())
    {
        if(obj.contains(field))
            _private->setProperty(field.toStdString().c_str(), obj.value(field).toString());
        else
        {
            qDebug() << "[ISqlTableItem][fromJsonObject] : Json object does not have field" << field;
            ok = false;
        }
    }
    if(!obj.contains("_uuid"))
    {
        ok = false;
        qDebug() << "[ISqlTableItem][fromJsonObject] : Json object does not have field '_uuid'!";
    }
    else
        setUuid(obj.value("_uuid").toString());

    return ok;
}

QJsonObject ISqlTableItem::toJsonObject() const
{
    QJsonObject obj;
    for(auto field: sqlFieldNames())
    {
        obj.insert(field, _private->property(field.toStdString().c_str()).toJsonValue());
    }
    obj.insert("_uuid", uuid());
    return obj;
}



