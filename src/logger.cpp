#include "logger.h"
#include <ctime>
#include <iostream>
#include <sstream>
#include <stdexcept>

Logger::Logger(LogLevel const &level, LogOut const &out, string const &fpath)
    : m_level(level), m_out(out), m_limit_buffer(10), m_current_buffer(0) {

  if (LogOut::FILE == m_out) {
    m_file.path = fpath;
    setFileHandler();
  }
}

Logger::Logger(LogLevel const &level, LogOut const &out)
    : m_level(level), m_out(out), m_limit_buffer(10), m_current_buffer(0) {

  if (LogOut::FILE == m_out) {
    m_file.path = "logs/";
    setFileHandler();
  }
}

Logger::~Logger() { closeFile(); }

void Logger::level(LogLevel level) { m_level = level; }

void Logger::outFilePath(string fpath) {
  if (LogOut::FILE != m_out) {
    m_out = LogOut::FILE;
  } else {
    closeFile();
  }
  m_file.path = fpath;
  setFileHandler();
}

void Logger::closeFile() {
  if (LogOut::FILE == m_out && m_file.handle) {
    m_file.handle.close();
    m_file.current = 0;
  }
}

void Logger::entry(LogLevel const &level, std::string const &text) {
  if (level >= m_level) {
    string log_entry;
    switch (level) {
    case LogLevel::INFO:
      log_entry = "<INF>[";
      break;
    case LogLevel::WARNING:
      log_entry = "<WRN>[";
      break;
    case LogLevel::ERROR:
      log_entry = "<ERR>[";
      break;
    }
    log_entry.append(getTimeStamp());
    log_entry.append("]");
    log_entry.append(text);
    if (LogOut::FILE == m_out) {
      m_file.handle << log_entry << '\n';
      m_file.current += 1;
      if (m_file.current >= m_file.limit) {
        m_file.handle.close();
        setFileHandler();
      }
    } else if (LogOut::STDOUT == m_out) {
      std::cout << log_entry << std::endl;
    }
  }
}

void Logger::setFileHandler() {
  string fpath = m_file.path;
  fpath.append(getTimeStamp());
  fpath.append(".log");
  m_file.handle.open(fpath);
  if (!m_file.handle) {
    throw std::runtime_error("Issues opening file: " + fpath);
  }
  m_file.current = 0;
  m_file.limit = 10000;
}
string Logger::getTimeStamp() {
  std::time_t time = std::time(NULL);
  std::tm *tm = std::localtime(&time);
  std::stringstream tmstmp;
  tmstmp << 'D' << (1900 + tm->tm_year) << tm->tm_mon << tm->tm_mday;
  tmstmp << 'T' << tm->tm_hour << ':' << tm->tm_min << ':' << tm->tm_sec;
  return tmstmp.str();
}
