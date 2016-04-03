#ifndef COMPILER_HPP
#define COMPILER_HPP

// Project Includes
#include "register.hpp"

// Standard Includes
#include <string>

enum Type
{
  TYPE_BOOL,
  TYPE_CHAR,
  TYPE_INT,
  TYPE_STRING
};


struct Expr
{
  Type type;
  bool isConst;
  Reg reg;
  int intVal;
  std::string strVal;
  std::string name;
  int exprNum; // TODO: Debug only - remove this when done.
};

void startProgram();
void endProgram();

void addConst(const std::string& id, Expr* expr);
void addId(const std::string& id);
void addVars(int type);

void assignExpr(Expr* lhs, Expr* rhs);
void readExpr(Expr* expr);
void writeExpr(Expr* expr);

void forInit(const std::string& lhs, Expr* rhs);
int forDownTo(Expr* expr);
int forTo(Expr* expr);
void forCounter(int val);

void ifBegin();
void ifCondition(Expr* expr);
void ifThen();
void ifEnd();

void repeatBegin();
void repeatCondition(Expr* expr);

void whileBegin();
void whileCondition(Expr* expr);
void whileEnd();

int simpleType(const std::string& id);

Expr* addExpr(Expr* lhs, Expr* rhs);
Expr* andExpr(Expr* lhs, Expr* rhs);
Expr* charExpr(int expr);
Expr* chrExpr(Expr* expr);
Expr* divExpr(Expr* lhs, Expr* rhs);
Expr* eqExpr(Expr* lhs, Expr* rhs);
Expr* gtExpr(Expr* lhs, Expr* rhs);
Expr* gteExpr(Expr* lhs, Expr* rhs);
Expr* intExpr(int expr);
Expr* loadExpr(Expr* expr);
Expr* ltExpr(Expr* lhs, Expr* rhs);
Expr* lteExpr(Expr* lhs, Expr* rhs);
Expr* lvalueExpr(const std::string& expr);
Expr* modExpr(Expr* lhs, Expr* rhs);
Expr* multExpr(Expr* lhs, Expr* rhs);
Expr* negExpr(Expr* expr);
Expr* neqExpr(Expr* lhs, Expr* rhs);
Expr* notExpr(Expr* expr);
Expr* orExpr(Expr* lhs, Expr* rhs);
Expr* ordExpr(Expr* expr);
Expr* predExpr(Expr* expr);
Expr* strExpr(const std::string& expr);
Expr* subExpr(Expr* lhs, Expr* rhs);
Expr* succExpr(Expr* expr);

Expr* expr(const std::string& msg = "");
Expr* expr(Expr*, const std::string& msg = "");
Expr* expr(Expr*, Expr*, const std::string& msg = "");

int noop(const std::string& msg = "");
int noop(int, const std::string& msg = "");
int noop(int, int, const std::string& msg = "");

#endif
