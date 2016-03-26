
#include <nstd/Console.h>
#include <nstd/Process.h>
#include <nstd/HashMap.h>
#include <nstd/List.h>

#include <clang-c/Index.h>

void_t usage(char_t* argv[])
{
  Console::errorf("Usage: %s <header> <source> -o <output>\n", argv[0]);
}

class MethodDecl
{
public:
  struct Param
  {
    String type;
    String name;
  };

public:
  String name;
  String comment;
  String type;
  List<Param> params;

public:
  void_t addParam(const String& name, const String& typeName)
  {
    Param& param = params.append(Param());
    param.name = name;
    param.type = typeName;
  }

  void_t print()
  {
    if(!comment.isEmpty())
      Console::printf("  %s\n", (const tchar_t*)comment);
    String printName;
    printName.append(getReturnType());
    printName.append(' ');
    printName.append(name);
    printName.append('(');
    for(List<Param>::Iterator i = params.begin(), end = params.end();;)
    {
      printName.append(i->type);
      if(!i->name.isEmpty())
      {
        printName.append(' ');
        printName.append(i->name);
      }
      if(++i == end)
        break;
      printName.append(", ");
    }
    printName.append(')');
    Console::printf("  %s\n", (const tchar_t*)printName);
  }

  String getReturnType() const
  {
    const char* functionType = type;
    const char* p = String::findOneOf(functionType, " <");
    if(!p)
      return String();
    if(*p == ' ')
      return String::fromCString(functionType, p - functionType);
    for(size_t depth = 1;;)
    {
      p = String::findOneOf(p + 1, "<>");
      if(*p == '<')
        ++depth;
      else if(depth == 1)
        return String::fromCString(functionType, p + 1 - functionType);
      else
        --depth;
    }
  }
};

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
  List<MethodDecl> methods;
  List<String> innerComments;

