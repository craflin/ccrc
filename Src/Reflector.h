
#pragma once

#include <nstd/Pool.h>

#include "ReflectorData.h"
#include "ParserData.h"

class Reflector
{
public:
  ReflectorData data;

public:
  bool_t reflect(const String& headerFile, const ParserData& parserData);

private:
  const ParserData* parserData;
  HashMap<String, const ParserData::TypeDecl*> referencedTypes;
  Pool<ParserData::TypeDecl> additionalTypes;

private:
  void_t addReferencedType(const String& name);
  ReflectorData::Type::ReflectionType getReflectionType(const ParserData::TypeDecl& type);

  static String getTemplateName(const String& type);
  static String extractClassDescriptions(const String& comment);
  static void_t extractMethodDescriptions(const String& comment, ReflectorData::Type::Method& method);

  static String buildinTypes[];
};
