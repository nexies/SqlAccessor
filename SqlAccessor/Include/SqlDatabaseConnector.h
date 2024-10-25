#pragma once
#include <QObject>
#include <QThread>
#include <QThreadPool>
#include <QSqlDatabase>
#include <QMutex>
#include <QUuid>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QQueue>
#include <QSqlDriver>
#include <QTextCodec>

#include "SqlNotification.h"


//!
//! \brief The QueryResult struct
//! Класс, в который записывается результат работы QSqlQuery
//!
//! \author Ivanov GD
//!
struct QueryResult
{
    QList<QJsonObject> records;
    bool isSelect;
    QSqlError error;
};

Q_DECLARE_METATYPE(QueryResult)
Q_DECLARE_METATYPE(QSqlDriver::NotificationSource)


//!
//! \brief The SqlDatabaseConnector class
//! Класс, который организует соединение с базой данных,
//! и исполнение запросов в отдельном потоке
//!
//! \author Ivanov GD
//!
class SqlDatabaseConnector : public QObject
{
    Q_OBJECT

public:
    //!
    //! \brief The State enum
    //! Текущее состояние коннектора
    enum State
    {
        Disconnected,
        Idle,
        Busy
    };
    Q_ENUM(State)

    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString connectionName READ connectionName WRITE setConnectionName)
    Q_PROPERTY(QString databaseName READ databaseName)
    Q_PROPERTY(QString hostName READ hostName)
    Q_PROPERTY(int port READ port)
    Q_PROPERTY(QString username READ username)
    Q_PROPERTY(QString password READ password)

public:
    //!
    //! \brief SqlDatabaseConnector
    //! \param parent
    //! Конструктор
    SqlDatabaseConnector(QObject * parent = nullptr);

    //!
    //! \brief SqlDatabaseConnector
    //! \param baseHost
    //! \param port
    //! \param baseName
    //! \param parent
    //!
    //! Конструктор, который так же устанавливает в соответствующие
    //! поля информацию про БД
    SqlDatabaseConnector(const QString baseHost, int port, const QString baseName,
                         QObject * parent = nullptr);

    //!
    //! Деструктор
    ~SqlDatabaseConnector();

    //!
    //! \brief databaseName
    //! \return Название базы данных
    //!
    const QString databaseName() const;

    //!
    //! \brief hostName
    //! \return Адрес сервера
    //!
    const QString hostName() const;

    //!
    //! \brief port
    //! \return Порт сервера
    //!
    int           port() const;

    //!
    //! \brief username
    //! \return Имя пользователя
    //!
    const QString username() const;

    //!
    //! \brief password
    //! \return Пароль
    //!
    const QString password() const;

    //!
    //! \brief connectionName
    //! \return Название подключения.
    //! Если не указывать специально, то название будет
    //! совпадать с названием базы данных
    const QString connectionName () const;

    //!
    //! \brief setConnectionName Метод установки названия подключения
    //! \param newConnectionName - Новое значение
    //!
    void setConnectionName(const QString &newConnectionName);

    //!
    //! \brief isOpen
    //! \return true/false - Подключено или нет
    //!
    bool isOpen();

    //!
    //! \brief connectToBase Метод подключения к базе данных
    //! \param username - Имя пользователя
    //! \param password - Пароль
    //! \return true/false - Удалось подключиться или нет
    //!
    //! Данные hostName, port и databaseName должны быть указаны до вызова
    //! этой функции
    bool connectToBase(const QString & username, const QString & password);

    //!
    //! \brief connectToBase Метод подключения к базе данных
    //! \param host - Адрес сервера
    //! \param port - Порт
    //! \param baseName - Название базы данных
    //! \param username - Имя пользователя
    //! \param password - Пароль
    //! \return true/false - Удалось подключиться или нет
    //!
    bool connectToBase(const QString & host, int port,
                       const QString & baseName, const QString & username, const QString & password);

    //!
    //! \brief disconnectFromBase Метод для разрыва существующего соединения
    //! \return true/false - Удалось или нет
    //!
    bool disconnectFromBase();

    //!
    //! \brief state
    //! \return Текущее состояние
    //!
    const State &state() const;

    //!
    //! \brief codec
    //! \return Кодировщик
    //! Возвращает nullptr, если кодировщик не был установлен
    QTextCodec * const codec () const;

    //!
    //! \brief setCodec Метод для установки кодировщика текста
    //! \param codec - указатель на кодировщик
    //! Чтобы убрать кодировщик, передайте в качества параметра nullptr
    void setCodec (QTextCodec * codec);


