
#include <nstd/Console.h>

#include "Objects.h"

void_t printObject(const Reflected& obj)
{
  const Reflected::Type& type = obj.getReflectedType();
  Console::printf("Name: %s\n", (const tchar_t*)type.name);
  Console::printf("Description: %s\n", (const tchar_t*)type.description);
  Console::printf("BaseTypes: ");
  for(size_t i = 0; i < type.baseTypesNum; ++i)
    Console::printf("%s, ", (const tchar_t*)type.baseTypes[i]->name);
  Console::printf("\n");

  Console::printf("\n");
}

int_t main(int_t argc, char_t* argv[])
{
  MyObject1 o1;
  SomeSpace1::MyObject2 o2;
  SomeSpace2::MyObject3 o3;
  SomeSpace2::MyObject4 o4;

  printObject(o1);
  printObject(o2);
  printObject(o3);
  printObject(o4);
  
  return 0;
}