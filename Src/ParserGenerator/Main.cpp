
#include <nstd/Console.h>
#include <nstd/Process.h>
#include <nstd/File.h>
#include <nstd/Error.h>
#include <nstd/HashMap.h>

void_t usage(char_t* argv[])
{
  Console::errorf("Usage: %s <input> -o <output>\n", argv[0]);
}


class Production
{
public:
  struct Data
  {
    Production* subProduction;
    uint_t lineIndex;
  };

public:
  HashMap<String, Data> productions;
};

class Rule
{
public:
  Rule() : productionLines(0) {}

public:
  Production productionRoot;
  uint_t productionLines;
};

HashMap<String, Rule*> rules;

bool_t getToken(const char_t*& p, String& token)
{
  while(String::isSpace(*p))
    ++p;
  if(!*p)
    return false;
  if(*p == '[')
  {
    const char_t* end = String::find(p, ']');
    if(end)
      ++end;
    else
      end = p + String::length(p);
    token = String(p, end - p);
    p = end;
    token.replace(" ", "");
    return true;
  }
  const char_t* end = String::find(p, ' ');
  if(!end)
    end = p + String::length(p);
  token = String(p, end - p);
  p = end;
  return true;
}

static bool_t handleLine(const String& line)
{
  const char_t* p = line;
  bool_t startsWithSpace = String::isSpace(*p);
  if(startsWithSpace)
    for(++p; String::isSpace(*p); ++p);

  if(*p == '#')
    return true;

  if(startsWithSpace)
  {
    if(rules.isEmpty())
      return false;

    Rule* rule = rules.back();
    uint_t lineIndex = rule->productionLines++;
    Production* productionRoot = &rule->productionRoot;
    Production** production = &productionRoot;
    String token;
    while(getToken(p, token))
    {
      if(!*production)
        *production = new Production;
      HashMap<String, Production::Data>::Iterator it = (*production)->productions.find(token);
      if(it == (*production)->productions.end())
      {
        Production::Data& data = (*production)->productions.append(token, Production::Data());
        data.lineIndex = lineIndex;
        data.subProduction = 0;
        production = &data.subProduction;
      }
      else
      {
        production = &it->subProduction;
      }
    }
  }
  else
  {
    const char_t* end = String::find(p, ' ');
    if(!end)
      end = p + String::length(p);
    rules.append(String(p, end - p), new Rule);
  }
  return true;
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
  String data;
  File file;
  if(!file.open(inputFile))
  {
    Console::errorf("Could not open file '%s': %s\n", (const char_t*)inputFile, (const char_t*)Error::getErrorString());
    return 1;
  }
  if(!file.readAll(data))
  {
    Console::errorf("Could not read file '%s': %s\n", (const char_t*)inputFile, (const char_t*)Error::getErrorString());
    return 1;
  }

  char_t* p = data;
  String lineStr;
  while(p)
  {
    const char_t* line = p;
    char_t* end = (char_t*)String::findOneOf(p, "\n\r");
    if(end)
    {
      p = end;
      if(*end == '\r' && end[1] == '\n')
        p += 2;
      else
        ++p;
      *end = '\0';
    }
    else
    {
      end = (char_t*)line + String::length(line);
      p = 0;
    }
   
    if(!*line)
      continue;
    lineStr.attach(line, p - line);
    if(!handleLine(lineStr))
      return 1;
  }

  struct PrintProduction
  {
    static void_t print(const String& indent, const Production& production)
    {
      for(HashMap<String, Production::Data>::Iterator i = production.productions.begin(), end  = production.productions.end(); i != end; ++i)
      {
        Console::printf("%s%s\n", (const char_t*)indent, (const char_t*)i.key());
        if(i->subProduction)
          print(indent + "    ", *i->subProduction);
      }
    }
  };
  for(HashMap<String, Rule*>::Iterator i = rules.begin(), end = rules.end(); i != end; ++i)
  {
    const Rule* rule = *i;
    Console::printf("%s:\n", (const char_t*)i.key());
    PrintProduction::print("    ", rule->productionRoot);
    Console::printf("\n");
  }

  return 0;
}