public slots:
    //!
    //! \brief sendQuery Слот для отправки запроса в базу данных.
    //! \param uuid - Уникальный идентификатор запроса
    //! \param query - Текст запроса
    //!
    void sendQuery(const QUuid & uuid, const QString & query);

protected slots:
    //!
    //! \brief onSendQuery Слот для отправки запроса в базу данных.
    //! Этот слот нельзя подключить, потому что он объявлен с модификатором доступа protected
    //! Используется для внутренней логики
    //! \param uuid - Уникальный идентификатор запроса
    //! \param query - Текст запроса
    //!
    void onSendQuery(const QUuid & uuid, const QString query);

    //!
    //! \brief onQueryFinished Слот-обработчик окончания запроса
    //! \param uuid - Уникальный идентификатор запроса
    //! \param res - Результат запроса
    //!
    void onQueryFinished(const QUuid & uuid, QueryResult res);

    //!
    //! \brief onDBNotify Слот-обработчик уведомления из базы даных
    //! Привязывается к сигналу QSqlDriver::notification
    //! \param name - Название уведомления
    //! \param source - Источник уведомления
    //! \param payload - Данные
    //!
    void onDBNotify (const QString & name, QSqlDriver::NotificationSource source, const QVariant & payload);

signals:

    //!
    //! \brief sendQuerySignal Сигнал отправки запроса
    //! Используется для внутренней логики
    //! \param uuid - Уникальный идентификатор
    //! \param query - Текст запроса
    //!
    void sendQuerySignal(const QUuid & uuid, const QString & query);

    //!
    //! \brief queryFinishedSignal Сигнал завершения запроса
    //! \param uuid - Уникальный идентификатор запроса
    //! \param res - Результат запроса
    //!
    void queryFinishedSignal(const QUuid &, QueryResult res);

    //!
    //! \brief queryErrorSignal Сигнал того, что запрос вернулся с ошибкой
    //! \param uuid - Уникальный идентификатор запроса
    //! \param error - Ошибка запроса
    //!
    void queryErrorSignal(const QUuid &, QSqlError error);

    //!
    //! \brief dbNotification Сигнал того, что пришло уведомление из базы данных
    //! \param notification - Уведомление
    //!
    void dbNotification(const SqlNotification notification);

    //!
    //! \brief stateChanged
    //! Сигнал того, что изменилось состояние коннектора
    //!
    void stateChanged();

    //!
    //! \brief connected
    //! Сигнал того, что соединение открылось
    void connected();

    //!
    //! \brief disconnected
    //! Сигнал того, что соединение закрылось
    void disconnected();

private:

    //!
    //! \brief _mutex
    //! Мютекс.
    //! Не знаю зачем, но пусть будет
    QMutex * _mutex { nullptr };
    //!
    //! \brief _database
    //! Объект, представляющий базу данных
    QSqlDatabase _database;
    //!
    //! \brief _query
    //! Объект, представляющий запрос в базу данных
    QSqlQuery * _query { nullptr };
    //!
    //! \brief _thread
    //! Поток, в котором открыто соединение
    QThread _thread;
    //!
    //! \brief _queue
    //! Очередь запросов на отправку
    QQueue<QPair<QUuid, QString>> _queue;
    //!
    //! \brief debug
    //! Режим дебаг. (Выводит информацию в консоль, если true)
    bool debug { false };
    //!
    //! \brief
    //! Кодировщик. Для доступа к базам данных с кодировкой не UTF
    QTextCodec * _codec { nullptr };
    //!
    //! \brief m_connectionName
    //! Название соединения
    QString m_connectionName;
    //!
    //! \brief m_state
    //! Состояние коннектора
    State   m_state { Disconnected };
};

