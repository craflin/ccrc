
#pragma once

#include <nstd/String.h>
#include <nstd/List.h>

class MetaMethodDecl
{
public:
  struct Param
  {
    String type;
    String name;
  };

public:
  String name;
  String comment;
  String type;
  List<Param> params;

public:
  void_t addParam(const String& name, const String& typeName)
  {
    Param& param = params.append(Param());
    param.name = name;
    param.type = typeName;
  }

  void_t print();

  String getReturnType() const
  {
    const char* functionType = type;
    const char* p = String::findOneOf(functionType, " <");
    if(!p)
      return String();
    if(*p == ' ')
      return String::fromCString(functionType, p - functionType);
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
};