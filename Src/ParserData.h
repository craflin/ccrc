
#pragma once

#include <nstd/String.h>
#include <nstd/HashMap.h>
#include <nstd/List.h>
#include <nstd/Variant.h>

class ParserData
{
public:
  struct TypeDecl
  {
    enum Type
    {
      enumType,
      classType,
      structType,
    };

    struct MethodDecl
    {
      struct Parameter
      {
        String name;
        String type;
        Variant value;
      };

      String name;
      String type;
      String comment;
      List<Parameter> parameters;

    public:
      void_t print();
      String getReturnType() const;
    };

    String name;
    Type type;
    String comment;
    String file;
    HashMap<String, String> templateParams;
    List<String> baseTypes;
    List<TypeDecl::MethodDecl> methods;
    List<String> innerComments;

  public:
    void_t print();
  };

public:
  void_t print();

public:
  HashMap<String, TypeDecl> declarations;
  HashMap<String, TypeDecl> templates;
  String headerFile;
};
