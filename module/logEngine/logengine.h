#pragma once
#include <memory>
#include <QString>
#include <QSqlDatabase>
#include <QDateTime>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <QMessageLogContext>
#include <QLoggingCategory>
#include <fstream>
#include <experimental/source_location>

class LogEngine
{
public:
  typedef std::shared_ptr<LogEngine> LogEngine_ptr_t;
  enum class log_type_t {
    error = 1,
    warning = 2,
    info = 3,
    dbg = 4,
    min = 1,
    max = 4,
  };
  enum class log_source_t {
    other = 0,
    sys = 1,
    db = 2,
    net = 3,
    p2p = 4,
    min = 1,
    max = 4,
  };


  static LogEngine_ptr_t instance();
  ~LogEngine();

private:
  struct LogMessage {
    QDateTime time;
    log_type_t type;
    log_source_t source;
    QString msg;
    QString filename;
    QString function;
    int line;
  };
  enum class State :uint16_t {
    _none,
    _started,
    _stopping,
    _stopped,
    _error
  };
  LogEngine() {};
  LogEngine(const LogEngine&) = delete;
  LogEngine& operator=(const LogEngine&) = delete;

  static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
public:
  static void add_msg(const QString& msg, log_source_t src = log_source_t::other, log_type_t type = log_type_t::dbg,
                        const std::experimental::source_location& location = std::experimental::source_location::current());
private:
  bool start();
  void stop();

  std::fstream logFile;
  QDate curDate;
  bool openLogFile();
  bool writeLogFile(const LogMessage* msg);
  bool writeServiceMsg(const QString &msg);

  State _state = State::_none;
  uint32_t maxQueueSize = 10000;
  std::queue<LogMessage*> messages;
  std::thread t;
  std::mutex m_cv;
  std::mutex m_q;
  std::condition_variable _cv;
  void push(LogMessage* msg);
  bool write(LogMessage* msg);
  LogMessage* get();
  void run();
  void _run();
};

Q_DECLARE_LOGGING_CATEGORY(logDB)
Q_DECLARE_LOGGING_CATEGORY(logNET)
Q_DECLARE_LOGGING_CATEGORY(logSYS)

