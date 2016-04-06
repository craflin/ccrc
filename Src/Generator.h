
#pragma once

#include <nstd/File.h>

struct ReflectorData;

class Generator
{
public:
  bool_t generate(const String& outputFile, const String& headerFile, const ReflectorData& data);

private:
  File file;

private:
  void_t write(const String& str) {file.write(str); }
  void_t write() {file.write("\n"); }

  static String getNamespacePrefix(const String& type);
  static String getNamespaceSuffix(const String& type);
  static String getVarName(const String& type);
  static String getFullVarName(const String& type);
  static String formatString(const String& str);
  static String formatVariant(const Variant& var);
};
