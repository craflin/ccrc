
#include <nstd/Directory.h>
#include <nstd/File.h>
#include <nstd/HashSet.h>

#include "ReflectorData.h"
#include "Generator.h"

bool_t Generator::generate(const String& outputFile, const String& headerFile, const ReflectorData& data)
{
  if(!file.open(outputFile, File::writeFlag))
    return false;

  String outputFileAbs = File::isAbsolutePath(outputFile) ? outputFile : String("/") + outputFile;
  String headerFileAbs = File::isAbsolutePath(headerFile) ? headerFile : String("/") + headerFile;

  write();
  write(String("#include <Reflected.h>\n"));
  write();
  write(String("#include \"") + File::getRelativePath(File::dirname(outputFileAbs), headerFileAbs) + "\"\n");
  write();
  write(String("namespace Reflection\n"));
  write(String("{\n"));

  for(HashMap<String, ReflectorData::Type>::Iterator i = data.types.begin(), end = data.types.end(); i != end; ++i)
  {
    const ReflectorData::Type& type = *i;
    write(String("  ") + getNamespacePrefix(type.name) + "extern const Reflected::Type " + getVarName(type.name) + ";" + getNamespaceSuffix(type.name) + "\n");
  }
  write();

  for(HashMap<String, ReflectorData::Type>::Iterator i = data.types.begin(), end = data.types.end(); i != end; ++i)
  {
    const ReflectorData::Type& type = *i;
    if(type.external && type.reflectionType != ReflectorData::Type::referencedType)
      continue;

    write(String("  ") + getNamespacePrefix(type.name) + "\n");
    if(!type.baseTypes.isEmpty())
    {
      write(String("    const Reflected::Type* _") + getVarName(type.name) + "_BaseTypes[] = {");
      for(List<ReflectorData::Type*>::Iterator i = type.baseTypes.begin(), end = type.baseTypes.end(); i != end; ++i)
        write(String("&") + getFullVarName((*i)->name) + ", ");
      write(String("};\n"));
    }
      
    for(List<ReflectorData::Type::Method>::Iterator i = type.methods.begin(), end = type.methods.end(); i != end; ++i)
    {
      const ReflectorData::Type::Method& method = *i;
      if(!method.parameters.isEmpty())
      {
        write(String("    const Reflected::Type::Method::Parameter _Method_") + getVarName(type.name) + "_" + method.name + "_Parameters[] = {\n");
        for(List<ReflectorData::Type::Method::Parameter>::Iterator i = method.parameters.begin(), end = method.parameters.end(); i != end; ++i)
        {
          const ReflectorData::Type::Method::Parameter& parameter = *i;
          write(String("      {") + formatString(parameter.name) + ", ");
          write(formatString(parameter.description) + ", ");
          write(String("&") + getFullVarName(parameter.type->name) + ", ");
          write(formatVariant(parameter.value) +  "},\n");
        }
        write(String("    };\n"));
      }
    }
    if(!type.methods.isEmpty())
    {
      write(String("    const Reflected::Type::Method _") + getVarName(type.name) + "_Methods[] = {\n");
      for(List<ReflectorData::Type::Method>::Iterator i = type.methods.begin(), end = type.methods.end(); i != end; ++i)
      {
        ReflectorData::Type::Method& method = *i;
        write(String("      {") + formatString(method.name) + ", ");
        write(formatString(method.description) + ", ");
        write(String("&") + getFullVarName(method.type->name) + ", ");
        if(method.parameters.isEmpty())
          write(String("0, "));
        else
          write(String("_Method_") + getVarName(type.name) + "_" + method.name + "_Parameters, ");
        write(String::fromUInt64(method.parameters.size()) + "},\n");
      }
      write(String("    };\n"));
    }
    write(String("    extern const Reflected::Type ") + getVarName(type.name) + " = {");
    write(formatString(type.name) + ", ");
    write(formatString(type.description) + ", ");
    if(type.methods.isEmpty())
      write(String("0, "));
    else
      write(String("_") + getVarName(type.name) + "_Methods, ");
    write(String::fromUInt64(type.methods.size()) + ", ");
    write(String("0, "));
    write(String("0, "));
    if(type.baseTypes.isEmpty())
      write(String("0, "));
    else
      write(String("_") + getVarName(type.name) + "_BaseTypes, ");
    write(String::fromUInt64(type.baseTypes.size()) + ",");
    write(String("};\n"));
    write(String("  ") + getNamespaceSuffix(type.name) + "\n");
  }

  write(String("};\n"));
  write();

  for(HashMap<String, ReflectorData::Type>::Iterator i = data.types.begin(), end = data.types.end(); i != end; ++i)
  {
    const ReflectorData::Type& type = *i;
    if(type.external || type.reflectionType != type.objectType)
      continue;
    write(String("const Reflected::Type& ") + type.name + "::getReflectedType() const {return Reflection::" + getFullVarName(type.name) + ";}\n");
  }
  return true;
}

