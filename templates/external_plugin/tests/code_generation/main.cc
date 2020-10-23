#include <string>

#define _<%= projectName %>_custom_annotation(...) \
  __attribute__((annotate("{gen};{<%= projectName %>_custom_annotation};CXTPL;" #__VA_ARGS__ )))

static void somefunc()
{
  {
    // generate code and append it after annotated variable
    _<%= projectName %>_custom_annotation(
      int a;\n
      #define A 1
      int c = "123";\n
    )
    std::string out = "";
  }
}
