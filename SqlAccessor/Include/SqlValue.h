#pragma once
#include <QVariant>
#include <QSqlField>


class SqlValue
{
public:
    SqlValue();

    explicit SqlValue(const QSqlField & field);
    SqlValue(const SqlValue & other);
    SqlValue(const QVariant & other);

private:
    QVariant::Type _type {QVariant::Invalid};
    QVariant _data { QVariant() };
    int _length { -1 };
};

