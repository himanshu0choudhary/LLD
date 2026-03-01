#include "bits/stdc++.h"

using namespace std;

enum LogLevel
{
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
};

struct Log
{
    LogLevel level;
    string message;
};

class LogFormatter
{
public:
    virtual string format(Log log) = 0;
    virtual ~LogFormatter() = default;
};

class SimpleLogFormatter : public LogFormatter
{
    map<LogLevel, string> levelMap;

public:
    SimpleLogFormatter()
    {
        levelMap[DEBUG] = "DEBUG";
        levelMap[INFO] = "INFO";
        levelMap[WARNING] = "WARNING";
        levelMap[ERROR] = "ERROR";
    }

    string format(Log log) override
    {
        return levelMap[log.level] + " -> " + log.message;
    }
};

class LogSink
{
public:
    virtual void write(const string &message) = 0;
    virtual ~LogSink() = default;
};

class SysOutLogSync : public LogSink
{
    mutex mtx;

public:
    void write(const string &message) override
    {
        lock_guard<mutex> lock(mtx);
        cout << message << endl;
    }
};

class Logger
{
private:
    Logger(shared_ptr<LogFormatter> formatter, vector<shared_ptr<LogSink>> sinks) : formatter(formatter), sinks(sinks) {}

    atomic<LogLevel> logLevel{WARNING};

    shared_ptr<LogFormatter> formatter;
    vector<shared_ptr<LogSink>> sinks;

public:
    static shared_ptr<Logger> getLogger(shared_ptr<LogFormatter> formatter, vector<shared_ptr<LogSink>> sinks)
    {
        static shared_ptr<Logger> logger;
        static once_flag flag;

        call_once(flag, [formatter, sinks]()
                  { logger = shared_ptr<Logger>(new Logger(formatter, sinks)); });

        return logger;
    }

    void setLevel(LogLevel level)
    {
        logLevel.store(level);
    }

    void log(string message, LogLevel level)
    {
        if (level < logLevel.load(memory_order_relaxed))
            return;

        auto out = formatter->format(Log{level, message});
        for (const auto &sink : sinks)
            sink->write(out);
    }

    void log(string message)
    {
        log(message, logLevel.load(memory_order_relaxed));
    }
};

int main()
{
    auto formatter = shared_ptr<LogFormatter>(new SimpleLogFormatter());
    vector<shared_ptr<LogSink>> sinks = {shared_ptr<LogSink>(new SysOutLogSync())};
    auto logger = Logger::getLogger(formatter, sinks);

    logger->log("test");
    logger->log("test", LogLevel::DEBUG);
}