// Project Includes
#include "compiler.hpp"
#include "logger.hpp"
#include "tables.hpp"

 // Standard Includes
#include <map>
#include <sstream>
#include <vector>

// prototype of bison-generated parser function
int yyparse();

namespace
{
  // TODO: Move these global variables into a singleton (maybe)
  int curSymbol = 0;
  int loopCounter = 0;
  std::vector<std::string> idList;
  std::vector<std::pair<std::string, int>> loopList;
  
  void writeMain()
  {
    logger::code(".globl main");
    logger::code(".text");
    logger::label("main");
    logger::code("la $gp, GA");
    logger::code("j prog");

    // TODO: This needs to be moved when functions are supported
    logger::label("prog");
  }

  std::string getExprStr(Expr* expr)
  {
    std::string str = "Expr(";
    if (expr)
      str += std::to_string(expr->exprNum);
    else
      str += "empty";
    str += ")";
    return str;
  }

  std::string getRegStr(Expr* expr)
  {
    return std::to_string(expr->intVal) + "(" + expr->strVal + ")";
  }

  std::string getInstStr(const std::string& inst, const std::string& arg1, const std::string& arg2 = "", const std::string& arg3 = "")
  {
    // TODO: Format so it looks nicer
    std::string str = inst + " " + arg1;
    if (!arg2.empty()) str += ", " + arg2;
    if (!arg3.empty()) str += ", " + arg3;
    return str;
  }

