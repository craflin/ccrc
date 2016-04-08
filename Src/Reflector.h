
#pragma once

#include "ReflectorData.h"
#include "ParserData.h"

class Reflector
{
public:
  ReflectorData data;

public:
  bool_t reflect(const String& headerFile, const ParserData& parserData);

private:
  static ReflectorData::Type::ReflectionType getReflectionType(const ParserData& parserData, const ParserData::TypeDecl& type);
  static String extractClassDescriptions(const String& comment);
  static void_t extractMethodDescriptions(const String& comment, ReflectorData::Type::Method& method);
};
