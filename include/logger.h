#ifndef LOGGER_H
#define LOGGER_H
#include <fstream>
#include <string>
#include <sys/_types/_u_int32_t.h>

using std::string;

class Logger {
public:
  enum LogLevel { INFO, WARNING, ERROR };
  enum LogOut { STDOUT, FILE };
  Logger(LogLevel const &level, LogOut const &out, string const &fpath);
  Logger(LogLevel const &level, LogOut const &out);
  ~Logger();
  void outFilePath(string fpath);
  void level(LogLevel level);
  void entry(LogLevel const &level, std::string const &text);

private:
  LogLevel m_level;
  LogOut m_out;
  u_int32_t m_limit_buffer; // nr of entries before content is flushed to output
  u_int32_t m_current_buffer;
  struct {
    string path;
    u_int32_t limit;   // nr of entries in each file
    u_int32_t current; // nr of current entries
    std::ofstream handle;
  } m_file;
  void setFileHandler();
  void closeFile();
  void flushBuffer();
  string getTimeStamp();
};

#endif
