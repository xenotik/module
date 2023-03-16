#include "session_table_from_db.h"
#include "qsqlquery.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QDate>
#include <QTime>

session_table_from_DB::session_table_from_DB(pgProcess &DBsql)
{
    this->DBsql = DBsql;
    UpdateTable();
}

//std::map<QUuid, QDateTime> session_table_from_DB::getNextSeance()
//{
//    auto dttm = QDateTime::currentDateTime();
//    list<QDateTime> resList;

//    for(const auto &it : tableSeanse)
//    {
//        QDateTime res;
//        if(it.second.first.begin_date > dttm)
//            continue;
//        else
//        {  switch (it.second.first.type) {
//            case RepeatPeriod::day:{
//                if(it.second.first.period){
//                    auto days = it.second.first.begin_date.date().daysTo(dttm.date());
//                    if(it.second.first.begin_date.time() < QTime::currentTime())
//                        days++;
//                    if(days % it.second.first.period)
//                        days = (1 + days / it.second.first.period) * it.second.first.period;
//                    res = QDateTime(it.second.first.begin_date.date().addDays(days), it.second.first.begin_date.time());
//                }
//                else{
//                    auto days = it.second.first.begin_date.daysTo(dttm);
//                    if(it.second.first.begin_date.time() < QTime::currentTime())
//                        days++;
//                    auto periods  = days / 7;
//                    if(auto day = days % 7; day < 7){
//                        auto dayOfWeek = dttm.date().dayOfWeek() - 1;
//                        for(auto i = day; i < 7; i++){
//                            if(dayOfWeek < 6)
//                            {
//                                res = it.second.first.begin_date.addDays(periods * 7 + i);
//                                break;
//                            }
//                            if(++dayOfWeek >= 7)
//                                dayOfWeek = 0;
//                        }
//                    }
//                    if(!res.isValid() || res < QDateTime::currentDateTime())
//                    {
//                        res.setDate(it.second.first.begin_date.date().addDays(++periods * 7));
//                        auto dayOfWeek = res.date().dayOfWeek() - 1;
//                        for(auto i = 0; i < 7; i++){
//                            if(dayOfWeek < 6)
//                            {
//                                res = it.second.first.begin_date.addDays(periods * 7 + i);
//                                break;
//                            }
//                            if(++dayOfWeek >= 7)
//                                dayOfWeek = 0;
//                        }
//                    }
//                    break;
//                }
//            }
//            case RepeatPeriod::week:{
//                auto days = it.second.first.begin_date.daysTo(dttm);
//                if(it.second.first.begin_date.time() < QTime::currentTime())
//                    days++;
//                auto periods  = days / (7 * it.second.first.period);
//                if(auto day = days % (7 * it.second.first.period); day < 7){
//                    auto dayOfWeek = dttm.date().dayOfWeek() - 1;
//                    for(auto i = day; i < 7; i++){
//                        if(1 << dayOfWeek & it.second.first.weekDays)
//                        {
//                            res =  it.second.first.begin_date.addDays(periods * 7 * it.second.first.period + i);
//                            break;
//                        }
//                        if(++dayOfWeek >= 7)
//                            dayOfWeek = 0;
//                    }
//                }
//                if(!res.isValid() || res < QDateTime::currentDateTime())
//                {
//                    res.setDate(it.second.first.begin_date.date().addDays(++periods * 7 *  it.second.first.period));
//                    auto dayOfWeek = res.date().dayOfWeek() - 1;
//                    for(auto i = 0; i < 7; i++){
//                        if(1 << dayOfWeek &  it.second.first.weekDays)
//                        {
//                            res = it.second.first.begin_date.addDays(periods * 7 *  it.second.first.period + i);
//                            break;
//                        }
//                        if(++dayOfWeek >= 7)
//                            dayOfWeek = 0;
//                    }
//                }
//                break;
//            }
//            case RepeatPeriod::month:{
//                auto _d = it.second.first.begin_date.date();
//                if(_d.day() > it.second.first.day)
//                    _d = _d.addMonths(1);
//                _d = QDate(_d.year(), _d.month(), 1);

//                auto mons = (dttm.date().year() - _d.year()) * 12 + dttm.date().month() - _d.month();
//                if(mons % it.second.first.period)
//                    mons = (1 + mons / it.second.first.period) + it.second.first.period;
//                _d = _d.addMonths(mons);
//                QDateTime ndt(QDate(_d.year(), _d.month(), qMin(static_cast<int>(it.second.first.day), _d.daysInMonth())), it.second.first.begin_date.time());
//                if(ndt <= dttm)
//                    ndt = ndt.addMonths(it.second.first.period);
//                res = ndt;
//            }
//            case RepeatPeriod::none:
//            {
//                if(it.second.first.begin_date < QDateTime::currentDateTime())
//                    res = it.second.first.begin_date;
//            }
//            default:
//                break;
//            }
//        }
//        resList.push_back(res);
//    }
//    return minRes(resList);
//}

QDateTime session_table_from_DB::minRes(list<QDateTime> lr)
{
    QDateTime minDT = lr.back();
    for(const auto& it : lr)
    {
        if(it < minDT && it > QDateTime::currentDateTime())
            minDT = it;
    }
    return minDT;
}

