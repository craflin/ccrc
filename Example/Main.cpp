
#include <nstd/Console.h>

#include "Objects.h"

void_t printObjectMethods(const Reflected::Type& type)
{
  for(size_t i = 0; i < type.baseTypesNum; ++i)
    printObjectMethods(*type.baseTypes[i]);
  for(size_t i = 0; i < type.methodsNum; ++i)
  {
    const Reflected::Type::Method& method = type.methods[i];
    Console::printf("%s %s(", (const tchar_t*)method.type->name, (const tchar_t*)method.name);
    for(size_t i = 0; i < method.parametersNum; ++i)
    {
      const Reflected::Type::Method::Parameter& parameter = method.parameters[i];
      Console::printf("%s %s%s", (const tchar_t*)parameter.type->name, (const tchar_t*)parameter.name, i + 1 < method.parametersNum ? ", " : "");
    }
    Console::printf(")\n");
  }
}

void_t printObject(const Reflected& obj)
{
  const Reflected::Type& type = obj.getReflectedType();
  Console::printf("Name: %s\n", (const tchar_t*)type.name);
  Console::printf("Description: %s\n", (const tchar_t*)type.description);
  Console::printf("BaseTypes: ");
  for(size_t i = 0; i < type.baseTypesNum; ++i)
    Console::printf("%s, ", (const tchar_t*)type.baseTypes[i]->name);
  Console::printf("\n");
  printObjectMethods(type);

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