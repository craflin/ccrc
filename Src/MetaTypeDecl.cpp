
#include <nstd/Console.h>

#include "MetaTypeDecl.h"

void_t MetaTypeDecl::print()
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
  for(List<MetaMethodDecl>::Iterator i = methods.begin(), end = methods.end(); i != end; ++i)
    i->print();
  for(List<String>::Iterator i = innerComments.begin(), end = innerComments.end(); i != end; ++i)
    Console::printf("  %s\n", (const tchar_t*)*i);
}
