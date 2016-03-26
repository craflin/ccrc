
#pragma once

#include <nstd/String.h>
#include <nstd/HashMap.h>
#include <nstd/List.h>

#include "MetaMethodDecl.h"

class MetaInfoData;

class MetaTypeDecl
{
public:
  enum Type
  {
    enumType,
    classType,
    structType
  };

public:
  String name;
  String comment;
  String file;
  Type type;
  HashMap<String, String> templateParams;
  List<String> baseTypes;
  List<MetaMethodDecl> methods;
  List<String> innerComments;

public:
  void_t addTemplateParam(const String& paramName, const String& typeName) {templateParams.append(paramName, typeName);}
  void_t addBaseType(const String& type) {baseTypes.append(type);}
  void_t addMethodDecl(const String& name, const String& type)
  {
    MetaMethodDecl& method = methods.append(MetaMethodDecl());
    method.name = name;
    method.type = type;
  }
  MetaMethodDecl* getLastMethodDecl() {return methods.isEmpty() ? 0 : &methods.back();}
  void_t addInnerComment(const String& comment) {innerComments.append(comment);}

  bool_t isReflected(const MetaInfoData& metaInfo) const
  {
    if(name == "Reflected")
      return true;
    //for(List<String>::Iterator i = baseTypes.begin(), end = baseTypes.end(); i != end; ++i)
    //{
    //  HashMap<String, MetaTypeDecl>::Iterator it = metaInfo.declarations.find(*i);
    //  if(it == metaInfo.declarations.end())
    //    continue;
    //
    //}
    return false;
  }

  void_t print();
};
