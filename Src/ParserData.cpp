
#include <nstd/Console.h>

#include "ParserData.h"

void_t ParserData::print()
{
  for(HashMap<String, TypeDecl>::Iterator i = declarations.begin(), end = declarations.end(); i != end; ++i)
    i->print();
}

void_t ParserData::TypeDecl::print()
{
  if(!comment.isEmpty())
    Console::printf("%s\n", (const tchar_t*)comment);
  String printName;
  switch(type)
  {
  case enumType:
    printName.append("enum");
    break;
  case classType:
    printName.append("class");
    break;
  case structType:
    printName.append("struct");
    break;
  }
  printName.append(" - ");
  printName.append(name);
  if(!templateParams.isEmpty())
  {
    printName.append('<');
    for(HashMap<String, String>::Iterator i = templateParams.begin(), end = templateParams.end();;)
    {
      printName.append(i.key());
      if(++i == end)
        break;
      printName.append(", ");
    }
    printName.append('>');
  }
  if(!baseTypes.isEmpty())
  {
    printName.append(" : ");
    for(List<String>::Iterator i = baseTypes.begin(), end = baseTypes.end();;)
    {
      printName.append(*i);
      if(++i == end)
        break;
      printName.append(", ");
    }
  }
  Console::printf("%s\n", (const tchar_t*)printName);
  for(List<TypeDecl::MethodDecl>::Iterator i = methods.begin(), end = methods.end(); i != end; ++i)
    i->print();
  for(List<String>::Iterator i = innerComments.begin(), end = innerComments.end(); i != end; ++i)
    Console::printf("  %s\n", (const tchar_t*)*i);
}

void_t ParserData::TypeDecl::MethodDecl::print()
{
  if(!comment.isEmpty())
    Console::printf("  %s\n", (const tchar_t*)comment);
  String printName;
  printName.append(getReturnType());
  printName.append(' ');
  printName.append(name);
  printName.append('(');
  if(!parameters.isEmpty())
    for(List<Parameter>::Iterator i = parameters.begin(), end = parameters.end();;)
    {
      printName.append(i->type);
      if(!i->name.isEmpty())
      {
        printName.append(' ');
        printName.append(i->name);
      }
      if(++i == end)
        break;
      printName.append(", ");
    }
  printName.append(')');
  Console::printf("  %s\n", (const tchar_t*)printName);
}

String ParserData::TypeDecl::MethodDecl::getReturnType() const
{
  const char* functionType = type;
  const char* p = String::findOneOf(functionType, " <");
  if(!p)
    return String();
  if(*p == ' ')
  {
    String result =String::fromCString(functionType, p - functionType);
    if(result == "const")
    {
      int k = 42;
    }
    return result;
  }
  for(size_t depth = 1;;)
  {
    p = String::findOneOf(p + 1, "<>");
    if(*p == '<')
      ++depth;
    else if(depth == 1)
      return String::fromCString(functionType, p + 1 - functionType);
    else
      --depth;
  }
}
