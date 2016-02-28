
#pragma once

#include <nstd/String.h>
#include <nstd/List.h>

class TypeName
{
public:
  void_t setName(const String& name) {this->name = name;}

  void_t addTemplateParam(TypeName* param) {templateParams.append(param);}

private:
  String name;
  List<TypeName*> templateParams;
};
