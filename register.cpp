// Project Includes
#include "register.hpp"

std::vector<std::string> Register::ms_pool =
  {
                  "$s7", "$s6", "$s5", "$s4", "$s3", "$s2", "$s1", "$s0",
    "$t9", "$t8", "$t7", "$t6", "$t5", "$t4", "$t3", "$t2", "$t1", "$t0"
  };

Register::Register(const std::string& name) : m_name(name) {}
  
Register::~Register()
{
  ms_pool.push_back(m_name);
}

Reg Register::allocate()
{
  auto reg = ms_pool.back();
  ms_pool.pop_back();
  return Reg(new Register(reg));
}

std::string Register::getName() const { return m_name; }

std::ostream& operator<<(std::ostream& rOs, const Reg& reg)
{
  rOs << reg->getName();
  return rOs;
}
