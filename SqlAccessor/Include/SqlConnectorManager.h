#pragma once
#include "SqlDatabaseConnector.h"
#include <QMap>


//!
//! \brief The SqlConnectorManager class
//! \author Ivanov GD
//!
//! Класс, отвечающий за создание и менеджмент
//! соединений с базами данных в программе
//!
//! Реализован по паттерну Singleton
//! один на всю программу, обращение через SqlConnectorManager::instance()
class SqlConnectorManager
{
    //!
    //! \brief SqlConnectorManager
    //! Конструктор
    SqlConnectorManager();
    //!
    //! Деструктор
    ~SqlConnectorManager();
    //!
    //! \brief _instance
    //! Экземпляр класса
    static SqlConnectorManager * _instance;
    //!
    //! \brief _connectors
    //! Коннекторы к базам данных
    QMap<QString, SqlDatabaseConnector *> _connectors;

public:
    //!
    //! \brief instance
    //! \return Экземпляр класса
    //!
    static SqlConnectorManager &instance();
    //!
    //! \brief freeInstance Метод освобождения памяти от экземпляра класса
    //!
    static void freeInstance ();

    //!
    //! \brief connectionNames
    //! \return Список существующих соединений
    //!
    QStringList connectionNames();

    //!
    //! \brief getConnector Метод, возвращающий коннектор по его имени
    //! Если соединения с таким названием нет - возвращает nullptr
    //! \param connectionName - Имя соединения
    //! \return коннектор
    //!
    SqlDatabaseConnector * getConnector(const QString & connectionName);

    //!
    //! \brief addConnection Метод добавления нового соединения
    //! \param baseName - название базы
    //! \param host - Адрес сервера
    //! \param port - Порт
    //! \param connectionName - Имя соединения. Если не указано, будет
    //! сгенерированно автоматически
    //! \return true/false - получилось добавить или нет
    //!
    bool addConnection (const QString & baseName,
                        const QString & host, int port,
                        QString connectionName = QString());

    //!
    //! \brief removeConnection Метод удаления существующего соединения
    //! \param connectionName - Имя соединения
    //!
    void removeConnection (const QString connectionName);

    //!
    //! \brief openConnection Метод открытия соединения.
    //! \param connectionName - Имя соединения
    //! \param username - Имя пользователя
    //! \param password - Пароль
    //! \return true/false - Удалось подкючиться или нет
    //!
    bool openConnection(const QString connectionName,
                        const QString & username, const QString & password);

    //!
    //! \brief closeConnection Метод закрытия соединения
    //! \param connectionName - Имя соединения
    //! \return true/fasle - Удалось отключиться или нет
    //!
    bool closeConnection(const QString & connectionName);
};

