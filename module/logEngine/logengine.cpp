#include "logengine.h"
#include <QSqlQuery>
#include <QVariant>
#include <QMessageLogContext>
#include <QSqlError>
#include <QDebug>
#include <QDir>
#include <iostream>
#include <QRegularExpression>

using namespace std;

const string _sSource[] = {"OTHER", "SYS", "DB", "NET"};

Q_LOGGING_CATEGORY(logSYS, _sSource[1].c_str())
Q_LOGGING_CATEGORY(logDB, _sSource[2].c_str())
Q_LOGGING_CATEGORY(logNET, _sSource[3].c_str())

LogEngine::LogEngine_ptr_t LogEngine::instance() {
  static mutex m;
  static weak_ptr<LogEngine> curLogger;
  lock_guard lk(m);
  auto instance = curLogger.lock();
  if(!instance){
    instance = LogEngine_ptr_t(new LogEngine);
    if(!instance->start())
      instance.reset();
    else
      curLogger = instance;
  }
  return instance;
}

LogEngine::~LogEngine(){
  stop();
  while(!messages.empty()) {
    delete messages.front();
    messages.pop();
  }
  logFile.close();
}

bool LogEngine::start() {
  if (lock_guard lk(m_cv);_state == State::_started)
    return false;
  curDate = QDate::currentDate();
  if(!openLogFile())
    return false;
  if(!writeServiceMsg("Logger started."))
    return false;
  t = thread(&LogEngine::_run, this);
  for(;;) {
    lock_guard lk(m_cv);
    if (_state != State::_none) {
      if (_state == State::_started)
        break;
      return false;
    }
  }
  qInstallMessageHandler(LogEngine::messageHandler);
  _cv.notify_one();
  return true;
}

void LogEngine::stop() {
  qInstallMessageHandler(nullptr);
  if (lock_guard lk(m_cv);_state == State::_started) {
    _state = State::_stopping;
    _cv.notify_one();
  }
  if(t.joinable()) t.join();
  writeServiceMsg(_state == State::_error ? "Logger error" : "Logger stopped");
  logFile.close();
}

bool LogEngine::openLogFile() {
  QRegularExpression filter("20[2-9]\\d_((0[1-9]|1[012])_(0[1-9]|[12]\\d)|(0[13-9]|1[012])_30|(0[13578]|1[02])_31)_bp_service.log");
  QDir logDir("./logs/");

  if(logFile.is_open())
    logFile.close();

  if(!logDir.exists()) {
    if(!QDir().mkdir(logDir.path()))
      return false;
  }

  auto list = logDir.entryList(QDir::Files, QDir::Name).filter(filter);
  while(list.size() > 30) {
    logDir.remove(list.first());
    list.pop_front();
  }

  QString fileName = logDir.path() + curDate.toString("/yyyy_MM_dd") + "_bp_service.log";
  logFile.open(fileName.toStdString(), ios_base::out | ios_base::app);
  return(logFile.is_open());
}

bool LogEngine::writeLogFile(const LogMessage *msg) {
  const char c_log_type[] {"EWID"};
  static QString line{"[%0:%1] %2 %3%4"};
  static QString dbg_info{" (%0->%1[%3])"};
  auto newDate = QDate::currentDate();
  if(curDate != newDate) {
    curDate = newDate;
    if(!openLogFile())
      return false;
  }
  char c = (msg->type < log_type_t::min || msg->type > log_type_t::max) ?  'U' : c_log_type[static_cast<int>(msg->type)  - 1];
  auto& sSource = msg->source < log_source_t::min || msg->source > log_source_t::max ? _sSource[0] : _sSource[static_cast<int>(msg->source)];
  QString _dbg_info;
  if(!msg->function.isEmpty())
    _dbg_info = dbg_info.arg(msg->function, msg->filename).arg(msg->line);
  auto s = line.arg(c).arg(sSource.c_str(), 5).arg(msg->time.toString(Qt::ISODateWithMs), msg->msg, _dbg_info);
  logFile << s.toUtf8().constData() << endl;
//#ifdef _DEBUG
  std::cout << s.toLocal8Bit().constData() << endl;
//#endif
  return !logFile.bad();
}

