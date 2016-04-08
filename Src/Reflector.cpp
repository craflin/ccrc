
#include <nstd/Console.h>

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
    reflectedType.description = extractClassDescriptions(type.comment);
    reflectedType.reflectionType = *i;
    reflectedType.external = type.file != headerFile;
    for(List<String>::Iterator i = type.baseTypes.begin(), end = type.baseTypes.end(); i != end; ++i)
      unresolvedTypes.append({*i, &reflectedType.baseTypes.append(0)});
    for(List<ParserData::TypeDecl::MethodDecl>::Iterator i = type.methods.begin(), end = type.methods.end(); i != end; ++i)
    {
      const ParserData::TypeDecl::MethodDecl& method = *i;
      if(!method.comment.find("@invokable"))
        continue;
      ReflectorData::Type::Method& reflectedMethod = reflectedType.methods.append({});
      reflectedMethod.name = method.name;
      unresolvedTypes.append({method.getReturnType(), &reflectedMethod.type});
      for(List<ParserData::TypeDecl::MethodDecl::Parameter>::Iterator i = method.parameters.begin(), end = method.parameters.end(); i != end; ++i)
      {
        ParserData::TypeDecl::MethodDecl::Parameter& parameter = *i;
        ReflectorData::Type::Method::Parameter& reflectedParameter = reflectedMethod.parameters.append({});
        reflectedParameter.name = parameter.name;
        reflectedParameter.value = parameter.value;
        if(Variant(reflectedParameter.value.toBool()).toString() == reflectedParameter.value.toString())
          reflectedParameter.value = reflectedParameter.value.toBool();
        else if(String::fromInt(reflectedParameter.value.toInt()) == reflectedParameter.value.toString())
          reflectedParameter.value = reflectedParameter.value.toInt();
        else if(String::fromUInt(reflectedParameter.value.toUInt()) == reflectedParameter.value.toString())
          reflectedParameter.value = reflectedParameter.value.toUInt();
        else if(String::fromInt64(reflectedParameter.value.toInt64()) == reflectedParameter.value.toString())
          reflectedParameter.value = reflectedParameter.value.toInt();
        else if(String::fromUInt64(reflectedParameter.value.toUInt64()) == reflectedParameter.value.toString())
          reflectedParameter.value = reflectedParameter.value.toUInt64();
        else if(String::fromDouble(reflectedParameter.value.toDouble()) == reflectedParameter.value.toString())
          reflectedParameter.value = reflectedParameter.value.toDouble();
        unresolvedTypes.append({parameter.type, &reflectedParameter.type});
      }
      extractMethodDescriptions(method.comment, reflectedMethod);
    }
  }

  // resolve all unresolved types in type reflection
  for(List<UnresolvedType>::Iterator i = unresolvedTypes.begin(), end = unresolvedTypes.end(); i != end; ++i)
  {
    HashMap<String, ReflectorData::Type>::Iterator it = data.types.find(i->name);
    if(it == data.types.end())
    {
      if(i->name == "int")
      {
        ReflectorData::Type& reflectedType = data.types.append(i->name, ReflectorData::Type());
        reflectedType.name = i->name;
        reflectedType.reflectionType = ReflectorData::Type::buildinType;
        reflectedType.external = true;
        it = --HashMap<String, ReflectorData::Type>::Iterator(data.types.end());
      }
      else
      {
        Console::errorf("Unknown type '%s'\n", (const tchar_t*)i->name);
        return false;
      }
    }
    *i->type = &*it;
  }

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