String Generator::getNamespacePrefix(const String& type)
{
  String prefix;
  const tchar_t* start = type;
  const tchar_t* namespaceStart = start;
  for(;;)
  {
    const tchar_t* namespaceEnd = String::find(namespaceStart, "::");
    if(!namespaceEnd)
      return prefix;
    prefix.append(String("namespace _") + type.substr(namespaceStart - start, namespaceEnd - namespaceStart) + " {");
    namespaceStart = namespaceEnd + 2;
  }
}

String Generator::getNamespaceSuffix(const String& type)
{
  String suffix;
  const tchar_t* start = type;
  const tchar_t* namespaceStart = start;
  for(;;)
  {
    const tchar_t* namespaceEnd = String::find(namespaceStart, "::");
    if(!namespaceEnd)
      return suffix;
    suffix.append('}');
    namespaceStart = namespaceEnd + 2;
  }
}

String Generator::getVarName(const String& type)
{
  const tchar_t* start = type;
  const tchar_t* var = String::findLast(start, "::");
  if(var)
    return type.substr(var + 2 - start);
  if(type == "int")
    return String("_") + type + "Type";
  return type;
}

String Generator::getFullVarName(const String& type)
{
  String prefix;
  const tchar_t* start = type;
  const tchar_t* namespaceStart = start;
  for(;;)
  {
    const tchar_t* namespaceEnd = String::find(namespaceStart, "::");
    if(!namespaceEnd)
      return prefix + getVarName(type);
    prefix.append(String("_") + type.substr(namespaceStart - start, namespaceEnd - namespaceStart) + "::");
    namespaceStart = namespaceEnd + 2;
  }
}

String Generator::formatString(const String& str)
{
  if(str.isEmpty())
    return "String()";
  String result(str.length() * 2);
  result.append('"');
  for(const tchar_t* p = str;;)
  {
    const tchar_t* end = String::findOneOf(p, "\"\\\r\n");
    if(!end)
    {
      result.append(String(p, String::length(p)));
      break;
    }
    result.append(String(p, end - p));
    if(*end == '\r' ||*end == '\n')
    {
      //result.append("\\n\"\n\"");
      result.append("\\n");
      if(*end == '\r' && end[1] == '\n')
        ++end;
    }
    else
    {
      result.append("\\");
      result.append(*end);
    }
    p = end + 1;
  }
  result.append('"');
  return result;
}

String Generator::formatVariant(const Variant& var)
{
  switch(var.getType())
  {
  case Variant::boolType:
    return String("Variant(") + (var.toBool() ? String("true") : String("false")) + ")";
  case Variant::doubleType:
    return String("Variant(") + String::fromDouble(var.toDouble()) + ")";
  case Variant::intType:
    return String("Variant(") + String::fromInt64(var.toUInt()) + ")";
  case Variant::uintType:
    return String("Variant(") + String::fromInt64(var.toUInt()) + "UL)";
  case Variant::int64Type:
    return String("Variant(") + String::fromInt64(var.toUInt64()) + "LL)";
  case Variant::uint64Type:
    return String("Variant(") + String::fromUInt64(var.toUInt64()) + "ULL)";
  case Variant::stringType:
    return String("Variant(") + formatString(var.toString()) + ")";
  default:
    return "Variant()";
  }
}