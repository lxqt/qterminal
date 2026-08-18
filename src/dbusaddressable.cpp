#include "dbusaddressable.h"

#include <QRegularExpression>

using namespace Qt::Literals::StringLiterals;

#ifdef HAVE_QDBUS
Q_DECLARE_METATYPE(QList<QDBusObjectPath>)

QString DBusAddressable::getDbusPathString()
{
    return m_path;
}

QDBusObjectPath DBusAddressable::getDbusPath()
{
    return QDBusObjectPath(m_path);
}
#endif

DBusAddressable::DBusAddressable(const QString& prefix, const QString& name)
{
    #ifdef HAVE_QDBUS
    QString suffix = name;
    if (suffix.isEmpty())
    {
        QString uuidString = QUuid::createUuid().toString();
        static const QRegularExpression regExp{u"[\\{\\}\\-]"_s};
        suffix = uuidString.replace(regExp, QString());
    }
    m_path = prefix + QLatin1Char('/') + suffix;
    #endif
}