String Reflector::extractClassDescriptions(const String& comment)
{
  List<String> lines;
  comment.split(lines, "\n\r");
  if(lines.isEmpty())
    return String();
  if(lines.front().startsWith("/**"))
    lines.front() = lines.front().substr(3);
  if(lines.back().endsWith("*/"))
    lines.back() = lines.back().substr(0, lines.back().length() - 2);
  for(List<String>::Iterator begin = lines.begin(), i = begin, end = lines.end(); i != end; ++i)
  {
    const tchar_t* p = *i;
    while(String::isSpace(*p))
      ++p;
    if(*p == '*' && i != begin)
    {
      ++p;
      while(String::isSpace(*p))
        ++p;
    }
    if(p != (const tchar_t*)*i)
      *i = i->substr(p - (const tchar_t*)*i);
  }
  String description;
  for(List<String>::Iterator i = lines.begin(), end = lines.end(); i != end; ++i)
  {
    const String& line = *i;
    if(line.isEmpty())
      continue;
    if(!description.isEmpty())
      description.append(' ');
    description.append(line);
  }
  return description;
}

void_t Reflector::extractMethodDescriptions(const String& comment, ReflectorData::Type::Method& method)
{
  List<String> lines;
  comment.split(lines, "\n\r");
  if(lines.isEmpty())
    return;
  if(lines.front().startsWith("/**"))
    lines.front() = lines.front().substr(3);
  if(lines.back().endsWith("*/"))
    lines.back() = lines.back().substr(0, lines.back().length() - 2);
  for(List<String>::Iterator begin = lines.begin(), i = begin, end = lines.end(); i != end; ++i)
  {
    const tchar_t* p = *i;
    while(String::isSpace(*p))
      ++p;
    if(*p == '*' && i != begin)
    {
      ++p;
      while(String::isSpace(*p))
        ++p;
    }
    if(p != (const tchar_t*)*i)
      *i = i->substr(p - (const tchar_t*)*i);
  }
  String& description = method.description;
  for(List<String>::Iterator i = lines.begin(), end = lines.end(); i != end; ++i)
  {
    const String& line = *i;
    if(line.isEmpty())
      continue;
    if(line.startsWith("@") || line.startsWith("\\"))
    {
      const tchar_t* p = (const tchar_t*)line + 1;
      if(String::compare(p, "class", 5) == 0 || String::compare(p, "interface", 9) == 0 || String::compare(p, "invokable", 9) == 0)
        continue;
      if(String::compare(p, "param", 5) == 0)
      {
        p += 5;
        while(String::isSpace(*p))
          ++p;
        if(*p == '[')
        {
          ++p;
          while(*p && *p != ']')
            ++p;
          while(String::isSpace(*p))
            ++p;
        }
        const tchar_t* end = String::findOneOf(p, " \t");
        if(!end)
          continue;
        String parameterName(p, end -p);
        p = end;
        while(String::isSpace(*p))
          ++p;
        String description(p, line.length() - (p - (const tchar_t*)line));
        for(List<ReflectorData::Type::Method::Parameter>::Iterator i =  method.parameters.begin(), end = method.parameters.end(); i != end; ++i)
        {
          ReflectorData::Type::Method::Parameter& parameter = *i;
          if(parameter.name == parameterName)
          {
            parameter.description = description;
            break;
          }
        }
        continue;
      }
    }
    if(!description.isEmpty())
      description.append(' ');
    description.append(line);
  }


  /*
  const tchar_t* p = comment;
  if(String::compare(p, "/**", 3) == 0)
    p += 3;
  for(;; ++p)
    switch(*p)
    {
    case ' ':
    case '\t':
      continue;
    case '\n':
      while(String::isSpace(*p))
        ++p;
      if(*p != '*')
        --p;
      continue;
    case '@':
    case '\\':
      ++p;
      if(String::compare(0, "param", 5) == 0)
      {
        p += 5;
        while(String::isSpace(*p))
          ++p;
        if(*p == '[')
        {
          ++p;
          while(*p && *p != ']')
            ++p;
          while(String::isSpace(*p))
            ++p;
        }

      }
      else if(String::compare(0, "class", 5) == 0 || String::compare(0, "interface", 9) == 0 || String::compare(0, "invokable", 9) == 0)
      {
        p = String::find(p, '\r');
        if(!p)
          goto finish;
      }
      continue;
    default:
      ??
    }
finish:
  ;
  */
}
