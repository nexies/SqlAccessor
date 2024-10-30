#ifndef SQL_ACCCESSOR_DEFS_H
#define SQL_ACCCESSOR_DEFS_H

#define DECLARE_SQL_FIELD(type, name) \
    private: \
    Q_PROPERTY(type name READ name WRITE set_##name) \
    type m_##name; \
    inline type name () const { return m_##name; } \
    inline void set_##name (const type & val) { m_##name = val; } \



#define SQL_TABLE_ITEM_UNKNOWN_USAGE  \
QRegularExpression rx ("(\\w+)::(\\w+)"); \
    QRegularExpressionMatch match = rx.match(Q_FUNC_INFO); \
    qWarning().noquote() << QString("[%1][%2] : РЅРµ СЂРµР°Р»РёР·РѕРІР°РЅР°!").arg(match.captured(1), match.captured(2));

#endif // SQL_ACCCESSOR_DEFS_H
