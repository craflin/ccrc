
#include <nstd/Console.h>
#include <nstd/Process.h>
#include <nstd/List.h>

#include "Parser.h"
#include "Reflector.h"
#include "Generator.h"

void_t usage(char_t* argv[])
{
  Console::errorf("Usage: %s <header> <source> -o <output>\n", argv[0]);
}

int_t main(int_t argc, char_t* argv[])
{
  String headerFile;
  String sourceFile;
  String outputFile;
  List<String> additionalArgs;
  {
    Process::Option options[] = {
        {'o', "output", Process::argumentFlag},
        {'h', "help", Process::optionFlag},
        {'I', 0, Process::argumentFlag},
    };
    Process::Arguments arguments(argc, argv, options);
    int_t character;
    String argument;
    while(arguments.read(character, argument))
      switch(character)
      {
      case 'o':
        outputFile = argument;
        break;
      case 0:
        if(headerFile.isEmpty())
          headerFile = argument;
        else if(sourceFile.isEmpty())
          sourceFile = argument;
        else
          return usage(argv), 1;
        break;
      case 'I':
        additionalArgs.append(String("-I") + argument);
        break;
      case '?':
        Console::errorf("Unknown option: %s.\n", (const char_t*)argument);
        return 1;
      case ':':
        Console::errorf("Option %s required an argument.\n", (const char_t*)argument);
        return 1;
      default:
        return usage(argv), 1;
      }
  }

  if(headerFile.isEmpty() || outputFile.isEmpty())
    return usage(argv), 1;

  // parse input file
  Parser parser;
  if(!parser.parse(sourceFile, headerFile, additionalArgs))
    return 1;
  parser.data.print();

  // create type reflection structure
  Reflector reflector;
  if(!reflector.reflect(headerFile, parser.data))
    return 1;

  // generate output file
  Generator generator;
  if(!generator.generate(outputFile, headerFile, reflector.data))
    return 1;

  /*

  // find a compiler
  String path = Process::getEnvironmentVariable("PATH");
  //List<String> paths;
  //path.split(';', paths);
  // todo
  String compiler = "C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\bin\\cl.exe";
  Process::setEnvironmentVariable("INCLUDE", "C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\INCLUDE;C:\\Program Files (x86)\\Windows Kits\\8.1\\include\\shared;C:\\Program Files (x86)\\Windows Kits\\8.1\\include\\um;C:\\Program Files (x86)\\Windows Kits\\8.1\\include\\winrt;");

  */

  return 0;
}
