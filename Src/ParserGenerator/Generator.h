
#pragma once

#include <nstd/String.h>
#include <nstd/File.h>
#include <nstd/HashSet.h>

class Rules;
class Production;

class Generator
{
public:
  bool_t generate(const Rules& rules, const String& outputFile);

  const String& getError() const {return error;}

private:
  String filePath;
  File file;
  String error;

private:
  void_t openFile(const String& file);
  void_t writeFile(const String& str);
  void_t closeFile();

  static String getTypeName(const String& name);
  static String getVariableName(const String& name);
  static String getTokenValue(const String& name);

  void_t writeFields(const Production& production, HashSet<String>& writtenFields);
  void_t writeProductionCode(const String& indent, uint_t depth, const String& typeName, const String& variableName, const Production& production);
};
