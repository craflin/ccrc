
#pragma once

#include <nstd/List.h>

#include "Namespace.h"

class TypeName;

class Class : public Namespace
{
public:
  void_t setComment(const String& comment) {this->comment = comment;}
  void_t addBaseType(TypeName& type) {baseTypes.append(&type);}

  virtual String getFullName() const;

private:
  String comment;
  List<TypeName*> baseTypes;
};
