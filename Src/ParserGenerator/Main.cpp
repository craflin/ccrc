
#include <nstd/Console.h>
#include <nstd/Process.h>

#include "Reader.h"
#include "Rules.h"

void_t usage(char_t* argv[])
{
  Console::errorf("Usage: %s <input> -o <output>\n", argv[0]);
}

int_t main(int_t argc, char_t* argv[])
{
  String inputFile;
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
        if(inputFile.isEmpty())
          inputFile = argument;
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

  if(inputFile.isEmpty() || outputFile.isEmpty())
    return usage(argv), 1;

  // action
  Reader reader;
  Rules rules;
  if(!reader.read(inputFile, rules))
    return Console::errorf("%s", (const char_t*)reader.getError()), 1;

  return 0;
}
