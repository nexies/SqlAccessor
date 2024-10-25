#pragma once
#include <QMap>
#include <QList>
#include <QPair>
#include <QVariant>

//!
//! \brief The SqlDataMapper class
//! \author Ivanov GD
//! Класс, для упрощения установления соответствия данных
//! в базе и программе
class SqlDataMapper
{
public:
    //!
    //! \brief SqlDataMapper Конструктор
    //! \param datamap Список соответствующих пар значений
    //!
    //! Первое значение в паре - то, как записано в базе
    //! Второе значение в паре - то, как записано в программе
    explicit SqlDataMapper(QList<QPair<QVariant, QVariant>> datamap);

private:
    //!
    //! \brief base_values
    //! Карта соответствия программа->база
    QMap<QVariant, QVariant> base_values;

    //!
    //! \brief screen_values
    //! Карта соответствия база->программа
    QMap<QVariant, QVariant> screen_values;

public:
    //!
    //! \brief bval Метод для получения значения в базе
    //! \param skey - значение в программе
    //! \return значение в базе
    //!
    const QVariant bval (const QVariant & skey) const;

    //!
    //! \brief sval Метод для получения значения в программе
    //! \param bkey - значение в базе
    //! \return значение в программе
    //!
    const QVariant sval (const QVariant & bkey) const;

    //!
    //! \brief registerValue Метод для добавления новой пары значений
    //! \param pair
    //! Первое значение в паре - то, как записано в базе
    //! Второе значение в паре - то, как записано в программе
    //!
    void registerValue (QPair<QVariant, QVariant> pair);

    //!
    //! \brief operator []
    //! \param key
    //! \return
    //!
    //! Оператор доступа, который сначала выполняет bval(),
    //! потом при неудаче sval(), а потом при неудаче
    //! возвращает "0"
    //!
    //! Не всегда безопасно пользоваться!
    const QVariant operator [] (const QVariant & key) const;

    //!
    //! \brief allScreenValues
    //! \return все возможные значения в программе
    //!
    QList<QVariant> allScreenValues () const;

    //!
    //! \brief allBaseValues
    //! \return все возможные значения в базе
    //!
    QList<QVariant> allBaseValues () const;
};
