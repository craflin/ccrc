
#include <nstd/Console.h>
#include <nstd/Process.h>
#include <nstd/HashMap.h>
#include <nstd/List.h>

#include <clang-c/Index.h>

void_t usage(char_t* argv[])
{
  Console::errorf("Usage: %s <header> <source> -o <output>\n", argv[0]);
}

class MetaTypeDecl
{
public:
  enum Type
  {
    enumType,
    classType,
    structType
  };

public:
  String name;
  String comment;
  Type type;
  HashMap<String, String> templateParams;
  List<String> baseTypes;

public:
  void_t addTemplateParam(const String& paramName, const String& typeName) {templateParams.append(paramName, typeName);}
  void_t addBaseType(const String& type) {baseTypes.append(type);}

  void_t print()
  {
    String printName;
    if(!comment.isEmpty())
    {
      printName.append(comment);
      printName.append('\n');
    }
    switch(type)
    {
    case enumType:
      printName.append("enum");
      break;
    case classType:
      printName.append("class");
      break;
    case structType:
      printName.append("struct");
      break;
    }
    printName.append(" - ");
    printName.append(name);
    if(!templateParams.isEmpty())
    {
      printName.append('<');
      for(HashMap<String, String>::Iterator i = templateParams.begin(), end = templateParams.end(); i != end;)
      {
        printName.append(i.key());
        if(++i == end)
          break;
        printName.append(", ");
      }
      printName.append('>');
    }
    if(!baseTypes.isEmpty())
    {
      printName.append(" : ");
      for(List<String>::Iterator i = baseTypes.begin(), end = baseTypes.end(); i != end;)
      {
        printName.append(*i);
        if(++i == end)
          break;
        printName.append(", ");
      }
    }
    Console::printf("%s\n", (const tchar_t*)printName);
  }
};

class MetaInfoData
{
public:
  MetaTypeDecl& getMetaTypeDecl(const String& name)
  {
    HashMap<String, MetaTypeDecl>::Iterator it = declarations.find(name);
    if(it == declarations.end())
      return declarations.append(name, MetaTypeDecl());
    return *it;
  }

  MetaTypeDecl& getLastMetaTypeDecl()
  {
    return declarations.back();
  }

  void_t print()
  {
    for(HashMap<String, MetaTypeDecl>::Iterator i = declarations.begin(), end = declarations.end(); i != end; ++i)
      i->print();
  }

private:
  HashMap<String, MetaTypeDecl> declarations;
};

class VisitorContext
{
public:
  MetaInfoData metaInfoData;

  CXSourceLocation lastLocation;
};

