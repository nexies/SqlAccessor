#pragma once
#include <QObject>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardItemModel>
#include "ISqlTableItem.h"
#include "SqlDatabaseConnector.h"

/****************************************************************************
 *                           ISqlTableManager                               *
 *                                                                          *
 *  Класс для управления конкретной таблицей (или несколькими таблицами)    *
 *  в базе данных, предоставляющий возможности для вставки, редактирования  *
 *  и удаления данных, а так же синхронизации данных в БД с данными в       *
 *  программе.                                                              *
 *                                                                          *
 ****************************************************************************
*/



//!
//! \brief The ISqlTableManager class
//! \author Ivanov GD
//!
class ISqlTableManager : public QObject
{
    Q_OBJECT

public:
    //!
    //! \brief The LoadedState enum
    //! Состояние данных
    enum LoadedState
    {
        NotLoaded,
        Loading,
        Loaded,
    };
    Q_ENUM(LoadedState)

    // enum SyncMode
    // {
    //     DoFullReloads,
    //     SyncChanges,
    // };
    // Q_ENUM(SyncMode)

    // enum UpdateMode
    // {
    //     UpdateModelFull,
    //     UpdateModelChanges,
    //     NoUpdate
    // };
    // Q_ENUM(UpdateMode)

    Q_PROPERTY(QString tableName READ tableName WRITE setTableName)
    Q_PROPERTY(QString tableScheme READ tableScheme WRITE setTableScheme)

protected:
    //!
    //! \brief m_baseName
    //! Название БД
    QString m_baseName;

    //!
    //! \brief m_tableName
    //! Название таблицы в БД
    QString m_tableName;

    //!
    //! \brief m_tableScheme
    //! Название схемы, в которой находится таблица в БД
    QString m_tableScheme;

    //!
    //! \brief _connector
    //! Указатель на коннектор к базе данных
    SqlDatabaseConnector * _connector { nullptr };

    //!
    //! \brief _model
    //! Указатель на модель данных, которая представляет данные из базы
    //! в программе
    QStandardItemModel * _model { nullptr };

    //!
    //! \brief _items
    //! Данные таблицы
    QMap<QString, ISqlTableItem::ptr> _items;

    //!
    //! \brief _awaitedQueries
    //! Список идентификаторов sql-запросов, которые были сделаны,
    //! но результат еще не вернулся
    QList<QUuid> _awaitedQueries;

    //!
    //! \brief _lastItemCheckString
    //! Строка с результатом последней проверки методом checkItemValid()
    QString _lastItemCheckString;


    //! Выводить или не выводить дебаг в консоль.
    bool _debug { true };


public:
    //!
    //! \brief ISqlTableManager - конструктор
    //! \param connector - Укаатель на коннектор к БД
    //! \param tableScheme - Название схемы
    //! \param tableName - Название таблицы
    //! \param parent - Указатель на родителя QObject
    ISqlTableManager(SqlDatabaseConnector * connector, const QString & tableScheme, const QString & tableName, QObject * parent = nullptr);

    //!
    //! \brief insert Метод для вставки элемента в таблицу БД
    //! \param row - Элемент
    //! \return Код причины, почему нельзя выполнить запрос (проверяется функцией checkItemValid)
    //!         0 - если запрос был успешно отправлен
    virtual int  insert(ISqlTableItem::ptr row);

    //!
    //! \brief insert Метод для обновления элемента в таблице БД
    //! \param row - Элемент
    //! \return Код причины, почему нельзя выполнить запрос (проверяется функцией checkItemValid)
    //!         0 - если запрос был успешно отправлен
    virtual int  update(ISqlTableItem::ptr row);

    //!
    //! \brief insert Метод для удаления элемента из таблицы БД
    //! \param row - Элемент
    //! \return Код причины, почему нельзя выполнить запрос (проверяется функцией checkItemValid)
    //!         0 - если запрос был успешно отправлен
    virtual int  remove(ISqlTableItem::ptr row);

    //!
    //! \brief checkItemValid Метод для проверки, что элемент корректный
    //! \param item - Элемент
    //! \return Код причины, почему элемент не корректный, или 0 в противном случае
    virtual int  checkItemValid(ISqlTableItem::ptr item);

    //!
    //! \brief lastItemCheckString Метод для получения строки с объяснением
    //! некорректности элемента при последней проверке
    //! \return
    //!
    QString lastItemCheckString();

    //!
    //! \brief items
    //! \return Список всех элементов, сейчас загруженных из базы
    const QList<ISqlTableItem::ptr> items();

    //!
    //! \brief item Метод для получения конкретного элемента
    //! \param uuid - Идентификатор элемента
    //! \return Элемент
    ISqlTableItem::ptr item(const QString & uuid);

    //!
    //! \brief load Метод для загрузки элементов из БД
    //!
    void load();

