
#include <clang-c/Index.h>

#include <nstd/Array.h>

#include "Parser.h"

class Parser::Private
{
public:
  class Cursor
  {
  public:
    Cursor() : cursor(clang_getNullCursor()) {}
    Cursor(const CXCursor& cursor) : cursor(cursor) {}
    operator size_t() const
    {
      size_t hashCode = cursor.kind;
      hashCode *= 16807;
      hashCode ^= cursor.xdata;
      for(size_t i = 0; i < sizeof(cursor.data) / sizeof(*cursor.data); ++i)
      {
        hashCode *= 16807;
        hashCode ^= (size_t)cursor.data[i];
      }
      return hashCode;
    }
    bool operator==(const Cursor& other) const
    {
      if(clang_equalCursors(cursor, other.cursor))
        return true;
      return false;
    }
  private:
    CXCursor cursor;
  };

  enum Action
  {
    noAction,
    classAction,
    methodAction,
    paramAction,
  };

public:
  ParserData& data;
  CXSourceLocation lastLocation;
  Action lastAction;
  HashMap<Cursor, ParserData::TypeDecl*> typesByCursor;
  HashMap<Cursor, ParserData::TypeDecl::MethodDecl*> methodsByCursor;

public:
  Private(ParserData& data) : data(data) {}

  bool_t parse(const String& sourceFile, const String& headerFile, const List<String>& additionalArgs)
  {
    data.headerFile = headerFile;

    Array<const char*> args(additionalArgs.size());
    for(List<String>::Iterator i = additionalArgs.begin(), end = additionalArgs.end(); i != end; ++i)
      args.append(*i);

    CXIndex index = clang_createIndex(1, 1);
    CXTranslationUnit tu = clang_createTranslationUnitFromSourceFile(index, sourceFile.isEmpty() ? (const char*)headerFile : (const char*)sourceFile, 
      (int)args.size(), args, 0, 0); //clang_createTranslationUnit(index, sourceFile);
    CXCursor cursor = clang_getTranslationUnitCursor(tu);
    lastLocation = clang_getCursorLocation(cursor);
    lastAction = noAction;
    clang_visitChildren(cursor, visitChildrenCallback, this);

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);
    return true;
  }