CXChildVisitResult visitChildrenCallback(CXCursor cursor,
                                         CXCursor parent,
                                         CXClientData client_data) {
  VisitorContext& context = *(VisitorContext*)client_data;

  CXSourceLocation location = clang_getCursorLocation(cursor);

  //show_spell(cursor);
  //show_linkage(cursor);
  //show_cursor_kind(cursor);
  //show_type(cursor);
  //show_parent(cursor, parent);
  //show_location(cursor);
  //show_usr(cursor);
  //show_included_file(cursor);
  //Console::printf("\n");

  switch(cursor.kind)
  {
  case CXCursor_CXXBaseSpecifier:
    {
      CXType type = clang_getCursorType(cursor);
      CXString typeName = clang_getTypeSpelling(type);
      String name = String::fromCString(clang_getCString(typeName));
      clang_disposeString(typeName);
      MetaTypeDecl& metaTypeDecl = context.metaInfoData.getLastMetaTypeDecl();
      metaTypeDecl.addBaseType(name);
      break;
    }
  case CXCursor_ClassTemplate:
    {
      CXCursorKind curKind = clang_getTemplateCursorKind(cursor);
      if(clang_isDeclaration(curKind))
      {
        CXString spell = clang_getCursorSpelling(cursor);
        String name = String::fromCString(clang_getCString(spell));
        clang_disposeString(spell);

        CXCursor parent = clang_getCursorSemanticParent(cursor);
        while(parent.kind != CXCursor_TranslationUnit)
        {
          CXString parentName = clang_getCursorSpelling(parent);
          name.prepend("::");
          name.prepend(String::fromCString(clang_getCString(parentName)));
          clang_disposeString(parentName);
          parent = clang_getCursorSemanticParent(parent);
        }
        //Console::printf("%s (template)\n", (const tchar_t*)name);
        MetaTypeDecl& metaTypeDecl = context.metaInfoData.getMetaTypeDecl(name);
        metaTypeDecl.name = name;
        metaTypeDecl.type = MetaTypeDecl::classType;
      }
      break;
    }
  case CXCursor_ClassDecl:
  case CXCursor_EnumDecl:
  case CXCursor_StructDecl:
    {
      CXType type = clang_getCursorType(cursor);
      switch(type.kind)
      {
      case CXType_Invalid:
      case CXType_Unexposed:
        break;
      default:
        {
          CXString typeName = clang_getTypeSpelling(type);
          String name = String::fromCString(clang_getCString(typeName));
          //Console::printf("%s\n", (const tchar_t*)name);
          clang_disposeString(typeName);
          
          CXTranslationUnit tu = clang_Cursor_getTranslationUnit(cursor);
          CXSourceLocation lastLocation = context.lastLocation;
          CXFile startFile, endFile;
          unsigned int startOffset, endOffset;
          clang_getFileLocation(lastLocation, &startFile, 0, 0, &startOffset);
          clang_getFileLocation(location, &endFile, 0, 0, &endOffset);
          if(startFile != endFile || startOffset > endOffset)
            lastLocation = clang_getLocationForOffset(tu, endFile, 0);
          CXSourceRange range = clang_getRange(lastLocation, location);
          CXToken* tokens;
          unsigned int tokensCount;
          clang_tokenize(tu, range, &tokens, &tokensCount);

          String comment;
          for(unsigned int i = tokensCount; i > 0; --i)
            if(clang_getTokenKind(tokens[i - 1]) == CXToken_Comment)
            {
              CXString spell = clang_getTokenSpelling(tu, tokens[i - 1]);
              if(String::compare(clang_getCString(spell), "/**", 3) == 0)
                comment  = String::fromCString(clang_getCString(spell));
              clang_disposeString(spell);
              break;
            }
          clang_disposeTokens(tu, tokens, tokensCount);

          MetaTypeDecl& metaTypeDecl = context.metaInfoData.getMetaTypeDecl(name);
          metaTypeDecl.name = name;
          metaTypeDecl.type = cursor.kind == CXCursor_ClassDecl ? MetaTypeDecl::classType : (cursor.kind == CXCursor_EnumDecl ? MetaTypeDecl::enumType : MetaTypeDecl::structType);
          metaTypeDecl.comment = comment;
          break;
        }
      }
      break;
    }
  case CXCursor_TemplateTypeParameter:
  case CXCursor_NonTypeTemplateParameter:
  case CXCursor_TemplateTemplateParameter:
    if(parent.kind == CXCursor_ClassTemplate)
    {
      MetaTypeDecl& metaTypeDecl = context.metaInfoData.getLastMetaTypeDecl();
      CXType type = clang_getCursorType(cursor);
      CXString typeSpelling = clang_getTypeSpelling(type);
      CXString paramSpelling = clang_getCursorSpelling(cursor);
      String typeName = String::fromCString(clang_getCString(typeSpelling));
      String paramName = String::fromCString(clang_getCString(paramSpelling));
      //Console::printf("%s - %s\n", (const tchar_t*)typeName, (const tchar_t*)paramName);
      clang_disposeString(typeSpelling);
      clang_disposeString(paramSpelling);
      metaTypeDecl.addTemplateParam(paramName, typeName);
    }
    break;
  default:
    break;
  }

  context.lastLocation = location;

  // visit children recursively
  clang_visitChildren(cursor, visitChildrenCallback, &context);

  return CXChildVisit_Continue;
}

int_t main(int_t argc, char_t* argv[])
{
  String headerFile;
  String sourceFile;
  String outputFile;
  {
    Process::Option options[] = {
        {'o', "output", Process::argumentFlag},
        {'h', "help", Process::optionFlag},
    };
    Process::Arguments arguments(argc, argv, options);
    int_t character;
    String argument;
    while(arguments.read(character, argument))
      switch(character)
      {
      case 'o':
        outputFile = argument;
        break;
      case 0:
        if(headerFile.isEmpty())
          headerFile = argument;
        else if(sourceFile.isEmpty())
          sourceFile = argument;
        else
          return usage(argv), 1;
        break;
      case '?':
        Console::errorf("Unknown option: %s.\n", (const char_t*)argument);
        return 1;
      case ':':
        Console::errorf("Option %s required an argument.\n", (const char_t*)argument);
        return 1;
      default:
        return usage(argv), 1;
      }
  }

  if(headerFile.isEmpty() || sourceFile.isEmpty() || outputFile.isEmpty())
    return usage(argv), 1;

  CXIndex index = clang_createIndex(1, 1);
  CXTranslationUnit tu = clang_createTranslationUnitFromSourceFile(index, sourceFile, 0, 0, 0, 0); //clang_createTranslationUnit(index, sourceFile);


  VisitorContext context;
  CXCursor cursor = clang_getTranslationUnitCursor(tu);
  clang_visitChildren(cursor, visitChildrenCallback, &context);

  clang_disposeTranslationUnit(tu);
  clang_disposeIndex(index);

  context.metaInfoData.print();

  /*

  // find a compiler
  String path = Process::getEnvironmentVariable("PATH");
  //List<String> paths;
  //path.split(';', paths);
  // todo
  String compiler = "C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\bin\\cl.exe";
  Process::setEnvironmentVariable("INCLUDE", "C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\INCLUDE;C:\\Program Files (x86)\\Windows Kits\\8.1\\include\\shared;C:\\Program Files (x86)\\Windows Kits\\8.1\\include\\um;C:\\Program Files (x86)\\Windows Kits\\8.1\\include\\winrt;");

  */

  return 0;
}
