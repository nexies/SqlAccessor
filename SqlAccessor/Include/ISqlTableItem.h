#pragma once
#include <QObject>
#include <QSharedPointer>
#include <QVariant>
#include <QUuid>
#include <QDateTime>
#include <QRegularExpression>
#include <QDebug>


//!
//! \brief The ISqlTableItem class
//! \author Ivanov GD
//!
class ISqlTableItem : public QObject
{

    friend class ISqlTableManager;

    Q_OBJECT

public:
    //!
    //! \brief ISqlTableItem
    //! Конструктор
    //!
    //! В конструкторе должно инициализироваться поле _private
    //! соответствующим классом приватной части элемента
    //!
    //! Например:
    //! <MySqlTableItem.cpp>
    //!
    //! MySqlTableItem::MySqlTableItem () : ISqlTableItem ()
    //! {
    //!     _private = new MySqlTableItemPrivate(this);
    //! }
    //!
    ISqlTableItem();
    //!
    //! \brief ~ISqlTableItem
    //! Деструктор
    virtual ~ISqlTableItem ();

    //!
    //! \brief Тип ptr и статическая функция create:
    //!
    //! В классе наследнике нужно добавить такое же, например:
    //! <MySqlTableItem.h>
    //!
    //! class MySqlTableItem : public ISqlTableItem {
    //!     ...
    //!     using ptr = QSharedPointer<MySqlTableItem>;
    //!     static ptr create() { return ptr(new MySqlTableItem); }
    //!     ...
    //! }
    using ptr = QSharedPointer<ISqlTableItem>;
    static ptr create() { return ptr(new ISqlTableItem); }

    //!
    //! \brief makeUuid Статический метод для генерации uuid
    //! \return
    //!
    static QString makeUuid ();

    //!
    //! \brief sqlFieldNames Метод, возвращающий список названий полей в SQL таблице
    //! \return
    //!
    QStringList sqlFields() const;

    //!
    //! \brief sqlValuesCount
    //! \return Количество колонок в таблице БД
    //!
    int count() const;

    //!
    //! \brief value Метод для получения значения поля из таблицы БД
    //! \param name - название поля
    //! \return
    //!
    QVariant value (const QString & name);

    //!
    //! \brief uuid
    //! \return Уникальный идентификатор элемента
    //!
    const QString &uuid() const;

    //!
    //! \brief setUuid Метод для установки уникального идентификатора элемента
    //! \param uuid - Новое значение
    //!
    void setUuid (const QString & uuid);

    //!
    //! \brief fromJsonObject Метод для десериализации из объекта Json
    //! \param obj - данные
    //! \return true/false - получилось или нет
    //!
    bool fromJsonObject(const QJsonObject & obj);

    //!
    //! \brief toJsonObject Метод для сериализации в Json объект
    //! \return данные
    //!
    QJsonObject toJsonObject() const;

    QString sqlNotaion (const QString & fieldName);

    QStringList allSqlNotations ();
protected:
    //!
    //! \brief _uuid
    //! Уникальный идентификатор элемента
    QString _uuid;

    //!
    //! \brief _creationTime
    //! Время создания элемента
    QDateTime _creationTime;

private:


};



