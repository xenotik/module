#ifndef SESSION_TABLE_FROM_DB_H
#define SESSION_TABLE_FROM_DB_H

#include <QSqlDatabase>
#include <QMap>
#include <QUuid>
#include <QDateTime>
#include "pgprocess.h"


using namespace std;
class session_table_from_DB
{
public:
    session_table_from_DB(pgProcess &DBsql);

    pgProcess DBsql;

    enum class RepeatPeriod {
        none,
        day,
        week,
        month,
        year,
    };

    enum class RepeatBefore {
        infinity,
        count,
        date,
    };

    enum class DayType : uint8_t {
        day,
        weekday,
        weekend,
        mon,
        tue,
        wed,
        thu,
        fri,
        sat,
        sun,
    };

    struct Period {
        QDateTime begin_date;
        RepeatPeriod type;
        int period;
        union{
            uint8_t weekDays;
            struct {
                int8_t day;
                DayType dayType;
            };
        };
    };

    struct Count {
        RepeatBefore type;
        union {
            int count;
            qint64 date;
        };
    };



    std::map<QUuid, QPair<Period, Count>> tableSeanse;
    QUuid id;

    std::map<QUuid, QDateTime> getNextSeance();
    QDateTime minRes(list<QDateTime> lr);

    void UpdateTable();

    std::map<QUuid, QDateTime> getANextSeance();

};

#endif // SESSION_TABLE_FROM_DB_H