void session_table_from_DB::UpdateTable()
{
    QString s = "Select id, abonent_id, begin_date, name, retry from public.seance_table";
    QSqlQuery q = DBsql.returnQuery(s);
    Period period;
    Count count;
    do{
        QUuid id = q.value("id").toUuid();
        period.type = RepeatPeriod::none;
        period.begin_date = q.value("begin_date").toDateTime();
        auto joRetry = QJsonDocument::fromJson(q.value("retry").toByteArray()).object();

        if(!joRetry.isEmpty()) {
            auto s_type = joRetry.value("type").toString().toLower();
            if(s_type == "day"){
                period.period = joRetry.value("period").toInt(-1);
                if(period.period >= 0)
                    period.type = RepeatPeriod::day;
            }
            else if(s_type == "week") {
                period.period = joRetry.value("period").toInt();
                period.type = RepeatPeriod::week;
                if(period.period > 0) {
                    period.weekDays = 0;
                    for(auto&& wd : joRetry.value("week_days").toArray()){
                        if(unsigned week_day = wd.toInt(); --week_day < 7)
                            period.weekDays |= 1 << week_day;
                    }
                }
            }
            else if(s_type == "month") {
                period.type = RepeatPeriod::month;
                period.day = joRetry.value("day").toInt();
                period.period = joRetry.value("period").toInt();
                if(period.day - 1 < 31 && period.period > 0)
                    period.type = RepeatPeriod::month;
            }
        }
        tableSeanse[id] = {period, count};
    }while(q.next());
}

std::map<QUuid, QDateTime> session_table_from_DB::getANextSeance()
{
    std::map<QUuid, QDateTime> result;
    auto dttm = QDateTime::currentDateTime();
    for(const auto &it : tableSeanse)
    {
        auto id = DBsql.returnQuery(QString("select abonent_id from seance_table where id = '%1'").arg(it.first.toString())).value(0).toUuid();
         QDateTime res;

        auto d = dttm.date();
        if(dttm.time() >= it.second.first.begin_date.time())
            d = d.addDays(1);
        switch (it.second.first.type) {

        case RepeatPeriod::none:
            if(it.second.first.begin_date > QDateTime::currentDateTime())
                res = it.second.first.begin_date;
            break;
        case RepeatPeriod::day:{
            if(it.second.first.period){
                auto days = it.second.first.begin_date.date().daysTo(d);
                if(days % it.second.first.period)
                    days = (1 + days / it.second.first.period) * it.second.first.period;
                res = QDateTime(it.second.first.begin_date.date().addDays(days), it.second.first.begin_date.time());
            }
            else{
                auto days = it.second.first.begin_date.date().daysTo(d);
                auto periods  = days / 7;
                if(auto day = days % 7; day < 7){
                    auto dayOfWeek = d.dayOfWeek() - 1;
                    for(auto i = day; i < 7; i++){
                        if(dayOfWeek <6)
                        {
                            res = it.second.first.begin_date.addDays(periods * 7 + i);
                            break;
                        }
                        if(++dayOfWeek >= 7)
                            dayOfWeek = 0;
                    }
                }
                d = it.second.first.begin_date.date().addDays(++periods * 7);
                auto dayOfWeek = d.dayOfWeek() - 1;
                for(auto i = 0; i < 7; i++){
                    if(dayOfWeek <6)
                    {
                        res = it.second.first.begin_date.addDays(periods * 7 + i);
                        break;
                    }
                    if(++dayOfWeek >= 7)
                        dayOfWeek = 0;
                }
            }
            break;
        }

        case RepeatPeriod::week:{
            auto days = it.second.first.begin_date.date().daysTo(d);
            auto periods  = days / (7 * it.second.first.period);
            if(auto day = days % (7 * it.second.first.period); day < 7){
                auto dayOfWeek = d.dayOfWeek() - 1;
                for(auto i = day; i < 7; i++){
                    if(1 << dayOfWeek & it.second.first.weekDays)
                    {
                        res = it.second.first.begin_date.addDays(periods * 7 * it.second.first.period + i);
                        break;
                    }
                    if(++dayOfWeek >= 7)
                        dayOfWeek = 0;
                }
            }
            d = it.second.first.begin_date.date().addDays(++periods * 7 * it.second.first.period);
            auto dayOfWeek = d.dayOfWeek() - 1;
            for(auto i = 0; i < 7; i++){
                if(1 << dayOfWeek & it.second.first.weekDays)
                {
                    res = it.second.first.begin_date.addDays(periods * 7 * it.second.first.period + i);
                    break;
                }
                if(++dayOfWeek >= 7)
                    dayOfWeek = 0;
            }
            break;
        }
        case RepeatPeriod::month:{
            auto _d = it.second.first.begin_date.date();
            if(_d.day() > it.second.first.day)
                _d = _d.addMonths(1);
            _d = QDate(_d.year(), _d.month(), 1);

            auto mons = (d.year() - _d.year()) * 12 + d.month() - _d.month();
            if(mons % it.second.first.period)
                mons = (1 + mons / it.second.first.period) + it.second.first.period;
            _d = _d.addMonths(mons);
            QDateTime ndt(QDate(_d.year(), _d.month(), qMin(static_cast<int>(it.second.first.day), _d.daysInMonth())), it.second.first.begin_date.time());
            if(ndt <= dttm)
                ndt = ndt.addMonths(it.second.first.period);
            res = ndt;
            break;
        }
            //            case RepeatPeriod::year:
            //                break;

        default:
            break;
        }
        if(res.isValid())
            if(result[id] > res or !result[id].isValid())
                result[id] = res;
    }
    return result;
}