    //!
    //! \brief unload Метод для выгрузки элементов из памяти
    //!
    void unload();

    //!
    //! \brief fullReload Полная перезагрузка всех элементов
    //!
    void fullReload();

    //!
    //! \brief updateModel Метод для обновления данных в модели представления
    //!
    //! Этот метод должен быть переопределен в классе-наследнике,
    //! иначе ваш менеджер будет являться виртуальным классом
    virtual void updateModel() = 0;

    //!
    //! \brief count
    //! \return Количество элементов, загруженных в память
    //!
    int count () const;

    //!
    //! \brief model
    //! \return Доступ к модели данных
    //!
    const QStandardItemModel * model() const;;

    //!
    //! \brief setModel Метод для задания модели данных
    //! \param newModel - Модель данных
    //!
    void setModel (QStandardItemModel * newModel);

    //!
    //! \brief tableName
    //! \return Название таблицы в БД
    //!
    const QString &tableName() const;

    //!
    //! \brief setTableName Метод для задания названия таблицы в БД
    //! \param newTableName - Новое название
    //!
    void setTableName(const QString &newTableName);

    //!
    //! \brief tableScheme
    //! \return Название схемы в БД
    //!
    const QString &tableScheme() const;

    //!
    //! \brief setTableScheme Метод для задания названия схемы БД
    //! \param newTableScheme - Новое название
    //!
    void setTableScheme(const QString &newTableScheme);

protected:
    //!
    //! \brief selectQuery Метод для создания SQL запроса SELECT
    //! \return
    //!
    virtual QString selectQuery();

    //!
    //! \brief insertQuery Метод для создания SQL запроса INSERT
    //! \param item - Элемент, который будет вставлен
    //! \return
    //!
    virtual QString insertQuery(ISqlTableItem::ptr item);

    //!
    //! \brief updateQuery Метод для создания SQL запроса UPDATE
    //! \param item - Элемент, который будет обновлен
    //! \return
    //!
    virtual QString updateQuery(ISqlTableItem::ptr item);

    //!
    //! \brief deleteQuery Метод для создания SQL запроса DELETE
    //! \param item - Элемент, который будет удален
    //! \return
    //!
    virtual QString deleteQuery(ISqlTableItem::ptr item);

    //!
    //! \brief parseSingleQuery Метод для десериализации элемента таблицы из
    //! его описания в формате Json
    //! \param record - Данные в формате Json
    //! \return Элемент таблицы
    //!
    //! Должен быть переопределен в классе-наследнике, иначе ваш менеджер
    //! будет являться виртуальным классом
    virtual ISqlTableItem::ptr parseSingleQuery(const QJsonObject & record) = 0;

    //!
    //! \brief autoParseQuery Метод для автоматической десериализации элемента таблицы
    //! из его описания в формате Json
    //! \param item - Элемент
    //! \param record - Данные в формате Json
    //! \return true/false - получилось или не получилось
    //!
    //! Чаще всего, в простых случаях, когда менеджер привязан только к одной
    //! таблице в БД, этот метод можно использовать внутри определения parseSingleQuery,
    //! чтобы его сократить
    //!
    //! Пример:
    //! <MySqlTableManager.cpp>
    //! -- ISqlTableItem::ptr MySqlTableManager::parseSingleQuery(const QJsonObject & record)
    //! -- {
    //! --     MySqlItem::ptr item = MySqlItem::create();
    //! --     autoParseQuery(item, record);
    //! --     return item;
    //! -- }
    //!
    bool autoParseQuery(ISqlTableItem::ptr item, const QJsonObject & record);

protected slots:
    //!
    //! \brief sendQuery Слот для отправки запроса в БД.
    //! Создает уникальный идентификатор для данного запроса, помещает его
    //! в список ожидаемых результатов и отправляет сигнал execQuerySignal
    //! \param query - Строка запроса
    //!
    void sendQuery(const QString & query);

    //!
    //! \brief onQueryFinished Слот обработки результата запроса в БД.
    //! Если идентификатор не совпадает ни с одним из ожидаемых, то
    //! результат игнорируется
    //! \param uuid - Идентификатор
    //! \param result - Результат запроса
    //!
    void onQueryFinished (const QUuid & uuid, QueryResult result);

    //!
    //! \brief onDBNotification Слот обработки уведомления из базы данных
    //!
    virtual void onDBNotification(const SqlNotification);

signals:
    //!
    //! \brief execQuerySignal Сигнал для отправки запроса в БД
    //!
    void execQuerySignal(const QUuid & uuid, const QString & query);

    //!
    //! \brief updated Сигнал того, что данные в менеджере обновились
    //!
    void updated();

    //!
    //! \brief updatedItem
    //!
    void updatedItem(ISqlTableItem::ptr item);

    //!
    //! \brief modelUpdated Сигнал того, что модель данных обновилась
    //!
    void modelUpdated();
};

