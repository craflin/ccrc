
#include <nstd/Console.h>

#include "MetaMethodDecl.h"

void_t MetaMethodDecl::print()
{
  if(!comment.isEmpty())
    Console::printf("  %s\n", (const tchar_t*)comment);
  String printName;
  printName.append(getReturnType());
  printName.append(' ');
  printName.append(name);
  printName.append('(');
  for(List<Param>::Iterator i = params.begin(), end = params.end();;)
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
