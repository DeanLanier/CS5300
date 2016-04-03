// Project Includes
#include "logger.hpp"

// Standard Includes
#include <iostream>

std::unique_ptr<logger::details::Logger> logger::details::Logger::m_pInstance;

logger::details::Logger::Logger(const std::string& output) : m_output(output), m_lineNum(1) {}

logger::details::Logger::~Logger()
{
  m_output.close();
}

bool logger::details::Logger::create(const std::string& output)
{
  m_pInstance.reset(new Logger(output));
  return m_pInstance->m_output.is_open();
}

int logger::details::Logger::getLineNum()
{
  return m_pInstance->m_lineNum;
}

void logger::details::Logger::incLineNum()
{
  m_pInstance->m_lineNum++;
}

void logger::details::Logger::log(const std::string& msg)
{
  m_pInstance->m_output << msg << std::endl;
}

bool logger::init(const std::string& output)
{
  return details::Logger::create(output);
}

int logger::getLineNumber()
{
  return details::Logger::getLineNum();
}

void logger::incLineNumber()
{
  details::Logger::incLineNum();
}

void logger::blankLine()
{
  details::Logger::log();
}

void logger::code(const std::string& snippet, const std::string& comment)
{
  std::string msg = "    " + snippet;
  if (!comment.empty())
  {
    if (snippet.size() < 12)
      msg += "\t";
    msg += "\t# " + comment;
  }
  details::Logger::log(msg);
}
void logger::comment(const std::string& msg)
{
  details::Logger::log("    # " + msg);
}

void logger::compileError(const std::string& msg)
{
  std::string str = "On line " + std::to_string(getLineNumber()) + " - " + msg;
  throw std::runtime_error(str);
}

void logger::compileWarning(const std::string& msg)
{
  std::cout << "Warning: On line " << getLineNumber() << " - " << msg << std::endl;
}

void logger::debug(const std::string& msg)
{
  details::Logger::log("\t\t\t\t\t\t\t# DEBUG: Line " + std::to_string(getLineNumber()) + ": " + msg);
}

void logger::error(const std::string& msg)
{
  std::cout << "Error: " << msg << std::endl;
}

void logger::info(const std::string& msg)
{
  std::cout << msg << std::endl;
}

void logger::label(const std::string& label, const std::string& code)
{
  std::string msg = label + ":";
  if (!code.empty())
    msg += " " + code;
  details::Logger::log(msg);
}

