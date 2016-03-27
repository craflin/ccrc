
#pragma once

#include <nstd/String.h>
#include <nstd/HashMap.h>
#include <nstd/List.h>

class ParserData
{
public:
  class TypeDecl
  {
  public:
    enum Type
    {
      enumType,
      classType,
      structType,
    };

    class MethodDecl
    {
    public:
      struct Parameter
      {
        String type;
        String name;
      };

    public:
      String name;
      String comment;
      String type;
      List<Parameter> parameters;

    public:
      void_t addParameter(const String& name, const String& typeName);
      void_t print();
      String getReturnType() const;
    };

  public:
    String name;
    String comment;
    String file;
    Type type;
    HashMap<String, String> templateParams;
    List<String> baseTypes;
    List<TypeDecl::MethodDecl> methods;
    List<String> innerComments;

  public:
    void_t addTemplateParam(const String& paramName, const String& typeName) {templateParams.append(paramName, typeName);}
    void_t addBaseType(const String& type) {baseTypes.append(type);}
    TypeDecl::MethodDecl& addMethodDecl(const String& name, const String& type);
    void_t addInnerComment(const String& comment) {innerComments.append(comment);}


    void_t print();
  };

public:
  TypeDecl& getTypeDecl(const String& name);

  void_t print();

public:
  HashMap<String, TypeDecl> declarations;
  String headerFile;
};
