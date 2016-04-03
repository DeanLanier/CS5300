#ifndef LOGGER_HPP
#define LOGGER_HPP

// STD Includes
#include <fstream>
#include <memory>
#include <string>

namespace logger
{
  namespace details
  {
    class Logger
    {
    public:
      ~Logger();
      
      static bool create(const std::string& output);
      static int getLineNum();
      static void incLineNum();
      static void log(const std::string& msg = "");

    private:
      Logger(const std::string& output);
      
      std::ofstream m_output;
      int m_lineNum;
      static std::unique_ptr<Logger> m_pInstance;
    };
  }

  bool init(const std::string& output);
  int getLineNumber();
  void incLineNumber();

  void blankLine();
  void code(const std::string& snippet, const std::string& comment = "");
  void comment(const std::string& msg);
  void compileError(const std::string& msg);
  void compileWarning(const std::string& msg);
  void debug(const std::string& msg);
  void error(const std::string& msg);
  void info(const std::string& msg);
  void label(const std::string& label, const std::string& code = "");
}

#endif

