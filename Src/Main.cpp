
#include <nstd/Console.h>
#include <nstd/Process.h>

#include "Parser.h"
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
  {
    Process::Option options[] = {
        {'o', "output", Process::argumentFlag},
        {'h', "help", Process::optionFlag},
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



  Parser parser;


  parser.metaInfoData.print();

  Generator generator;
  if(!generator.generate(outputFile, headerFile, parser.metaInfoData))
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
