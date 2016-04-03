// Project Includes
#include "compiler.hpp"
#include "logger.hpp"

// Standard Includes
#include <cstdio>
#include <cstdlib>
#include <sstream>

// Boost Includes
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main (int argc, char** argv)
{
  try
  {
    po::options_description desc("Allowed options");
    desc.add_options()
      ("help,h", "produce help message")
      ("input,i", po::value<std::string>(), "input cpsl file")
      ("output,o", po::value<std::string>(), "output asm file");

    po::positional_options_description posOpts;
    posOpts.add("input", 1);
    posOpts.add("output", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(posOpts).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
      std::ostringstream oss;
      oss << desc;
      logger::info(oss.str());
      return EXIT_SUCCESS;
    }

    std::string outFile = "out.asm";
    if (vm.count("output"))
      outFile = vm["output"].as<std::string>();

    std::string inFile = "in.cpsl";
    if (vm.count("input"))
      inFile = vm["input"].as<std::string>();

    if (!logger::init(outFile))
    {
      logger::error(std::string(argv[0]) + ": Output file '" + outFile + "' cannot be opened.");
      return EXIT_FAILURE;
    }

    if (freopen(inFile.c_str(), "r", stdin) == nullptr)
    {
      logger::error(std::string(argv[0]) + ": Input file '" + inFile + "' cannot be opened.");
      return EXIT_FAILURE;
    }

    startProgram();
  }
  catch (const std::exception& e)
  {
    logger::error(e.what());
    return EXIT_FAILURE;
  }
  catch (...)
  {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