public:
  void_t addTemplateParam(const String& paramName, const String& typeName) {templateParams.append(paramName, typeName);}
  void_t addBaseType(const String& type) {baseTypes.append(type);}
  void_t addMethodDecl(const String& name, const String& type)
  {
    MethodDecl& method = methods.append(MethodDecl());
    method.name = name;
    method.type = type;
  }
  MethodDecl* getLastMethodDecl() {return methods.isEmpty() ? 0 : &methods.back();}
  void_t addInnerComment(const String& comment) {innerComments.append(comment);}

  void_t print()
  {
    if(!comment.isEmpty())
      Console::printf("%s\n", (const tchar_t*)comment);
    String printName;
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
      for(HashMap<String, String>::Iterator i = templateParams.begin(), end = templateParams.end();;)
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
      for(List<String>::Iterator i = baseTypes.begin(), end = baseTypes.end();;)
      {
        printName.append(*i);
        if(++i == end)
          break;
        printName.append(", ");
      }
    }
    Console::printf("%s\n", (const tchar_t*)printName);
    for(List<MethodDecl>::Iterator i = methods.begin(), end = methods.end(); i != end; ++i)
      i->print();
    for(List<String>::Iterator i = innerComments.begin(), end = innerComments.end(); i != end; ++i)
      Console::printf("  %s\n", (const tchar_t*)*i);
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

  MetaTypeDecl* getLastMetaTypeDecl() {return declarations.isEmpty() ? 0 : &declarations.back();}

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

  //show_spell(cursor);
  //show_linkage(cursor);
  //show_cursor_kind(cursor);
  //show_type(cursor);
  //show_parent(cursor, parent);
  //show_location(cursor);
  //show_usr(cursor);
  //show_included_file(cursor);
  //Console::printf("\n");

  enum Action
  {
    noAction,
    classAction,
    methodAction,
  } action = noAction;

  switch(cursor.kind)
  {
  case CXCursor_CXXBaseSpecifier:
    {
      MetaTypeDecl* metaTypeDecl = context.metaInfoData.getLastMetaTypeDecl();
      if(metaTypeDecl)
      {
        CXType type = clang_getCursorType(cursor);
        CXString typeName = clang_getTypeSpelling(type);
        String name = String::fromCString(clang_getCString(typeName));
        clang_disposeString(typeName);
        metaTypeDecl->addBaseType(name);
      }
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
        action = classAction;
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

          MetaTypeDecl& metaTypeDecl = context.metaInfoData.getMetaTypeDecl(name);
          metaTypeDecl.name = name;
          metaTypeDecl.type = cursor.kind == CXCursor_ClassDecl ? MetaTypeDecl::classType : (cursor.kind == CXCursor_EnumDecl ? MetaTypeDecl::enumType : MetaTypeDecl::structType);
          action = classAction;

          CXTranslationUnit tu = clang_Cursor_getTranslationUnit(cursor);
          CXSourceRange classRange = clang_getCursorExtent(cursor);
          CXToken* tokens;
          unsigned int tokensCount;
          clang_tokenize(tu, classRange, &tokens, &tokensCount);
          int depth = 0;
          for(unsigned int i = 0; i < tokensCount; ++i)
          {
            CXToken& token = tokens[i];
            switch(clang_getTokenKind(token))
            {
            case CXToken_Comment:
              {
                if(depth == 1)
                {
                  CXString spell = clang_getTokenSpelling(tu, tokens[i]);
                  if(String::compare(clang_getCString(spell), "/**", 3) == 0)
                    metaTypeDecl.addInnerComment(String::fromCString(clang_getCString(spell)));
                  clang_disposeString(spell);
                }
                break;
              }
            case CXToken_Punctuation:
              {
                CXString spell = clang_getTokenSpelling(tu, tokens[i]);
                const char* tokenCStr = clang_getCString(spell);
                if(String::compare(tokenCStr, "{") == 0)
                  ++depth;
                else if(String::compare(tokenCStr, "}") == 0)
                  --depth;
                clang_disposeString(spell);
                break;
              }
            default:
              break;
            }
          }
          clang_disposeTokens(tu, tokens, tokensCount);

          break;
        }
      }
      break;
    }
  case CXCursor_CXXMethod:
    {
      MetaTypeDecl* metaTypeDecl = context.metaInfoData.getLastMetaTypeDecl();
      if(metaTypeDecl)
      {
        CXType type = clang_getCursorType(cursor);
        CXString spelling = clang_getCursorSpelling(cursor);
        CXString typeSpelling = clang_getTypeSpelling(type);
        //Console::printf("%s - %s\n", (const tchar_t*)extractReturnType(clang_getCString(typeSpelling)), clang_getCString(spelling));
        metaTypeDecl->addMethodDecl(String::fromCString(clang_getCString(spelling)), String::fromCString(clang_getCString(typeSpelling)));
        clang_disposeString(typeSpelling);
        clang_disposeString(spelling);
        action = methodAction;
      }
    }
    break;
  case CXCursor_TemplateTypeParameter:
  case CXCursor_NonTypeTemplateParameter:
  case CXCursor_TemplateTemplateParameter:
    if(parent.kind == CXCursor_ClassTemplate)
    {
      MetaTypeDecl* metaTypeDecl = context.metaInfoData.getLastMetaTypeDecl();
      if(metaTypeDecl)
      {
        CXType type = clang_getCursorType(cursor);
        CXString typeSpelling = clang_getTypeSpelling(type);
        CXString paramSpelling = clang_getCursorSpelling(cursor);
        String typeName = String::fromCString(clang_getCString(typeSpelling));
        String paramName = String::fromCString(clang_getCString(paramSpelling));
        //Console::printf("%s - %s\n", (const tchar_t*)typeName, (const tchar_t*)paramName);
        clang_disposeString(typeSpelling);
        clang_disposeString(paramSpelling);
        metaTypeDecl->addTemplateParam(paramName, typeName);
      }
    }
    break;
  case CXCursor_ParmDecl:
    {
      MetaTypeDecl* metaTypeDecl = context.metaInfoData.getLastMetaTypeDecl();
      if(metaTypeDecl)
      {
        MethodDecl* methodDecl = metaTypeDecl->getLastMethodDecl();
        if(methodDecl)
        {
          CXType type = clang_getCursorType(cursor);
          CXString typeSpelling = clang_getTypeSpelling(type);
          CXString paramSpelling = clang_getCursorSpelling(cursor);
          methodDecl->addParam(String::fromCString(clang_getCString(paramSpelling)), String::fromCString(clang_getCString(typeSpelling)));
          clang_disposeString(typeSpelling);
          clang_disposeString(paramSpelling);
        }
      }
      break;
    }
  default:
    //{
    //  CXString spelling = clang_getCursorSpelling(cursor);
    //  if(String::fromCString(clang_getCString(spelling)) == "param1")
    //  {
    //    int k = 32;
    //  }
    //  clang_disposeString(spelling);
    //}
    break;
  }

  CXSourceRange cursorRange = clang_getCursorExtent(cursor);
  CXSourceLocation location = clang_getRangeStart(cursorRange);
  if(action != noAction)
  {
    CXTranslationUnit tu = clang_Cursor_getTranslationUnit(cursor);
    CXFile startFile, endFile;
    unsigned int startOffset, endOffset;
    clang_getFileLocation(context.lastLocation, &startFile, 0, 0, &startOffset);
    clang_getFileLocation(location, &endFile, 0, 0, &endOffset);
    if(startFile != endFile || startOffset > endOffset)
      context.lastLocation = clang_getLocationForOffset(tu, endFile, 0);
    CXSourceRange range = clang_getRange(context.lastLocation, location);
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

    if(!comment.isEmpty())
    {
      switch(action)
      {
      case classAction:
        context.metaInfoData.getLastMetaTypeDecl()->comment = comment;
        break;
      case methodAction:
        context.metaInfoData.getLastMetaTypeDecl()->getLastMethodDecl()->comment = comment;
        break;
      default:
        break;
      }
    }
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



  VisitorContext context;
  {
    CXIndex index = clang_createIndex(1, 1);
    CXTranslationUnit tu = clang_createTranslationUnitFromSourceFile(index, sourceFile, 0, 0, 0, 0); //clang_createTranslationUnit(index, sourceFile);
    CXCursor cursor = clang_getTranslationUnitCursor(tu);
    context.lastLocation = clang_getCursorLocation(cursor);
    clang_visitChildren(cursor, visitChildrenCallback, &context);

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);
  }

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
