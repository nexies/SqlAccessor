#include "SqlDataMapper.h"

SqlDataMapper::SqlDataMapper(QList<QPair<QVariant, QVariant>> datamap)
{
    for(auto pair: datamap)
    {
        registerValue(pair);
    }
}

const QVariant SqlDataMapper::bval(const QVariant &skey) const
{
    if(screen_values.contains(skey))
        return screen_values[skey];
    else
        return "Неизвестно";
}

const QVariant SqlDataMapper::sval(const QVariant &bkey) const
{
    if(base_values.contains(bkey))
        return base_values[bkey];
    else
        return "0";
}

void SqlDataMapper::registerValue(QPair<QVariant, QVariant> pair)
{
    base_values[pair.first] = pair.second;
    screen_values[pair.second] = pair.first;
}

const QVariant SqlDataMapper::operator [](const QVariant &key) const
{
    if(base_values.contains(key))
        return base_values[key];
    else if (screen_values.contains(key))
        return screen_values[key];
    else
        return "0";
}

QList<QVariant> SqlDataMapper::allScreenValues() const
{
    return screen_values.keys();
}

QList<QVariant> SqlDataMapper::allBaseValues() const
{
    return base_values.keys();
}
