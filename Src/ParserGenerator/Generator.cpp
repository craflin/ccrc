
#include <nstd/Error.h>
#include <nstd/HashSet.h>
#include <nstd/Console.h>
#include <nstd/Debug.h>

#include "Generator.h"
#include "Rules.h"

bool_t Generator::generateHeader(const Rules& rules, const String& outputFile)
{
  openFile(outputFile);

  writeFile("\n");
  writeFile("#pragma once\n");
  writeFile("\n");
  writeFile("#include <nstd/Base.h>\n");
  writeFile("\n");

  writeFile("namespace Parser\n");
  writeFile("{\n");
  writeFile("\n");

  for(HashMap<String, Rule*>::Iterator i = rules.rules.begin(), end = rules.rules.end(); i != end; ++i)
    writeFile(String("struct ") + getTypeName(i.key()) + ";\n");

  writeFile("\n");

  HashSet<String> writtenFields;
  for(HashMap<String, Rule*>::Iterator i = rules.rules.begin(), end = rules.rules.end(); i != end; ++i)
  {
    Rule* rule = *i;
    writeFile(String("struct ") + getTypeName(i.key()) + "\n");
    writeFile("{\n");
    writeFile("  int_t type;\n");
    writtenFields.clear();
    writeFields(rule->productionRoot, writtenFields);
    writeFile("};\n\n");
  }

  for(HashMap<String, Rule*>::Iterator i = rules.rules.begin(), end = rules.rules.end(); i != end; ++i)
  {
    Rule* rule = *i;
    String typeName = getTypeName(i.key());
    String variableName = getVariableName(i.key());
    writeFile(typeName + "* parse" + typeName + "();\n");
  }

  writeFile("};\n");

  closeFile();
  return error.isEmpty();
}

bool_t Generator::generateSource(const String& headerFile, const Rules& rules, const String& outputFile)
{
  openFile(outputFile);

  writeFile("\n");

  writeFile("#include \"Parser.h\"\n");
  writeFile("\n");

  if(!headerFile.isEmpty())
  {
    writeFile(String("#include \"") + headerFile + "\"\n");
    writeFile("\n");
  }

  writeFile("namespace Parser\n");
  writeFile("{\n");
  writeFile("\n");

  for(HashMap<String, Rule*>::Iterator i = rules.rules.begin(), end = rules.rules.end(); i != end; ++i)
  {
    Rule* rule = *i;
    String typeName = getTypeName(i.key());
    String variableName = getVariableName(i.key());

    bool isLeftRecursive = false;
    {for(HashMap<String, Production::Data>::Iterator i = rule->productionRoot.productions.begin(), end = rule->productionRoot.productions.end(); i != end; ++i)
      if(getTypeName(i.key()) == typeName)
        isLeftRecursive = true;}
    if(isLeftRecursive)
      Console::printf("%s\n", (const char_t*)i.key());

    writeFile(typeName + "* parse" + typeName + "()\n");
    writeFile("{\n");

    if(isLeftRecursive)
      writeFile(String("  ") + typeName + "* result = 0;\n");

    writeFile(String("  ") + typeName + " " + variableName + " = {};\n");

    //if(isLeftRecursive)
    //  writeFile(String("  ") + variableName + "." + variableName + " =  result;\n");

    writeProductionCode(String(), 0, typeName, variableName, isLeftRecursive, rule->productionRoot);

    if(isLeftRecursive)
      writeFile("  return result;\n");
    else
      writeFile("  return 0;\n");
    writeFile("}\n\n");
  }

  writeFile("};\n");

  closeFile();
  return error.isEmpty();
}

void_t Generator::openFile(const String& filePath)
{
  this->filePath = filePath;
  if(!file.open(filePath, File::writeFlag))
    error.printf("Could not open file '%s': %s", (const char_t*)filePath, (const char_t*)Error::getErrorString());
}

void_t Generator::writeFile(const String& str)
{
  if(!file.isOpen())
    return;
  if(!file.write(str))
  {
    error.printf("Could not write to file '%s': %s", (const char_t*)filePath, (const char_t*)Error::getErrorString());
    file.close();
  }
}

void_t Generator::closeFile()
{
  if(file.isOpen())
  {
    file.close();
    Console::printf("%s\n", (const char_t*)filePath);
  }
}

String Generator::getTypeName(const String& name)
{
  String result(name.length());
  const char_t* p = name;
  bool_t isOptional = *p == '[';
  if(isOptional)
    ++p;
  while(*p)
  {
    const char_t* m = String::find(p, '_');
    size_t len = m ? (m - p) : (String::length(p) - (isOptional ? 1 : 0));
    result.append(p, len);
    char_t& c = ((char_t*)result)[result.length() - len];
    c = String::toUpperCase(c);
    p += len;
    if(*p)
      ++p;
  }
  if(result.length() > 0 && String::isDigit(((const char_t*)result)[result.length() - 1]))
    result = result.substr(0, result.length() - 1);
  return result;
}

