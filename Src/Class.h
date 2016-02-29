
#pragma once

#include <nstd/List.h>

#include "Namespace.h"

class TypeName;

class TemplateParameter
{
public:
  TemplateParameter() : defaultType(0) {}

  void_t setName(const String& name) {this->name = name;}
  const String& getName() const {return name;}
  void_t setDefaultType(TypeName* defaultType) {this->defaultType = defaultType;}

private:
  String name;
  TypeName* defaultType;
};

class Class : public Namespace
{
public:
  void_t setComment(const String& comment) {this->comment = comment;}
  void_t addBaseType(TypeName& type) {baseTypes.append(&type);}

  virtual String getFullName() const;

  void_t addTemplateParam(TemplateParameter& param) {templateParameters.append(&param);}

private:
  String comment;
  List<TypeName*> baseTypes;
  List<TemplateParameter*> templateParameters;
};
