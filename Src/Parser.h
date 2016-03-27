
#pragma once

#include "ParserData.h"

class Parser
{
public:
  ParserData data;

public:
  Parser();
  ~Parser();

  bool_t parse(const String& sourceFile, const String& headerFile, const List<String>& additionalArgs);

private:
  class Private;
  Private* p;
};
