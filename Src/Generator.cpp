
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
  write(String("#include <Reflected.h>"));
  write();
  write(String("#include \"") + File::getRelativePath(File::dirname(outputFileAbs), headerFileAbs) + "\"");
  write();
  write(String("namespace Reflection"));
  write(String("{"));

  for(HashMap<String, ReflectorData::Type>::Iterator i = data.types.begin(), end = data.types.end(); i != end; ++i)
  {
    const ReflectorData::Type& type = *i;
    write(String("  ") + getNamespacePrefix(type.name) + "extern const Reflected::Type " + getVarName(type.name) + ";" + getNamespaceSuffix(type.name));
  }
  write();

  for(HashMap<String, ReflectorData::Type>::Iterator i = data.types.begin(), end = data.types.end(); i != end; ++i)
  {
    const ReflectorData::Type& type = *i;
    if(type.external && type.reflectionType != ReflectorData::Type::referencedType)
      continue;

    write(String("  ") + getNamespacePrefix(type.name));
    if(!type.baseTypes.isEmpty())
    {
      write(String("    const Reflected::Type* _") + getVarName(type.name) + "_BaseTypes[] = { ");
      for(List<ReflectorData::Type*>::Iterator i = type.baseTypes.begin(), end = type.baseTypes.end(); i != end; ++i)
        write(String("      &") + (*i)->name + ",");
      write(String("    };"));
    }
      
    for(List<ReflectorData::Type::Method>::Iterator i = type.methods.begin(), end = type.methods.end(); i != end; ++i)
    {
      const ReflectorData::Type::Method& method = *i;
      if(!method.parameters.isEmpty())
      {
        write(String("    const Reflected::Type::Method::Parameter _Method_") + getVarName(type.name) + "_" + method.name + "_Parameters[] = { ");
        for(List<ReflectorData::Type::Method::Parameter>::Iterator i = method.parameters.begin(), end = method.parameters.end(); i != end; ++i)
        {
          const ReflectorData::Type::Method::Parameter& parameter = *i;
          write(String(String("      {") + formatString(parameter.name) +  "},"));
        }
        write(String("    };"));
      }
    }
    if(!type.methods.isEmpty())
    {
      write(String("    const Reflected::Type::Method _") + getVarName(type.name) + "_Methods[] = { ");
      for(List<ReflectorData::Type::Method>::Iterator i = type.methods.begin(), end = type.methods.end(); i != end; ++i)
      {
        ReflectorData::Type::Method& method = *i;
        write(String("      {") + formatString(method.name) + "},");
      }
    }
    write(String("    };"));
    write(String("    extern const Reflected::Type ") + getVarName(type.name) + " = { ");
    write(String("      ") + formatString(type.name) + ",");
    write(String("      ") + formatString(type.description) + ",");
    write(String("      _") + getVarName(type.name) + "_Methods,");
    write(String("      ") + String::fromUInt64(type.methods.size()) + ",");
    write(String("      0,"));
    write(String("      0,"));
    if(type.baseTypes.isEmpty())
      write(String("      0,"));
    else
      write(String("      _") + getVarName(type.name) + "_BaseTypes,");
    write(String("      ") + String::fromUInt64(type.baseTypes.size()) + ",");
    write(String("    };"));
    write(String("  ") + getNamespaceSuffix(type.name));
  }

  write(String("};"));
  write();

  for(HashMap<String, ReflectorData::Type>::Iterator i = data.types.begin(), end = data.types.end(); i != end; ++i)
  {
    const ReflectorData::Type& type = *i;
    if(type.external || type.reflectionType != type.objectType)
      continue;
    write(String("const Reflected::Type& ") + type.name + "::getReflectedType() const {return Reflection::" + type.name + ";}");
    write();
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
    prefix.append(String("namespace ") + type.substr(namespaceStart - start, namespaceEnd - namespaceStart) + " {");
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
  return type;
}

String Generator::formatString(const String& str)
{
  return String("\"") + str + String("\"");
}
