
#pragma once

#include "MetaInfoData.h"

class Parser
{
public:
  MetaInfoData metaInfoData;

public:
  Parser();
  ~Parser();

  bool_t parse(const String& sourceFile, const String& headerFile);

private:
  class Private;
  Private* p;
};