String Generator::getVariableName(const String& name)
{
  String result(name.length());
  const char_t* p = name;
  bool_t isOptional = *p == '[';
  if(isOptional)
    ++p;
  while(*p)
  {
    const char_t* m = String::find(p, '_');
    size_t len = m ? (m - p) : (String::length(p) - (isOptional ? 1 : 0));
    result.append(p, len);
    if(result.length() > len)
    {
      char_t& c = ((char_t*)result)[result.length() - len];
      c = String::toUpperCase(c);
    }
    p += len;
    if(*p)
      ++p;
  }
  if(result == "operator")
    return "op";
  return result;
}

String Generator::getTokenValue(const String& name)
{
  String result = ((const char_t*)name)[0] == '[' ? name.substr(1, name.length() - 2) : name;
  ((char_t*)result)[0] = '"';
  ((char_t*)result)[result.length() - 1] = '"';
  return result;
}

void_t Generator::writeFields(const Production& production, HashSet<String>& writtenFields)
{
  for(HashMap<String, Production::Data>::Iterator i = production.productions.begin(), end  = production.productions.end(); i != end; ++i)
  {
    const String& name = i.key();
    if(!name.startsWith("'") && !name.startsWith("\"") && !name.startsWith("['") && !name.startsWith("[\""))
    {
      String variableName = getVariableName(name);
      if(!writtenFields.contains(variableName))
      {
        writeFile(String("  ") + getTypeName(name) + "* " + variableName + ";\n");
        writtenFields.append(variableName);
      }
    }
    if(i->subProduction)
      writeFields(*i->subProduction, writtenFields);
  }
}

void_t Generator::writeProductionCode(const String& indent, uint_t depth, const String& typeName, const String& variableName, bool_t isLeftRecursive, const Production& production)
{
  writeFile(indent + "  pushState();\n");
  for(HashMap<String, Production::Data>::Iterator i = production.productions.begin(), end  = production.productions.end(); i != end; ++i)
  {
    const String& name = i.key();
    bool onLeftRecursiveBranch = isLeftRecursive && getTypeName(name) == typeName;
    bool optional = name.startsWith("[");
    bool terminal = name.startsWith("'") || name.startsWith("\"") || name.startsWith("['") || name.startsWith("[\"");
    if(onLeftRecursiveBranch)
    {
      ASSERT(depth == 0);
      writeFile(indent + "  popState();\n");
      writeFile(indent + "loop:\n");
      writeFile(indent + "  "  + variableName + " = {};\n");
      writeFile(indent + "  "  + variableName + "." + variableName + " = result;\n");
      writeFile(indent + "  pushState();\n");
    }
    if(!terminal)
    {
      if(!onLeftRecursiveBranch)
        writeFile(indent + "  " + variableName + "." + getVariableName(name) + " = parse" + getTypeName(name) + "();\n");
      if(!optional)
        writeFile(indent + "  if(" + variableName + "." + getVariableName(name) + ")\n");
    }
    else
    {
      writeFile(indent + "  if(token.value == " + getTokenValue(name) + ")\n");
      if(optional)
        writeFile(indent + "    readToken();\n");
    }
    writeFile(indent + "  {\n");
    if(terminal && !optional)
        writeFile(indent + "    readToken();\n");
    if(!i->subProduction)
    {
      if(depth + 1 > 0)
        writeFile(indent + "    dropState(" + String::fromUInt(depth + 1) + ");\n");
      writeFile(indent + "    " + variableName + ".type = " + String::fromUInt(i->lineIndex) + ";\n");
      if(isLeftRecursive)
      {
        writeFile(indent + "    result = new " + typeName + "(" + variableName + ");\n");
        writeFile(indent + "    goto loop;\n");
      }
      else
        writeFile(indent + "    return new " + typeName + "(" + variableName + ");\n");
    }
    else
      writeProductionCode(indent + "  ", depth + 1, typeName, variableName, isLeftRecursive, *i->subProduction);
    writeFile(indent + "  }\n");

    if(onLeftRecursiveBranch)
    {
      writeFile(indent + "  if(result)\n");
      writeFile(indent + "  {\n");
      writeFile(indent + "    popState();\n");
      writeFile(indent + "    return result;\n");
      writeFile(indent + "  }\n");
    }
  }
  writeFile(indent + "  popState();\n");
}