public:
  static CXChildVisitResult visitChildrenCallback(CXCursor cursor, CXCursor parent, CXClientData client_data)
  {
    Private& parser = *(Private*)client_data;

    Action action = noAction;

    switch(cursor.kind)
    {
    case CXCursor_CXXBaseSpecifier:
      {
        ParserData::TypeDecl* typeDecl = *parser.typesByCursor.find(parent);
        if(typeDecl)
        {
          CXType type = clang_getCursorType(cursor);
          CXString typeName = clang_getTypeSpelling(type);
          String name = String::fromCString(clang_getCString(typeName));
          clang_disposeString(typeName);
          typeDecl->baseTypes.append(name);
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
          HashMap<String, ParserData::TypeDecl>::Iterator it = parser.data.declarations.find(name);
          ParserData::TypeDecl& typeDecl = it == parser.data.declarations.end() ? parser.data.declarations.append(name, {}) : *it;
          typeDecl.name = name;
          typeDecl.type = ParserData::TypeDecl::classType;
          action = classAction;
          parser.typesByCursor.append(cursor, &typeDecl);
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

            HashMap<String, ParserData::TypeDecl>::Iterator it = parser.data.declarations.find(name);
            ParserData::TypeDecl& typeDecl = it == parser.data.declarations.end() ? parser.data.declarations.append(name, {}) : *it;
            typeDecl.name = name;
            typeDecl.type = cursor.kind == CXCursor_ClassDecl ? ParserData::TypeDecl::classType : (cursor.kind == CXCursor_EnumDecl ? ParserData::TypeDecl::enumType : ParserData::TypeDecl::structType);
            action = classAction;
            parser.typesByCursor.append(cursor, &typeDecl);
            break;
          }
        }
        break;
      }
    case CXCursor_CXXMethod:
      {
        ParserData::TypeDecl* typeDecl = *parser.typesByCursor.find(parent);
        if(typeDecl)
        {
          CXType type = clang_getCursorType(cursor);
          CXString spelling = clang_getCursorSpelling(cursor);
          CXString typeSpelling = clang_getTypeSpelling(type);
          //Console::printf("%s - %s\n", (const tchar_t*)extractReturnType(clang_getCString(typeSpelling)), clang_getCString(spelling));
          ParserData::TypeDecl::MethodDecl& method = typeDecl->methods.append({String::fromCString(clang_getCString(spelling)), String::fromCString(clang_getCString(typeSpelling))});
          clang_disposeString(typeSpelling);
          clang_disposeString(spelling);
          action = methodAction;
          parser.methodsByCursor.append(cursor, &method);
        }
      }
      break;
    case CXCursor_TemplateTypeParameter:
    case CXCursor_NonTypeTemplateParameter:
    case CXCursor_TemplateTemplateParameter:
      if(parent.kind == CXCursor_ClassTemplate)
      {
        ParserData::TypeDecl* typeDecl = *parser.typesByCursor.find(parent);
        if(typeDecl)
        {
          CXType type = clang_getCursorType(cursor);
          CXString typeSpelling = clang_getTypeSpelling(type);
          CXString paramSpelling = clang_getCursorSpelling(cursor);
          String typeName = String::fromCString(clang_getCString(typeSpelling));
          String paramName = String::fromCString(clang_getCString(paramSpelling));
          //Console::printf("%s - %s\n", (const tchar_t*)typeName, (const tchar_t*)paramName);
          clang_disposeString(typeSpelling);
          clang_disposeString(paramSpelling);
          typeDecl->templateParams.append(paramName, typeName);
        }
      }
      break;
    case CXCursor_ParmDecl:
      {
        ParserData::TypeDecl* typeDecl = *parser.typesByCursor.find(clang_getCursorSemanticParent(parent));
        if(typeDecl)
        {
          ParserData::TypeDecl::MethodDecl* methodDecl = *parser.methodsByCursor.find(parent);
          if(methodDecl)
          {
            CXType type = clang_getCursorType(cursor);
            CXString typeSpelling = clang_getTypeSpelling(type);
            CXString paramSpelling = clang_getCursorSpelling(cursor);
            methodDecl->parameters.append({String::fromCString(clang_getCString(paramSpelling)), String::fromCString(clang_getCString(typeSpelling))});
            if(methodDecl->parameters.back().name == "p0")
            {
              int k = 42;
            }
            clang_disposeString(typeSpelling);
            clang_disposeString(paramSpelling);
            action = paramAction;
          }
        }
        break;
      }
    case CXCursor_IntegerLiteral:
    case CXCursor_StringLiteral:
    case CXCursor_FloatingLiteral:
      {
        if(parser.lastAction == paramAction)
        {
          ParserData::TypeDecl::MethodDecl* methodDecl = *parser.methodsByCursor.find(clang_getCursorSemanticParent(parent));
          if(methodDecl)
          {
            if(!methodDecl->parameters.isEmpty())
            {
              CXSourceRange range = clang_getCursorExtent(cursor);
              CXToken *tokens = 0;
              unsigned int tokensCount = 0;
              CXTranslationUnit tu = clang_Cursor_getTranslationUnit(cursor);
              clang_tokenize(tu, range, &tokens, &tokensCount);
              if(tokensCount)
              {
                CXString spelling = clang_getTokenSpelling(tu, tokens[0]);
                methodDecl->parameters.back().value = String::fromCString(clang_getCString(spelling));
                clang_disposeString(spelling);
              }
              clang_disposeTokens(tu, tokens, tokensCount);
            }
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
      ParserData::TypeDecl* typeDecl = parser.typesByCursor.back();
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
                typeDecl->innerComments.append(String::fromCString(clang_getCString(spell)));
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
        parser.typesByCursor.back()->file = String::fromCString(clang_getCString(filename));
        clang_disposeString(filename);
      }

      if(!comment.isEmpty())
      {
        switch(action)
        {
        case classAction:
          parser.typesByCursor.back()->comment = comment;
          break;
        case methodAction:
          parser.methodsByCursor.back()->comment = comment;
          break;
        default:
          break;
        }
      }
    }
    parser.lastLocation = location;
    parser.lastAction = action;

    // visit children recursively
    clang_visitChildren(cursor, visitChildrenCallback, &parser);

    return CXChildVisit_Continue;
  }
};

Parser::Parser() : p(new Private(data)) {}
Parser::~Parser() {delete p;}
bool_t Parser::parse(const String& sourceFile, const String& headerFile, const List<String>& additionalArgs) {return p->parse(sourceFile, headerFile, additionalArgs);}

