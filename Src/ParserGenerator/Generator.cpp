
#include <nstd/Error.h>
#include <nstd/HashSet.h>
#include <nstd/Console.h>

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
    if(i.key() == "class_head")
    {
      int k = 42;
    }
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

  //writeFile("\n");
  //writeFile("\n");

  for(HashMap<String, Rule*>::Iterator i = rules.rules.begin(), end = rules.rules.end(); i != end; ++i)
  {
    Rule* rule = *i;
    String typeName = getTypeName(i.key());
    String variableName = getVariableName(i.key());
    writeFile(typeName + "* parse" + typeName + "()\n");
    writeFile("{\n");
    writeFile(String("  ") + typeName + " " + variableName + " = {};\n");

    writeProductionCode(String(), 0, typeName, variableName, rule->productionRoot);

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

void_t Generator::writeProductionCode(const String& indent, uint_t depth, const String& typeName, const String& variableName, const Production& production)
{
  bool needPushPop = false;
  for(HashMap<String, Production::Data>::Iterator i = production.productions.begin(), end  = production.productions.end(); i != end; ++i)
  {
    const String& name = i.key();
    if(name.startsWith("[") || name.startsWith("\"") || name.startsWith("'"))
    {
      needPushPop = true;
      break;
    }
  }
  if(needPushPop)
    writeFile(indent + "  pushState();\n");
  else
    --depth;
  for(HashMap<String, Production::Data>::Iterator i = production.productions.begin(), end  = production.productions.end(); i != end; ++i)
  {
    const String& name = i.key();
    if(!name.startsWith("'") && !name.startsWith("\"") && !name.startsWith("['") && !name.startsWith("[\""))
    {
      writeFile(indent + String("  ") + variableName + "." + getVariableName(name) + " = parse" + getTypeName(name) + "();\n");
      if(!name.startsWith("["))
        writeFile(indent + String("  if(") + variableName + "." + getVariableName(name) + ")\n");
      writeFile(indent + "  {\n");
      if(!i->subProduction)
      {
        if(depth + 1 > 0)
          writeFile(indent + String("    dropState(") + String::fromUInt(depth + 1) + ");\n");
        writeFile(indent + String("    ") + variableName + ".type = " + String::fromUInt(i->lineIndex) + ";\n");
        writeFile(indent + String("    return new ") + typeName + "(" + variableName + ");\n");
      }
      else
        writeProductionCode(indent + "  ", depth + 1, typeName, variableName, *i->subProduction);
      writeFile(indent + "  }\n");
    }
    else
    {
      if(!name.startsWith("["))
      {
        writeFile(indent + String("  if(token.value == ") + getTokenValue(name) + ")\n");
        writeFile(indent + "  {\n");
        writeFile(indent + "    readToken();\n");
      }
      else
      {
        writeFile(indent + String("  if(token.value == ") + getTokenValue(name) + ")\n");
        writeFile(indent + "    readToken();\n");
        writeFile(indent + "  {\n");
      }
      if(!i->subProduction)
      {
        if(depth + 1 > 0)
          writeFile(indent + String("    dropState(") + String::fromUInt(depth + 1) + ");\n");
        writeFile(indent + String("    ") + variableName + ".type = " + String::fromUInt(i->lineIndex) + ";\n");
        writeFile(indent + String("    return new ") + typeName + "(" + variableName + ");\n");
      }
      else
        writeProductionCode(indent + "  ", depth + 1, typeName, variableName, *i->subProduction);
      writeFile(indent + "  }\n");
    }
  }
  if(needPushPop)
    writeFile(indent + "  popState();\n");
}
