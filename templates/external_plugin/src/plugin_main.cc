#include <<%= projectName %>/EventHandler.hpp>

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

#include <base/logging.h>
#include <base/cpu.h>
#include <base/bind.h>
#include <base/command_line.h>
#include <base/debug/alias.h>
#include <base/debug/stack_trace.h>
#include <base/memory/ptr_util.h>
#include <base/sequenced_task_runner.h>
#include <base/strings/string_util.h>
#include <base/trace_event/trace_event.h>

#if !defined(CORRADE_DYNAMIC_PLUGIN)
#error "plugin must be shared library with CORRADE_DYNAMIC_PLUGIN=1"
#endif  // CORRADE_DYNAMIC_PLUGIN

namespace plugin {

/// \note class name must not collide with
/// class names from other loaded plugins
class ExternalPlugin
  final
  : public ::plugin::ToolPlugin {
 public:
  explicit ExternalPlugin(
    ::plugin::AbstractManager& manager
    , const std::string& plugin)
    : ::plugin::ToolPlugin{manager, plugin}
  {
    DETACH_FROM_SEQUENCE(sequence_checker_);
  }

  std::string title() const override
  {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

    return metadata()->data().value("title");
  }

  std::string author() const override
  {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

    return metadata()->data().value("author");
  }

  std::string description() const override
  {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

    return metadata()->data().value("description");
  }

  bool load() override
  {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    TRACE_EVENT0("toplevel",
                 "plugin::ExternalPlugin::load()");

    DLOG(INFO)
      << "loaded plugin with title = "
      << title()
      << " and description = "
      << description().substr(0, 100)
      << "...";

    return true;
  }

  void disconnect_dispatcher(
    entt::dispatcher &event_dispatcher) override
  {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    TRACE_EVENT0("toplevel",
                 "plugin::ExternalPlugin::disconnect_dispatcher()");

    VLOG(9)
      << "plugin::ExternalPlugin::disconnect_dispatcher()";

    event_dispatcher.sink<
      ::plugin::ToolPlugin::Events::StringCommand>()
        .disconnect<
          &ExternalPluginEventHandler::StringCommand>(&eventHandler_);

    event_dispatcher.sink<
      ::plugin::ToolPlugin::Events::RegisterAnnotationMethods>()
        .disconnect<
          &ExternalPluginEventHandler::RegisterAnnotationMethods>(&eventHandler_);

#if defined(CLING_IS_ON)
    event_dispatcher.sink<
      ::plugin::ToolPlugin::Events::RegisterClingInterpreter>()
        .disconnect<
          &ExternalPluginEventHandler::RegisterClingInterpreter>(&eventHandler_);
#endif // CLING_IS_ON
  }

  void connect_to_dispatcher(
    entt::dispatcher &event_dispatcher) override
  {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    TRACE_EVENT0("toplevel",
                 "plugin::ExternalPlugin::connect_to_dispatcher()");

    VLOG(9)
      << "plugin::ExternalPlugin::connect_to_dispatcher()";

    event_dispatcher.sink<
      ::plugin::ToolPlugin::Events::StringCommand>()
        .connect<
          &ExternalPluginEventHandler::StringCommand>(&eventHandler_);

    event_dispatcher.sink<
      ::plugin::ToolPlugin::Events::RegisterAnnotationMethods>()
        .connect<
          &ExternalPluginEventHandler::RegisterAnnotationMethods>(&eventHandler_);

#if defined(CLING_IS_ON)
    event_dispatcher.sink<
      ::plugin::ToolPlugin::Events::RegisterClingInterpreter>()
        .connect<
          &ExternalPluginEventHandler::RegisterClingInterpreter>(&eventHandler_);
#endif // CLING_IS_ON
  }

  bool unload() override
  {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    TRACE_EVENT0("toplevel",
                 "plugin::ExternalPlugin::unload()");

    DLOG(INFO)
      << "unloaded plugin with title = "
      << title()
      << " and description = "
      << description().substr(0, 100)
      << "...";

    return true;
  }

private:
  ExternalPluginEventHandler eventHandler_{};

  DISALLOW_COPY_AND_ASSIGN(ExternalPlugin);
};

} // namespace plugin

REGISTER_PLUGIN(/*name*/ ExternalPlugin
    , /*className*/ plugin::ExternalPlugin
    // plugin interface version checks to avoid unexpected behavior
    , /*interface*/ "backend.ToolPlugin/1.0")
