
#pragma once

#include <nstd/Variant.h>

class Reflected
{
public:
 struct Type
  {
    struct Method
    {
      struct Parameter
      {
        String name;
        String description;
        Type* type;
        Variant defaultValue;
      };
      
      String name;
      String description;
      const Type* type; // return type
      const Parameter* parameters;
      size_t parametersNum;

      //virtual bool_t invoke(Reflected& reflected, Variant& result, ...) const = 0;
    };
    
    struct Property
    {
      String name;
      String description;
      bool readOnly;
      //virtual bool_t set(Reflected& reflected, const Variant& value) const = 0;
      //virtual bool_t get(const Reflected& reflected, Variant& value) const = 0;
    };

    String name;
    String description;
    const Method* methods;
    size_t methodsNum;
    const Property* properties;
    size_t propertiesNum;
    const Type* const * baseTypes;
    size_t baseTypesNum;
  };

public:
  virtual const Reflected::Type& getReflectedType() const = 0;
  
public:
  //bool_t invokeMethod(const String& name, Variant& result, ...);
  //bool_t getPropery(const String& name, Variant& value);
  //bool_t setPropery(const String& name, const Variant& value);
};

