// Primary Include
#include "tables.hpp"

// Project Includes
#include "logger.hpp"

// Standard Includes
#include <map>
#include <vector>

namespace
{
  using Table_t = std::map<std::string, tables::Symbol>;

  std::vector<Table_t> symbolTables(1);
  std::vector<std::string> stringTable;

  int getGpOffset(int size = 4)
  {
    static int gpOffset = 0;
    int offset = gpOffset;
    gpOffset += size;
    return offset;
  }

  std::string getTypeStr(Type type)
  {
    switch (type)
    {
    case TYPE_BOOL: return "boolean";
    case TYPE_CHAR: return "character";
    case TYPE_INT: return "integer";
    case TYPE_STRING: return "string";
    default: return "unknown type";
    }
  }

  tables::Symbol addSymbol(const std::string& name, const tables::Symbol& symbol)
  {
    for (auto it = symbolTables.rbegin(); it != symbolTables.rend(); ++it)
    {
      if (it->find(name) != it->end())
        logger::error("Multiple definitions: Symbol '" + name + "' is already defined");
    }

    symbolTables.back()[name] = symbol;
    return symbol;
  }

  tables::Symbol addIntSymbol(const std::string& name, Type type, bool isConst, int value)
  {
    int val = isConst ? value : getGpOffset();
    std::string loc = isConst ? "" : "$gp";
    return addSymbol(name, { type, isConst, loc, val });
  }
}

tables::Symbol tables::addBoolean(const std::string& name, bool isConst, int value)
{
  if (value < 0 || value > 1)
    value = 1;
  return addIntSymbol(name, TYPE_BOOL, isConst, value);
}

tables::Symbol tables::addCharacter(const std::string& name, bool isConst, int value)
{
  return addIntSymbol(name, TYPE_CHAR, isConst, value);
}

tables::Symbol tables::addInteger(const std::string& name, bool isConst, int value)
{
  return addIntSymbol(name, TYPE_INT, isConst, value);
}

tables::Symbol tables::addString(const std::string& name, const std::string& str)
{
  return addSymbol(name, { TYPE_STRING, true, addString(str), 0 });
}

std::string tables::addString(const std::string& str)
{
  std::string loc = "STR_" + std::to_string(stringTable.size());
  stringTable.push_back(str);
  return loc;
}

tables::Symbol tables::getSymbol(const std::string& name)
{
  for (auto it = symbolTables.rbegin(); it != symbolTables.rend(); ++it)
  {
    auto found = it->find(name);
    if (found != it->end()) return found->second;
  }
  // TODO: Create compile error
  logger::compileError("Symbol '" + name + "' was not declared");
}

//void tables::pushTable()
//{
//  symbolTables.push_back(Table_t());
//}
//
//void tables::popTable()
//{
//  symbolTables.pop_back();
//}

void tables::writeTables()
{
  // Write string table
  logger::blankLine();
  logger::code(".data");
  int idx = 0;
  for (auto&& str : stringTable)
    logger::label("STR_" + std::to_string(idx++), ".asciiz " + str);

  // Write symbol tables
  logger::blankLine();
  logger::code(".align 4", "Align on a word boundary");
  logger::label("GA");
  logger::code(".space " + std::to_string(getGpOffset(0)));
}
