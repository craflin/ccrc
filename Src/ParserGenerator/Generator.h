
#pragma once

#include <nstd/String.h>
#include <nstd/File.h>
#include <nstd/HashSet.h>

class Rules;
class Production;

class Generator
{
public:
  bool_t generateHeader(const Rules& rules, const String& outputFile);
  bool_t generateSource(const String& headerFile, const Rules& rules, const String& outputFile);

  const String& getError() const {return error;}

public:
  static String getTypeName(const String& name);
  static String getVariableName(const String& name);
  static String getTokenValue(const String& name);

private:
  String filePath;
  File file;
  String error;

private:
  void_t openFile(const String& file);
  void_t writeFile(const String& str);
  void_t closeFile();


  void_t writeFields(const Production& production, HashSet<String>& writtenFields);
  void_t writeProductionCode(const String& indent, uint_t depth, const String& typeName, const String& variableName, bool_t isLeftRecursive, const Production& production);
};
