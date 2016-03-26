
#pragma once

#include <nstd/File.h>

class MetaInfoData;

class Generator
{
public:
  bool_t generate(const String& outputFile, const String& headerFile, const MetaInfoData& metaInfoData);

private:
  File file;

private:
  void_t write(const String& str) {file.write(str + "\n"); }
  void_t write() {file.write("\n"); }
};