  std::string getArithInstStr(const std::string& inst, Expr* expr1, Expr* expr2, Expr* expr3 = nullptr, bool immediate = false)
  {
    if (expr3)
    {
      if (immediate)
        return getInstStr(inst, expr1->reg->getName(), expr2->reg->getName(), std::to_string(expr3->intVal));
      return getInstStr(inst, expr1->reg->getName(), expr2->reg->getName(), expr3->reg->getName());
    }
    if (immediate)
      return getInstStr(inst, expr1->reg->getName(), std::to_string(expr2->intVal));
    return getInstStr(inst, expr1->reg->getName(), expr2->reg->getName());
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

  void printExpr(const std::string& expr)
  {
    logger::debug("Expr(" + std::to_string(curSymbol) + "): " + expr);
  }

  void addBoolVars()
  {
    for (auto&& id : idList)
      tables::addBoolean(id);
  }

  void addCharVars()
  {
    for (auto&& id : idList)
      tables::addCharacter(id);
  }

  void addIntVars()
  {
    for (auto&& id : idList)
      tables::addInteger(id);
  }

  void addStringVars()
  {
    // TODO: Check if strings can be variables - get the actual string instead of "blah"
    for (auto&& id : idList)
      tables::addString(id, "blah");
  }

  void checkType(Expr* expr)
  {
    if (expr->type == TYPE_STRING)
      logger::compileError("Strings are immutable");
  }

  void checkTypes(Expr* lhs, Expr* rhs)
  {
    if (lhs->type != rhs->type)
      logger::compileError("Incompatible types: '" + getTypeStr(lhs->type) + "' does not match '" + getTypeStr(rhs->type) + "'");

    checkType(lhs);
    checkType(rhs);
  }

  void pushLoop(const std::string& id = "")
  {
    loopList.push_back({ id, loopCounter++ });
  }

  void popLoop()
  {
    if (!loopList.empty())
      loopList.pop_back();
  }

  Expr* getLoopCounter()
  {
    return loadExpr(lvalueExpr(loopList.back().first));
  }

  std::string getLoopLabel(const std::string& label)
  {
    return label + "_" + std::to_string(loopList.back().second);
  }

  void loadImmediate(Expr* expr)
  {
    if (!expr->reg)
    {
      expr->reg = Register::allocate();
      logger::code(getInstStr("li", expr->reg->getName(), std::to_string(expr->intVal)));
    }
  }
}

void startProgram()
{
  tables::addBoolean("false", true, 0);
  tables::addBoolean("FALSE", true, 0);
  tables::addBoolean("true", true, 1);
  tables::addBoolean("TRUE", true, 1);
  writeMain();
  yyparse();
}

void endProgram()
{
  tables::writeTables();
}

void addConst(const std::string& id, Expr* expr)
{
  logger::debug("Const " + getTypeStr(expr->type) + " " + id + " = " + getExprStr(expr));


  switch (expr->type)
  {
  case TYPE_BOOL:
    tables::addBoolean(id, true, expr->intVal);
    break;
  case TYPE_CHAR:
    tables::addCharacter(id, true, expr->intVal);
    break;
  case TYPE_INT:
    tables::addInteger(id, true, expr->intVal);
    break;
  case TYPE_STRING:
    tables::addString(id, expr->strVal);
    break;
  }

  delete expr;
}

void addId(const std::string& id)
{
  idList.push_back(id);
}

void addVars(int type)
{
  Type realType = static_cast<Type>(type);
  for (auto&& id : idList)
    logger::debug("Adding Variable: " + getTypeStr(realType) + " " + id);

  switch (realType)
  {
  case TYPE_BOOL:
    addBoolVars();
    break;
  case TYPE_CHAR:
    addCharVars();
    break;
  case TYPE_INT:
    addIntVars();
    break;
  case TYPE_STRING:
    addStringVars();
    break;
  }
  idList.clear();
}

int simpleType(const std::string& id)
{
  if (id == "boolean" || id == "BOOLEAN") return TYPE_BOOL;
  if (id == "char" || id == "CHAR") return TYPE_CHAR;
  if (id == "integer" || id == "INTEGER") return TYPE_INT;
  if (id == "string" || id == "STRING") return TYPE_STRING;
  logger::compileError("Unknown Type: Symbol '" + id + "' is not a valid type");
}

void assignExpr(Expr* lhs, Expr* rhs)
{
  logger::debug("ASSIGN " + getExprStr(lhs) + " = " + getExprStr(rhs));

  checkTypes(lhs, rhs);
  if (lhs->isConst)
    logger::compileError("Invalid L-value: symbol '" + lhs->name + "' should be non-const");

  loadImmediate(rhs);
  logger::code(getInstStr("sw", rhs->reg->getName(), getRegStr(lhs)), "Assign to " + getTypeStr(lhs->type) + " '" + lhs->name + "'");

  delete lhs;
  delete rhs;
}

void readExpr(Expr* expr)
{
  logger::debug("READ " + getExprStr(expr));

  checkType(expr);
  if (expr->isConst)
    logger::compileError("Invalid L-value: symbol '" + expr->name + "' should be non-const");

  switch (expr->type)
  {
  case TYPE_BOOL:
  case TYPE_INT:
    logger::code(getInstStr("li", "$v0", "5"), "Read " + getTypeStr(expr->type));
    break;
  case TYPE_CHAR:
    logger::code(getInstStr("li", "$v0", "12"), "Read character");
    break;
  }

  logger::code("syscall");
  logger::code(getInstStr("sw", "$v0", getRegStr(expr)));

  delete expr;
}

void writeExpr(Expr* expr)
{
  logger::debug("WRITE " + getExprStr(expr));
  loadImmediate(expr);
  logger::code(getInstStr("move", "$a0", expr->reg->getName()));

  switch (expr->type)
  {
  case TYPE_BOOL:
  case TYPE_INT:
    logger::code(getInstStr("li", "$v0", "1"), "Write " + getTypeStr(expr->type));
    break;
  case TYPE_CHAR:
    logger::code(getInstStr("li", "$v0", "11"), "Write character");
    break;
  case TYPE_STRING:
    logger::code(getInstStr("li", "$v0", "4"), "Write string");
    break;
  }

  logger::code("syscall");

  delete expr;
}


void forInit(const std::string& lhs, Expr* rhs)
{
  pushLoop(lhs);
  logger::comment("Init for loop counter");
  assignExpr(lvalueExpr(lhs), rhs);
  logger::label(getLoopLabel("for"));
}

int forDownTo(Expr* expr)
{
  logger::comment("Check for loop condition");
  auto counter = getLoopCounter();
  loadImmediate(expr);
  logger::code(getInstStr("slt", expr->reg->getName(), counter->reg->getName(), expr->reg->getName()), "For loop comparison");
  logger::code(getInstStr("bne", expr->reg->getName(), "$zero", getLoopLabel("for_done")), "End foor loop if comparison is true");
  logger::comment("For loop body");
  delete counter;
  delete expr;
  return -1;
}

int forTo(Expr* expr)
{
  logger::comment("Check for loop condition");
  auto counter = getLoopCounter();
  loadImmediate(expr);
  logger::code(getInstStr("slt", expr->reg->getName(), expr->reg->getName(), counter->reg->getName()), "For loop comparison");
  logger::code(getInstStr("bne", expr->reg->getName(), "$zero", getLoopLabel("for_done")), "End for loop if comparison is true");
  logger::comment("For loop body");
  delete counter;
  delete expr;
  return 1;
}

void forCounter(int val)
{
  auto counter = getLoopCounter();
  std::string action = val == 1 ? "Increment" : "Decrement";
  logger::comment("Update for loop counter");
  logger::code(getInstStr("addi", counter->reg->getName(), counter->reg->getName(), std::to_string(val)), action + " for loop counter");
  logger::code(getInstStr("sw", counter->reg->getName(), getRegStr(counter)));
  logger::code(getInstStr("j", getLoopLabel("for")));
  logger::label(getLoopLabel("for_done"));
  popLoop();
  delete counter;
}

void ifBegin()
{
  logger::comment("If statement");
  pushLoop();
}

void ifCondition(Expr* expr)
{
  pushLoop();
  loadImmediate(expr);
  logger::code(getInstStr("beq", expr->reg->getName(), "$zero", getLoopLabel("else")), "Jump if condition is false");
  delete expr;
}

void ifThen()
{
  auto label = getLoopLabel("else");
  popLoop();
  logger::code(getInstStr("j", getLoopLabel("if_done")), "Jump to the end of the if statement");
  logger::label(label);
}

void ifEnd()
{
  logger::label(getLoopLabel("if_done"));
  popLoop();
}

void repeatBegin()
{
  pushLoop();
  logger::label(getLoopLabel("repeat"));
}

void repeatCondition(Expr* expr)
{
  loadImmediate(expr);
  logger::code(getInstStr("beq", expr->reg->getName(), "$zero", getLoopLabel("repeat")), "Repeat if condition is false");
  logger::comment("Done repeating: " + getLoopLabel("repeat"));
  popLoop();
  delete expr;
}

void whileBegin()
{
  pushLoop();
  logger::label(getLoopLabel("while"));
}

void whileCondition(Expr* expr)
{
  loadImmediate(expr);
  logger::code(getInstStr("beq", expr->reg->getName(), "$zero", getLoopLabel("while_done")), "End while loop if condition is false");
  delete expr;
}

void whileEnd()
{
  logger::code(getInstStr("j", getLoopLabel("while")));
  logger::label(getLoopLabel("while_done"));
  popLoop();
}

Expr* addExpr(Expr* lhs, Expr* rhs)
{
  printExpr(getExprStr(lhs) + " ADD " + getExprStr(rhs));
  checkTypes(lhs, rhs);

  auto newExpr = new Expr;
  newExpr->exprNum = curSymbol++;
  newExpr->type = lhs->type;

  if (lhs->isConst)
  {
    if (rhs->isConst)
    {
      newExpr->isConst = true;
      newExpr->intVal = lhs->intVal + rhs->intVal;
      logger::debug("Resulting " + getTypeStr(newExpr->type) + ": " + std::to_string(newExpr->intVal) + " = " + std::to_string(lhs->intVal) + " + " + std::to_string(rhs->intVal));
    }
    else
    {
      newExpr->isConst = false;
      newExpr->reg = Register::allocate();
      logger::code(getArithInstStr("addi", newExpr, rhs, lhs, true));
    }
  }
  else
  {
    if (rhs->isConst)
    {
      newExpr->isConst = false;
      newExpr->reg = Register::allocate();
      logger::code(getArithInstStr("addi", newExpr, lhs, rhs, true));
    }
    else
    {
      newExpr->isConst = false;
      newExpr->reg = Register::allocate();
      logger::code(getArithInstStr("add", newExpr, lhs, rhs));
    }
  }

  delete lhs;
  delete rhs;

  return newExpr;
}

Expr* andExpr(Expr* lhs, Expr* rhs)
{
  printExpr(getExprStr(lhs) + " AND " + getExprStr(rhs));
  checkTypes(lhs, rhs);

  auto newExpr = new Expr;
  newExpr->exprNum = curSymbol++;
  newExpr->type = TYPE_BOOL;
  newExpr->isConst = lhs->isConst && rhs->isConst;

  if (lhs->type != TYPE_BOOL)
    logger::compileWarning("Implicit conversion of integer type to boolean");

  if (newExpr->isConst)
  {
    newExpr->intVal = lhs->intVal && rhs->intVal;
    logger::debug("Resulting " + getTypeStr(newExpr->type) + ": " + std::to_string(newExpr->intVal) + " = " + std::to_string(lhs->intVal) + " && " + std::to_string(rhs->intVal));
  }
  else
  {
    newExpr->reg = Register::allocate();
    loadImmediate(lhs);
    loadImmediate(rhs);
    logger::code(getArithInstStr("and", newExpr, lhs, rhs));
    logger::code(getInstStr("sne", newExpr->reg->getName(), newExpr->reg->getName(), "$zero"));
  }

  delete lhs;
  delete rhs;

  return newExpr;
}

Expr* charExpr(int expr)
{
  std::string s = "0";
  s[0] = static_cast<char>(expr);
  if (expr < 32)
    s = "ascii(" + std::to_string(expr) + ")";
  printExpr(std::string("Char = '") + s + "'");

  auto newExpr = new Expr;
  newExpr->exprNum = curSymbol++;
  newExpr->type = TYPE_CHAR;
  newExpr->isConst = true;
  newExpr->intVal = expr;
  return newExpr;
}

Expr* chrExpr(Expr* expr)
{
  printExpr("CHR " + getExprStr(expr));
  if (expr->type != TYPE_INT)
    logger::compileError("CHR() only accepts integers");

  expr->type = TYPE_CHAR;

  return expr;
}

Expr* divExpr(Expr* lhs, Expr* rhs)
{
  printExpr(getExprStr(lhs) + " DIVIDE " + getExprStr(rhs));
  checkTypes(lhs, rhs);

  auto newExpr = new Expr;
  newExpr->exprNum = curSymbol++;
  newExpr->type = lhs->type;
  newExpr->isConst = lhs->isConst && rhs->isConst;

  if (newExpr->isConst)
  {
    if (rhs->intVal == 0)
      logger::compileError("Cannot divide by zero");
    newExpr->intVal = lhs->intVal / rhs->intVal;
    logger::debug("Resulting " + getTypeStr(newExpr->type) + ": " + std::to_string(newExpr->intVal) + " = " + std::to_string(lhs->intVal) + " / " + std::to_string(rhs->intVal));
  }
  else
  {
    newExpr->reg = Register::allocate();
    loadImmediate(lhs);
    loadImmediate(rhs);
    logger::code(getArithInstStr("div", lhs, rhs));
    logger::code(getInstStr("mflo", newExpr->reg->getName()));
  }

  delete lhs;
  delete rhs;

  return newExpr;
}

Expr* eqExpr(Expr* lhs, Expr* rhs)
{
  printExpr(getExprStr(lhs) + " EQ " + getExprStr(rhs));
  checkTypes(lhs, rhs);

  auto newExpr = new Expr;
  newExpr->exprNum = curSymbol++;
  newExpr->type = TYPE_BOOL;
  newExpr->isConst = lhs->isConst && rhs->isConst;

  if (newExpr->isConst)
  {
    newExpr->intVal = lhs->intVal == rhs->intVal;
    logger::debug("Resulting " + getTypeStr(newExpr->type) + ": " + std::to_string(newExpr->intVal) + " = " + std::to_string(lhs->intVal) + " == " + std::to_string(rhs->intVal));
  }
  else
  {
    newExpr->reg = Register::allocate();
    loadImmediate(lhs);
    loadImmediate(rhs);
    logger::code(getArithInstStr("seq", newExpr, lhs, rhs));
  }

  delete lhs;
  delete rhs;

  return newExpr;
}

Expr* gtExpr(Expr* lhs, Expr* rhs)
{
  printExpr(getExprStr(lhs) + " GT " + getExprStr(rhs));
  checkTypes(lhs, rhs);

  auto newExpr = new Expr;
  newExpr->exprNum = curSymbol++;
  newExpr->type = TYPE_BOOL;
  newExpr->isConst = lhs->isConst && rhs->isConst;

  if (newExpr->isConst)
  {
    newExpr->intVal = lhs->intVal > rhs->intVal;
    logger::debug("Resulting " + getTypeStr(newExpr->type) + ": " + std::to_string(newExpr->intVal) + " = " + std::to_string(lhs->intVal) + " > " + std::to_string(rhs->intVal));
  }
  else
  {
    newExpr->reg = Register::allocate();
    loadImmediate(lhs);
    loadImmediate(rhs);
    logger::code(getArithInstStr("sgt", newExpr, lhs, rhs));
  }

  delete lhs;
  delete rhs;

  return newExpr;
}

Expr* gteExpr(Expr* lhs, Expr* rhs)
{
  printExpr(getExprStr(lhs) + " GTE " + getExprStr(rhs));
  checkTypes(lhs, rhs);

  auto newExpr = new Expr;
  newExpr->exprNum = curSymbol++;
  newExpr->type = TYPE_BOOL;
  newExpr->isConst = lhs->isConst && rhs->isConst;

  if (newExpr->isConst)
  {
    newExpr->intVal = lhs->intVal >= rhs->intVal;
    logger::debug("Resulting " + getTypeStr(newExpr->type) + ": " + std::to_string(newExpr->intVal) + " = " + std::to_string(lhs->intVal) + " >= " + std::to_string(rhs->intVal));
  }
  else
  {
    newExpr->reg = Register::allocate();
    loadImmediate(lhs);
    loadImmediate(rhs);
    logger::code(getArithInstStr("sge", newExpr, lhs, rhs));
  }

  delete lhs;
  delete rhs;

  return newExpr;
}

Expr* intExpr(int expr)
{
  printExpr(std::string("Int = '") + std::to_string(expr) + "'");

  auto newExpr = new Expr;
  newExpr->exprNum = curSymbol++;
  newExpr->type = TYPE_INT;
  newExpr->isConst = true;
  newExpr->intVal = expr;
  return newExpr;
}

Expr* loadExpr(Expr* expr)
{
  // TODO: Check for nullptr everywhere
  if (expr->type == TYPE_STRING)
  {
    expr->reg = Register::allocate();
    logger::code(getInstStr("la", expr->reg->getName(), expr->strVal), "Load string '" + expr->name + "'");
  }
  else if (expr->isConst)
  {
    logger::debug("Load const " + getTypeStr(expr->type) + " '" + expr->name + " = " + std::to_string(expr->intVal) + "'");
  }
  else
  {
    expr->reg = Register::allocate();
    logger::code(getInstStr("lw", expr->reg->getName(), getRegStr(expr)), "Load " + getTypeStr(expr->type) + " '" + expr->name + "'");
  }

  return expr;
}

Expr* ltExpr(Expr* lhs, Expr* rhs)
{
  printExpr(getExprStr(lhs) + " LT " + getExprStr(rhs));
  checkTypes(lhs, rhs);

  auto newExpr = new Expr;
  newExpr->exprNum = curSymbol++;
  newExpr->type = TYPE_BOOL;
  newExpr->isConst = lhs->isConst && rhs->isConst;

  if (newExpr->isConst)
  {
    newExpr->intVal = lhs->intVal < rhs->intVal;
    logger::debug("Resulting " + getTypeStr(newExpr->type) + ": " + std::to_string(newExpr->intVal) + " = " + std::to_string(lhs->intVal) + " < " + std::to_string(rhs->intVal));
  }
  else
  {
    newExpr->reg = Register::allocate();
    loadImmediate(lhs);
    loadImmediate(rhs);
    logger::code(getArithInstStr("slt", newExpr, lhs, rhs));
  }

  delete lhs;
  delete rhs;

  return newExpr;
}

Expr* lteExpr(Expr* lhs, Expr* rhs)
{
  printExpr(getExprStr(lhs) + " LTE " + getExprStr(rhs));
  checkTypes(lhs, rhs);

  auto newExpr = new Expr;
  newExpr->exprNum = curSymbol++;
  newExpr->type = TYPE_BOOL;
  newExpr->isConst = lhs->isConst && rhs->isConst;

  if (newExpr->isConst)
  {
    newExpr->intVal = lhs->intVal <= rhs->intVal;
    logger::debug("Resulting " + getTypeStr(newExpr->type) + ": " + std::to_string(newExpr->intVal) + " = " + std::to_string(lhs->intVal) + " <= " + std::to_string(rhs->intVal));
  }
  else
  {
    newExpr->reg = Register::allocate();
    loadImmediate(lhs);
    loadImmediate(rhs);
    logger::code(getArithInstStr("sle", newExpr, lhs, rhs));
  }

  delete lhs;
  delete rhs;

  return newExpr;
}

Expr* lvalueExpr(const std::string& expr)
{
  printExpr("LVALUE = '" + expr + "'");
  auto sym = tables::getSymbol(expr);

  auto newExpr = new Expr;
  newExpr->exprNum = curSymbol++;
  newExpr->type = sym.type;
  newExpr->isConst = sym.isConst;
  newExpr->intVal = sym.value;
  newExpr->strVal = sym.location;
  newExpr->name = expr;
  return newExpr;
}

Expr* modExpr(Expr* lhs, Expr* rhs)
{
  printExpr(getExprStr(lhs) + " MOD " + getExprStr(rhs));
  checkTypes(lhs, rhs);

  auto newExpr = new Expr;
  newExpr->exprNum = curSymbol++;
  newExpr->type = lhs->type;
  newExpr->isConst = lhs->isConst && rhs->isConst;

  if (newExpr->isConst)
  {
    if (rhs->intVal == 0)
      logger::compileError("Cannot divide by zero");
    newExpr->intVal = lhs->intVal % rhs->intVal;
    logger::debug("Resulting " + getTypeStr(newExpr->type) + ": " + std::to_string(newExpr->intVal) + " = " + std::to_string(lhs->intVal) + " % " + std::to_string(rhs->intVal));
  }
  else
  {
    newExpr->reg = Register::allocate();
    loadImmediate(lhs);
    loadImmediate(rhs);
    logger::code(getArithInstStr("div", lhs, rhs));
    logger::code(getInstStr("mfhi", newExpr->reg->getName()));
  }

  delete lhs;
  delete rhs;

  return newExpr;
}

Expr* multExpr(Expr* lhs, Expr* rhs)
{
  printExpr(getExprStr(lhs) + " MULTIPLY " + getExprStr(rhs));
  checkTypes(lhs, rhs);

  auto newExpr = new Expr;
  newExpr->exprNum = curSymbol++;
  newExpr->type = lhs->type;
  newExpr->isConst = lhs->isConst && rhs->isConst;

  if (newExpr->isConst)
  {
    newExpr->intVal = lhs->intVal * rhs->intVal;
    logger::debug("Resulting " + getTypeStr(newExpr->type) + ": " + std::to_string(newExpr->intVal) + " = " + std::to_string(lhs->intVal) + " * " + std::to_string(rhs->intVal));
  }
  else
  {
    newExpr->reg = Register::allocate();
    loadImmediate(lhs);
    loadImmediate(rhs);
    logger::code(getArithInstStr("mult", lhs, rhs));
    logger::code(getInstStr("mflo", newExpr->reg->getName()));
  }

  delete lhs;
  delete rhs;

  return newExpr;
}

Expr* negExpr(Expr* expr)
{
  printExpr("NEGATE " + getExprStr(expr));
  checkType(expr);

  if (expr->isConst)
    expr->intVal = -expr->intVal;
  else
    logger::code(getInstStr("sub", expr->reg->getName(), "$zero", expr->reg->getName()), "Negate");
  return expr;
}

Expr* neqExpr(Expr* lhs, Expr* rhs)
{
  printExpr(getExprStr(lhs) + " NEQ " + getExprStr(rhs));
  checkTypes(lhs, rhs);

  auto newExpr = new Expr;
  newExpr->exprNum = curSymbol++;
  newExpr->type = TYPE_BOOL;
  newExpr->isConst = lhs->isConst && rhs->isConst;

  if (newExpr->isConst)
  {
    newExpr->intVal = lhs->intVal != rhs->intVal;
    logger::debug("Resulting " + getTypeStr(newExpr->type) + ": " + std::to_string(newExpr->intVal) + " = " + std::to_string(lhs->intVal) + " != " + std::to_string(rhs->intVal));
  }
  else
  {
    newExpr->reg = Register::allocate();
    loadImmediate(lhs);
    loadImmediate(rhs);
    logger::code(getArithInstStr("sne", newExpr, lhs, rhs));
  }

  delete lhs;
  delete rhs;

  return newExpr;
}

Expr* notExpr(Expr* expr)
{
  printExpr("NOT " + getExprStr(expr));
  checkType(expr);

  if (expr->isConst)
  {
    if (expr->type == TYPE_BOOL)
      expr->intVal = 1 - expr->intVal;
    else
      expr->intVal = ~expr->intVal;
  }
  else
  {
    if (expr->type == TYPE_BOOL)
      logger::code(getInstStr("xori", expr->reg->getName(), expr->reg->getName(), "1"), "Negate");
    else
      logger::code(getInstStr("xori", expr->reg->getName(), expr->reg->getName(), "-1"), "Negate");
  }

  return expr;
}

Expr* orExpr(Expr* lhs, Expr* rhs)
{
  printExpr(getExprStr(lhs) + " OR " + getExprStr(rhs));
  checkTypes(lhs, rhs);

  auto newExpr = new Expr;
  newExpr->exprNum = curSymbol++;
  newExpr->type = TYPE_BOOL;
  newExpr->isConst = lhs->isConst && rhs->isConst;

  if (lhs->type != TYPE_BOOL)
    logger::compileWarning("Implicit conversion of integer type to boolean");

  if (newExpr->isConst)
  {
    newExpr->intVal = lhs->intVal || rhs->intVal;
    logger::debug("Resulting " + getTypeStr(newExpr->type) + ": " + std::to_string(newExpr->intVal) + " = " + std::to_string(lhs->intVal) + " || " + std::to_string(rhs->intVal));
  }
  else
  {
    newExpr->reg = Register::allocate();
    loadImmediate(lhs);
    loadImmediate(rhs);
    logger::code(getArithInstStr("or", newExpr, lhs, rhs));
    logger::code(getInstStr("sne", newExpr->reg->getName(), newExpr->reg->getName(), "$zero"));
  }

  delete lhs;
  delete rhs;

  return newExpr;
}

Expr* ordExpr(Expr* expr)
{
  printExpr("ORD " + getExprStr(expr));
  if (expr->type != TYPE_CHAR)
    logger::compileError("ORD() only accepts characters");

  expr->type = TYPE_INT;

  return expr;
}

Expr* predExpr(Expr* expr)
{
  printExpr("PRED " + getExprStr(expr));
  checkType(expr);

  if (expr->type == TYPE_BOOL)
    expr = notExpr(expr); 
  else if (expr->isConst)
    expr->intVal--;
  else
    logger::code(getInstStr("addi", expr->reg->getName(), expr->reg->getName(), "-1"), "Decrement");

  return expr;
}

Expr* strExpr(const std::string& expr)
{
  printExpr("String = " + expr);

  auto newExpr = new Expr;
  newExpr->exprNum = curSymbol++;
  newExpr->type = TYPE_STRING;
  newExpr->isConst = true;
  newExpr->name = tables::addString(expr);
  newExpr->strVal = newExpr->name;

  return loadExpr(newExpr);
}

Expr* subExpr(Expr* lhs, Expr* rhs)
{
  printExpr(getExprStr(lhs) + " SUBTRACT " + getExprStr(rhs));
  checkTypes(lhs, rhs);

  auto newExpr = new Expr;
  newExpr->exprNum = curSymbol++;
  newExpr->type = lhs->type;
  newExpr->isConst = lhs->isConst && rhs->isConst;

  if (newExpr->isConst)
  {
    newExpr->intVal = lhs->intVal - rhs->intVal;
    logger::debug("Resulting " + getTypeStr(newExpr->type) + ": " + std::to_string(newExpr->intVal) + " = " + std::to_string(lhs->intVal) + " - " + std::to_string(rhs->intVal));
  }
  else
  {
    newExpr->reg = Register::allocate();
    loadImmediate(lhs);
    loadImmediate(rhs);
    logger::code(getArithInstStr("sub", newExpr, lhs, rhs));
  }

  delete lhs;
  delete rhs;

  return newExpr;
}

Expr* succExpr(Expr* expr)
{
  printExpr("SUCC " + getExprStr(expr));
  checkType(expr);

  if (expr->type == TYPE_BOOL)
    expr = notExpr(expr);
  else if (expr->isConst)
    expr->intVal++;
  else
    logger::code(getInstStr("addi", expr->reg->getName(), expr->reg->getName(), "1"), "Increment");

  return expr;
}

Expr* expr(const std::string& msg)
{
  noop(msg);

  auto newExpr = new Expr;
  newExpr->exprNum = curSymbol++;
  newExpr->type = TYPE_INT;
  newExpr->isConst = true;
  newExpr->intVal = 0;
  newExpr->strVal = "";
  newExpr->name = "Empty Expr";
  return newExpr;
}

Expr* expr(Expr*, const std::string& msg) { return expr(msg); }
Expr* expr(Expr*, Expr*, const std::string& msg) { return expr(msg); }

int noop(const std::string& msg)
{
  if (!msg.empty()) logger::debug(msg);
  return -1;
}

int noop(int, const std::string& msg) { return noop(msg); }
int noop(int, int, const std::string& msg) { return noop(msg); }
