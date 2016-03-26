
#include "MetaInfoData.h"
#include "Generator.h"

bool_t Generator::generate(const String& outputFile, const String& headerFile, const MetaInfoData& metaInfoData)
{
  if(!file.open(outputFile, File::writeFlag))
    return false;

  write();
  write(String("#include <Meta.h>"));
  write();
  write(String("#include \"") + headerFile + "\"");
  write();

  for(HashMap<String, MetaTypeDecl>::Iterator i = metaInfoData.declarations.begin(), end = metaInfoData.declarations.end(); i != end; ++i)
  {
    const MetaTypeDecl& type = *i;
    if(type.file == headerFile && type.isReflected(metaInfoData))
    {
      write(String("const Meta::Type& ") + type.name + "::getMetaType() const");
      write("{");
      write("}");
      write();
    }
  }

  return true;
}
