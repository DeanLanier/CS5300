#ifndef REGISTER_HPP
#define REGISTER_HPP

// Standard Includes
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Register;
using Reg = std::shared_ptr<Register>;

class Register
{
public:
  ~Register();
  
  static Reg allocate();
  std::string getName() const;

private:
  Register(const std::string& name);
  
  static std::vector<std::string> ms_pool;
  std::string m_name;
};

std::ostream& operator<<(std::ostream& rOs, const Reg& reg);

#endif
