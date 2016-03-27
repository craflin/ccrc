
#pragma once

#include <nstd/String.h>
#include <nstd/List.h>
#include <nstd/HashMap.h>

struct ReflectorData
{
public:
  struct Type
  {
    enum ReflectionType
    {
      interfaceType,
      objectType,
      referencedType,
    };

    struct Field
    {
      String name;
      int64_t value;
    };

    struct Method
    {
      struct Parameter
      {
        Type* type;
        String name;
        String description;
      };

      String name;
      String description;
      Type* type; // return type
      List<Parameter> parameters;
    };

    struct Property
    {
      String name;
      String description;
      Type* type;
      String getter;
      String setter;
      String variable;
    };

    String name;
    String description;
    List<Type*> baseTypes;
    List<Method> methods;
    List<Property> properties;
    bool_t external;
    ReflectionType reflectionType;
  };

  HashMap<String, Type> types;
};
