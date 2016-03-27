
#include "Reflector.h"

bool_t Reflector::reflect(const String& headerFile, const ParserData& parserData)
{
  // find all referenced types in header file
  HashMap<const ParserData::TypeDecl*, ReflectorData::Type::ReflectionType> referencedTypes;
  for(HashMap<String, ParserData::TypeDecl>::Iterator i = parserData.declarations.begin(), end = parserData.declarations.end(); i != end; ++i)
  {
    const ParserData::TypeDecl& type = *i;
    if(type.file != headerFile)
      continue;
    ReflectorData::Type::ReflectionType reflectionType = getReflectionType(parserData, type);
    if(reflectionType != ReflectorData::Type::interfaceType && reflectionType != ReflectorData::Type::objectType)
      continue;
    referencedTypes.append(&type, reflectionType);
    for(List<String>::Iterator i = type.baseTypes.begin(), end = type.baseTypes.end(); i != end; ++i)
    {
      HashMap<String, ParserData::TypeDecl>::Iterator it = parserData.declarations.find(*i);
      if(it != parserData.declarations.end())
        if(referencedTypes.find(&*it) == referencedTypes.end())
          referencedTypes.append(&*it, getReflectionType(parserData, type));
    }
    for(List<ParserData::TypeDecl::MethodDecl>::Iterator i = type.methods.begin(), end = type.methods.end(); i != end; ++i)
    {
      ParserData::TypeDecl::MethodDecl& method = *i;
      HashMap<String, ParserData::TypeDecl>::Iterator it = parserData.declarations.find(method.getReturnType());
      if(it != parserData.declarations.end())
        if(referencedTypes.find(&*it) == referencedTypes.end())
          referencedTypes.append(&*it, getReflectionType(parserData, type));
      for(List<ParserData::TypeDecl::MethodDecl::Parameter>::Iterator i = method.parameters.begin(), end = method.parameters.end(); i != end; ++i)
      {
        ParserData::TypeDecl::MethodDecl::Parameter& parameter = *i;
        HashMap<String, ParserData::TypeDecl>::Iterator it = parserData.declarations.find(parameter.type);
        if(it != parserData.declarations.end())
          if(referencedTypes.find(&*it) == referencedTypes.end())
            referencedTypes.append(&*it, getReflectionType(parserData, type));
      }
    }
  }

  // create type reflection
  struct UnresolvedType
  {
    String name;
    ReflectorData::Type** type;
  };
  List<UnresolvedType> unresolvedTypes;
  for(HashMap<const ParserData::TypeDecl*, ReflectorData::Type::ReflectionType>::Iterator i = referencedTypes.begin(), end = referencedTypes.end(); i != end; ++i)
  {
    const ParserData::TypeDecl& type = *i.key();
    ReflectorData::Type& reflectedType = data.types.append(type.name, ReflectorData::Type());
    reflectedType.name = type.name;
    reflectedType.description = type.comment;
    reflectedType.reflectionType = *i;
    reflectedType.external = type.file != headerFile;
    for(List<String>::Iterator i = type.baseTypes.begin(), end = type.baseTypes.end(); i != end; ++i)
      unresolvedTypes.append({*i, &reflectedType.baseTypes.append(0)});
    for(List<ParserData::TypeDecl::MethodDecl>::Iterator i = type.methods.begin(), end = type.methods.end(); i != end; ++i)
    {
      const ParserData::TypeDecl::MethodDecl& method = *i;
      ReflectorData::Type::Method& reflectedMethod = reflectedType.methods.append({});
      reflectedMethod.name = method.name;
      reflectedMethod.description = method.comment;
      unresolvedTypes.append({method.getReturnType(), &reflectedMethod.type});
      for(List<ParserData::TypeDecl::MethodDecl::Parameter>::Iterator i = method.parameters.begin(), end = method.parameters.end(); i != end; ++i)
      {
        ParserData::TypeDecl::MethodDecl::Parameter& parameter = *i;
        ReflectorData::Type::Method::Parameter& reflectedParameter = reflectedMethod.parameters.append({});
        reflectedParameter.name = parameter.name;
        unresolvedTypes.append({parameter.type, &reflectedParameter.type});
      }
    }
  }

  // resolve all unresolved types in type reflection
  for(List<UnresolvedType>::Iterator i = unresolvedTypes.begin(), end = unresolvedTypes.end(); i != end; ++i)
    *i->type = &*data.types.find(i->name);

  return true;
}

ReflectorData::Type::ReflectionType Reflector::getReflectionType(const ParserData& parserData, const ParserData::TypeDecl& type)
{
  if(type.name == "Reflected")
    return ReflectorData::Type::objectType;
  ReflectorData::Type::ReflectionType refType = ReflectorData::Type::referencedType;
  for(List<String>::Iterator i = type.baseTypes.begin(), end = type.baseTypes.end(); i != end; ++i)
  {
    HashMap<String, ParserData::TypeDecl>::Iterator it = parserData.declarations.find(*i);
    if(it == parserData.declarations.end())
      continue;
    ReflectorData::Type::ReflectionType baseRefType = getReflectionType(parserData, *it);
    if(baseRefType == ReflectorData::Type::objectType)
      return ReflectorData::Type::objectType;
    if(baseRefType == ReflectorData::Type::interfaceType)
      refType = ReflectorData::Type::interfaceType;
  }
  if(refType == ReflectorData::Type::interfaceType)
    return ReflectorData::Type::interfaceType;
  for(List<String>::Iterator i = type.innerComments.begin(), end = type.innerComments.end(); i != end; ++i)
    if(i->find("@invokable") || i->find("@property"))
      return ReflectorData::Type::interfaceType;
  return ReflectorData::Type::referencedType;
}
