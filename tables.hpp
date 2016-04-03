#ifndef CS5300_TABLES_HPP
#define CS5300_TABLES_HPP

// Project Includes
#include "compiler.hpp"

// Standard Includes
#include <string>

namespace tables
{
  struct Symbol
  {
    Type type;
    bool isConst;
    std::string location;
    int value;
  };

  //void addArray(const std::string& name, int value, int size, bool isConst);
  Symbol addBoolean(const std::string& name, bool isConst = false, int value = 0);
  Symbol addCharacter(const std::string& name, bool isConst = false, int value = 0);
  Symbol addInteger(const std::string& name, bool isConst = false, int value = 0);
  Symbol addString(const std::string& name, const std::string& str);
  std::string addString(const std::string& str);

  Symbol getSymbol(const std::string& name);

  //void pushTable();
  //void popTable();

  void writeTables();
}

#endif
