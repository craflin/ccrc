
#include <clang-c/Index.h>

#include "Parser.h"

class Parser::Private
{
public:
  MetaInfoData& metaInfoData;
  CXSourceLocation lastLocation;

public:
  Private(MetaInfoData& metaInfoData) : metaInfoData(metaInfoData) {}

  bool_t parse(const String& sourceFile, const String& headerFile)
  {
    CXIndex index = clang_createIndex(1, 1);
    CXTranslationUnit tu = clang_createTranslationUnitFromSourceFile(index, sourceFile.isEmpty() ? (const char*)headerFile : (const char*)sourceFile, 0, 0, 0, 0); //clang_createTranslationUnit(index, sourceFile);
    CXCursor cursor = clang_getTranslationUnitCursor(tu);
    lastLocation = clang_getCursorLocation(cursor);
    clang_visitChildren(cursor, visitChildrenCallback, this);

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);
    return true;
  }

public:
  static CXChildVisitResult visitChildrenCallback(CXCursor cursor, CXCursor parent, CXClientData client_data)
  {
    Private& parser = *(Private*)client_data;

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
        MetaTypeDecl* metaTypeDecl = parser.metaInfoData.getLastMetaTypeDecl();
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
          MetaTypeDecl& metaTypeDecl = parser.metaInfoData.getMetaTypeDecl(name);
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

            MetaTypeDecl& metaTypeDecl = parser.metaInfoData.getMetaTypeDecl(name);
            metaTypeDecl.name = name;
            metaTypeDecl.type = cursor.kind == CXCursor_ClassDecl ? MetaTypeDecl::classType : (cursor.kind == CXCursor_EnumDecl ? MetaTypeDecl::enumType : MetaTypeDecl::structType);
            action = classAction;
            break;
          }
        }
        break;
      }
    case CXCursor_CXXMethod:
      {
        MetaTypeDecl* metaTypeDecl = parser.metaInfoData.getLastMetaTypeDecl();
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
        MetaTypeDecl* metaTypeDecl = parser.metaInfoData.getLastMetaTypeDecl();
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
        MetaTypeDecl* metaTypeDecl = parser.metaInfoData.getLastMetaTypeDecl();
        if(metaTypeDecl)
        {
          MetaMethodDecl* methodDecl = metaTypeDecl->getLastMethodDecl();
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

    if(action == classAction)
    {
      MetaTypeDecl* metaTypeDecl = parser.metaInfoData.getLastMetaTypeDecl();
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
                metaTypeDecl->addInnerComment(String::fromCString(clang_getCString(spell)));
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
    }

    CXSourceRange cursorRange = clang_getCursorExtent(cursor);
    CXSourceLocation location = clang_getRangeStart(cursorRange);
    if(action != noAction)
    {
      CXTranslationUnit tu = clang_Cursor_getTranslationUnit(cursor);
      CXFile startFile, endFile;
      unsigned int startOffset, endOffset;
      clang_getFileLocation(parser.lastLocation, &startFile, 0, 0, &startOffset);
      clang_getFileLocation(location, &endFile, 0, 0, &endOffset);
      if(startFile != endFile || startOffset > endOffset)
        parser.lastLocation = clang_getLocationForOffset(tu, endFile, 0);
      CXSourceRange range = clang_getRange(parser.lastLocation, location);
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

      if(action == classAction)
      {
        CXString filename = clang_getFileName(endFile);
        parser.metaInfoData.getLastMetaTypeDecl()->file = String::fromCString(clang_getCString(filename));
        clang_disposeString(filename);
      }

      if(!comment.isEmpty())
      {
        switch(action)
        {
        case classAction:
          parser.metaInfoData.getLastMetaTypeDecl()->comment = comment;
          break;
        case methodAction:
          parser.metaInfoData.getLastMetaTypeDecl()->getLastMethodDecl()->comment = comment;
          break;
        default:
          break;
        }
      }
    }
    parser.lastLocation = location;

    // visit children recursively
    clang_visitChildren(cursor, visitChildrenCallback, &parser);

    return CXChildVisit_Continue;
  }
};

Parser::Parser() : p(new Private(metaInfoData)) {}
Parser::~Parser() {delete p;}
bool_t Parser::parse(const String& sourceFile, const String& headerFile) {return p->parse(sourceFile, headerFile);}

