#include <<%= projectName %>/Tooling.hpp> // IWYU pragma: associated

#include <flexlib/reflect/ReflTypes.hpp>
#include <flexlib/reflect/ReflectAST.hpp>
#include <flexlib/reflect/ReflectionCache.hpp>
#include <flexlib/ToolPlugin.hpp>
#include <flexlib/core/errors/errors.hpp>
#include <flexlib/utils.hpp>
#include <flexlib/funcParser.hpp>
#include <flexlib/inputThread.hpp>
#include <flexlib/clangUtils.hpp>
#include <flexlib/clangPipeline.hpp>
#include <flexlib/annotation_parser.hpp>
#include <flexlib/annotation_match_handler.hpp>
#include <flexlib/matchers/annotation_matcher.hpp>
#include <flexlib/options/ctp/options.hpp>
#if defined(CLING_IS_ON)
#include "flexlib/ClingInterpreterModule.hpp"
#endif // CLING_IS_ON

#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/Lexer.h>

#include <base/cpu.h>
#include <base/bind.h>
#include <base/command_line.h>
#include <base/debug/alias.h>
#include <base/debug/stack_trace.h>
#include <base/memory/ptr_util.h>
#include <base/sequenced_task_runner.h>
#include <base/trace_event/trace_event.h>
#include <base/logging.h>
#include <base/strings/string_util.h>
#include <base/strings/string_number_conversions.h>
#include <base/strings/string_split.h>
#include <base/strings/utf_string_conversions.h>
#include <base/stl_util.h>
#include <base/files/file_util.h>

#include <any>
#include <string>
#include <vector>
#include <regex>
#include <iostream>
#include <fstream>

namespace plugin {

namespace {

static void insertCodeAfterPos(
  const std::string& processedAnnotation
  , clang::AnnotateAttr* annotateAttr
  , const clang_utils::MatchResult& matchResult
  , clang::Rewriter& rewriter
  , const clang::Decl* nodeDecl
  , clang::SourceLocation& nodeStartLoc
  , clang::SourceLocation& nodeEndLoc
  , const std::string& codeToInsert
){
  clang::SourceManager &SM
    = rewriter.getSourceMgr();

  const clang::LangOptions& langOptions
    = rewriter.getLangOpts();

  clang_utils::expandLocations(
    nodeStartLoc, nodeEndLoc, rewriter);

  clang::CharSourceRange charSourceRange(
    clang::SourceRange{nodeStartLoc, nodeEndLoc},
    true // IsTokenRange
  );

#if defined(DEBUG_VERBOSE_PLUGIN)
  if(charSourceRange.isValid()) {
    StringRef sourceText
      = clang::Lexer::getSourceText(
          charSourceRange
          , SM, langOptions, 0);
    DCHECK(nodeStartLoc.isValid());
    VLOG(9)
      << "original code: "
      << sourceText.str()
      << " at "
      << nodeStartLoc.printToString(SM);
  } else {
    DCHECK(nodeStartLoc.isValid());
    LOG(ERROR)
      << "variable declaration with"
         " annotation"
         " must have initializer"
         " that is valid: "
      << nodeStartLoc.printToString(SM);
    return;
  }
#endif // DEBUG_VERBOSE_PLUGIN

  // MeasureTokenLength gets us past the last token,
  // and adding 1 gets us past the ';'
  int offset = clang::Lexer::MeasureTokenLength(
    nodeEndLoc,
    SM,
    langOptions) + 1;

  clang::SourceLocation realEnd
    = nodeEndLoc.getLocWithOffset(offset);

  rewriter.InsertText(realEnd, codeToInsert,
    /*InsertAfter=*/true, /*IndentNewLines*/ false);
}

} // namespace

ExternalPluginTooling::ExternalPluginTooling(
  const ::plugin::ToolPlugin::Events::RegisterAnnotationMethods& event
#if defined(CLING_IS_ON)
  , ::cling_utils::ClingInterpreter* clingInterpreter
#endif // CLING_IS_ON
) : clingInterpreter_(clingInterpreter)
{
  DCHECK(clingInterpreter_);

  DETACH_FROM_SEQUENCE(sequence_checker_);

  DCHECK(event.sourceTransformPipeline);
  ::clang_utils::SourceTransformPipeline& sourceTransformPipeline
    = *event.sourceTransformPipeline;

  sourceTransformRules_
    = &sourceTransformPipeline.sourceTransformRules;
}

ExternalPluginTooling::~ExternalPluginTooling()
{
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
}

void ExternalPluginTooling::<%= projectName %>_custom_annotation(
  const std::string& processedAnnotation
  , clang::AnnotateAttr* annotateAttr
  , const clang_utils::MatchResult& matchResult
  , clang::Rewriter& rewriter
  , const clang::Decl* nodeDecl)
{
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  TRACE_EVENT0("toplevel",
               "plugin::ExternalPlugin::callFuncBySignature");

#if defined(CLING_IS_ON)
  DCHECK(clingInterpreter_);
#endif // CLING_IS_ON

  DCHECK(nodeDecl);

  VLOG(9)
    << "<%= projectName %>_custom_annotation called...";

  clang::SourceManager &SM
    = rewriter.getSourceMgr();

  const clang::LangOptions& langOptions
    = rewriter.getLangOpts();

  /// VarDecl - An instance of this class is created to represent a variable
  /// declaration or definition.
  const clang::VarDecl* nodeVarDecl =
    matchResult.Nodes.getNodeAs<
      clang::VarDecl>("bind_gen");

  if(!nodeVarDecl) {
    /// CXXRecordDecl Represents a C++ struct/union/class.
    clang::CXXRecordDecl const *record =
        matchResult.Nodes
        .getNodeAs<clang::CXXRecordDecl>("bind_gen");
    LOG(WARNING)
      << "annotation must be"
         " placed near to clang::VarDecl "
      << (record ? record->getNameAsString() : "");
    return;
  }

  const std::string nodeName = nodeVarDecl->getNameAsString();

  DCHECK(!nodeName.empty());

  VLOG(9)
    << "nodeVarDecl name: "
    << nodeName;

  VLOG(9)
    << "nodeVarDecl processedAnnotation: "
    << processedAnnotation;

  clang::SourceLocation nodeStartLoc
    = nodeVarDecl->getLocStart();
  // Note Stmt::getLocEnd() returns the source location prior to the
  // token at the end of the line.  For instance, for:
  // var = 123;
  //      ^---- getLocEnd() points here.
  clang::SourceLocation nodeEndLoc
    = nodeVarDecl->getLocEnd();
  DCHECK(nodeStartLoc != nodeEndLoc);

  base::string16 contentsUTF16
    = base::UTF8ToUTF16(processedAnnotation);

  base::StringPiece16 clean_contents = contentsUTF16;

  VLOG(9)
    << "nodeVarDecl clean_contents: "
    << clean_contents;

  if(clean_contents.empty()) {
    DCHECK(nodeStartLoc.isValid());
    LOG(ERROR)
      << "variable declaration with"
         " annotation"
         " must be valid: "
      << nodeStartLoc.printToString(SM);
  }

  insertCodeAfterPos(
    processedAnnotation
    , annotateAttr
    , matchResult
    , rewriter
    , nodeDecl
    , nodeStartLoc
    , nodeEndLoc
    , base::UTF16ToUTF8(clean_contents)
  );
}

} // namespace plugin
