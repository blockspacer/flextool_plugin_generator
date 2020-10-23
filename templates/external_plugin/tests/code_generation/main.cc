#include <string>

#define _external_plugin_custom_annotation(...) \
  __attribute__((annotate("{gen};{external_plugin_custom_annotation};CXTPL;" #__VA_ARGS__ )))

static void somefunc()
{
  {
    // generate code and append it after annotated variable
    _external_plugin_custom_annotation(
      int a;\n
      #define A 1
      int c = "123";\n
    )
    std::string out = "";
  }
}