bool LogEngine::writeServiceMsg(const QString& msg) {
  static const char line[]{"[S:     ] %1 %2"};
  auto newDate = QDate::currentDate();
  if(curDate != newDate) {
    curDate = newDate;
    if(!openLogFile())
      return false;
  }
  auto s = QString(line).arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs), msg);
  logFile << s.toUtf8().constData() << endl;
  return !logFile.bad();
}

//void LogEngine::log_db_execute_error(const QSqlQuery &q, const QString &sql) {
//  static const QString err{s_db_execute_error};
//  qCritical(logDB).noquote() << err.arg(q.lastError().text(), sql);
//}

//void LogEngine::log_db_preapre_error(const QSqlQuery &q, const QString &sql) {
//  static const QString err{s_db_prepare_error};
//  qCritical(logDB).noquote() << err.arg(q.lastError().text(), sql);
//}

void LogEngine::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
  auto _msg = new LogMessage;
  switch (type) {
    case QtMsgType::QtCriticalMsg:
    case QtMsgType::QtFatalMsg:
      _msg->type = log_type_t::error;
      break;
    case QtMsgType::QtInfoMsg:
      _msg->type = log_type_t::info;
      break;
    case QtMsgType::QtWarningMsg:
      _msg->type = log_type_t::warning;
      break;
    default:
      _msg->type = log_type_t::dbg;
      break;
  }
  if(context.category == _sSource[1])
    _msg->source = log_source_t::sys;
  else if(context.category == _sSource[2])
    _msg->source = log_source_t::db;
  else if(context.category == _sSource[3])
    _msg->source = log_source_t::net;
  else
    _msg->source = log_source_t::other;

  _msg->time = QDateTime::currentDateTime();
  _msg->msg = msg;
  _msg->filename = context.file;
  _msg->function = context.function;
  _msg->line = context.line;
  LogEngine::instance()->push(_msg);
}

void LogEngine::add_msg(const QString &msg, log_source_t src, log_type_t type, const experimental::source_location &location) {
  auto _msg = new LogMessage;

  _msg->type = type;
  _msg->source = src;
  _msg->time = QDateTime::currentDateTime();
  _msg->msg = msg;
  _msg->filename = location.file_name();
  _msg->function = location.function_name();
  _msg->line = location.line();
  if(auto logengine = LogEngine::instance())
    logengine->push(_msg);
}

void LogEngine::push(LogMessage *msg) {
  lock_guard lk(m_q);
  if (messages.size() < maxQueueSize) {
    messages.push(msg);
    _cv.notify_one();
  }
  else {
    if (messages.back()) {
      messages.push(nullptr);
    }
    delete msg;
  }
}

bool LogEngine::write(LogMessage *msg) {
  if(!writeLogFile(msg)) {
    delete msg;
    return false;
  }

  if(msg->source < log_source_t::min || msg->source > log_source_t::max) {
    delete msg;
    return true;
  }
  return false;
}

LogEngine::LogMessage *LogEngine::get() {
  lock_guard lk(m_q);
  if (!messages.empty()) {
    LogMessage* msg = messages.front();
    messages.pop();
    if(!msg) {
      msg = new LogMessage;
      msg->msg = "message buffer overflows, one or more messages skipped";
      msg->source = log_source_t::sys;
      msg->type = log_type_t::error;
      msg->time = QDateTime::currentDateTime();
    }
    return msg;
  }
  return nullptr;
}

void LogEngine::_run() {
  unique_lock lock(m_cv);


  for(_state = State::_started; _cv.wait(lock), _state == State::_started;) {
    while(auto msg = get()) {
      if (!write(msg)) {
        _state = State::_error;
        return;
      }
    }
  }
  while(auto msg = get()) {
    if (!write(msg)) {
      _state = State::_error;
      return;
    }
  }
_state = State::_stopped;
}
