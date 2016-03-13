
#include <nstd/File.h>
#include <nstd/Error.h>
#include <nstd/Console.h>
#include <nstd/HashMap.h>

#include "Reader.h"
#include "Rules.h"
#include "Generator.h"

bool_t Reader::read(const String& filePath, Rules& rules)
{
  String data;
  File file;
  if(!file.open(filePath))
    return error.printf("Could not open file '%s': %s", (const char_t*)filePath, (const char_t*)Error::getErrorString()), false;
  if(!file.readAll(data))
    return error.printf("Could not read file '%s': %s\n", (const char_t*)filePath, (const char_t*)Error::getErrorString()), false;

  this->rules = &rules;

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

  /*
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
  for(HashMap<String, Rule*>::Iterator i = rules.rules.begin(), end = rules.rules.end(); i != end; ++i)
  {
    const Rule* rule = *i;
    Console::printf("%s:\n", (const char_t*)i.key());
    PrintProduction::print("    ", rule->productionRoot);
    Console::printf("\n");
  }
  */
  return true;
}

bool_t Reader::getToken(const char_t*& p, String& token)
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
    goto returnToken;
  }
  const char_t* end = String::find(p, ' ');
  if(!end)
    end = p + String::length(p);
  token = String(p, end - p);
  p = end;
returnToken:
  while(String::isSpace(*p))
    ++p;
  return true;
}

bool_t Reader::handleLine(const String& line)
{
  const char_t* p = line;
  bool_t startsWithSpace = String::isSpace(*p);
  if(startsWithSpace)
    for(++p; String::isSpace(*p); ++p);

  if(*p == '#')
    return true;

  if(startsWithSpace)
  {
    if(rules->rules.isEmpty())
      return false;

    HashMap<String, uint_t> typeNames;
    Rule* rule = rules->rules.back();
    uint_t lineIndex = rule->productionLines++;
    Production* productionRoot = &rule->productionRoot;
    Production** production = &productionRoot;
    String token;
    while(getToken(p, token))
    {
      bool isOptional = token.startsWith("[");
      bool isTerminal = token.startsWith("'") || token.startsWith("\"") || token.startsWith("['") || token.startsWith("[\"");

      if(!isTerminal)
      {
        String typeName = Generator::getTypeName(token);
        HashMap<String, uint_t>::Iterator it2 = typeNames.find(typeName);
        size_t& count = it2 == typeNames.end() ? typeNames.append(typeName, 0) : *it2;
        ++count;

        if(count > 1)
        {
          if(isOptional)
            token = token.substr(0, token.length() - 1) + String("_") + String::fromUInt(count) + "]";
          else
            token += String("_") + String::fromUInt(count);
        }
      }

      if(!*production)
        *production = new Production;
      HashMap<String, Production::Data>::Iterator it = (*production)->productions.find(token);
      if(it == (*production)->productions.end())
      {
        Production::Data& data = (*production)->productions.append(token, Production::Data());
        data.lineIndex = *p ? -1 : lineIndex;
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
    String ruleName(p, end - p);
    rules->rules.append(ruleName, new Rule);
  }
  return true;
}
