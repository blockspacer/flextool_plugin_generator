# About

Plugin <%= projectName %> creates custom annotation for [flextool](https://github.com/blockspacer/flextool) `_<%= projectName %>_custom_annotation(...)` that can be used like so:

```cpp
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
```

By default, plugin appends passed code after annotated variable.

You can change plugin logic in `src/Tooling.cc`, see `<%= projectName %>_custom_annotation` method.

## Global installation

Make sure that you installed [flextool](https://github.com/blockspacer/flextool) with common plugins (you can use `buildConanThirdparty.cmake` script).

```bash
export CXX=clang++-10
export CC=clang-10

# NOTE: change `build_type=Debug` to `build_type=Release` in production
# NOTE: use --build=missing if you got error `ERROR: Missing prebuilt package`
CONAN_REVISIONS_ENABLED=1 \
CONAN_VERBOSE_TRACEBACK=1 \
CONAN_PRINT_RUN_COMMANDS=1 \
CONAN_LOGGING_LEVEL=10 \
GIT_SSL_NO_VERIFY=true \
    cmake -E time \
      conan create . conan/stable \
      -s build_type=Debug -s cling_conan:build_type=Release \
      -o <%= projectName %>:shared=True \
      --profile clang \
          -e <%= projectName %>:enable_tests=True
```

## Local development

Make sure that you installed [flextool](https://github.com/blockspacer/flextool) with common plugins (you can use `buildConanThirdparty.cmake` script).

Commands below may be used to build project locally, without system-wide installation.

```bash
export CXX=clang++-10
export CC=clang-10

cmake -E remove_directory build

cmake -E make_directory build

# NOTE: change `build_type=Debug` to `build_type=Release` in production
build_type=Debug

# install conan requirements
CONAN_REVISIONS_ENABLED=1 \
    CONAN_VERBOSE_TRACEBACK=1 \
    CONAN_PRINT_RUN_COMMANDS=1 \
    CONAN_LOGGING_LEVEL=10 \
    GIT_SSL_NO_VERIFY=true \
        cmake -E chdir build cmake -E time \
            conan install \
            -s build_type=${build_type} -s cling_conan:build_type=Release \
            -o <%= projectName %>:shared=True \
            --build=missing \
            --profile clang \
                -e enable_tests=True \
                ..

# optional: remove generated files (change paths to yours)
rm build/*generated*
rm build/generated/ -rf
rm build/bin/${build_type}/ -rf

# configure via cmake
cmake -E chdir build \
  cmake -E time cmake .. \
  -DBUILD_SHARED_LIBS=ON \
  -DENABLE_TESTS=FALSE \
  -DCONAN_AUTO_INSTALL=OFF \
  -DCMAKE_BUILD_TYPE=${build_type}

# build code
cmake -E chdir build \
  cmake -E time cmake --build . \
  --config ${build_type} \
  -- -j8
```

## For contibutors: cppcheck

Make sure you use `Debug` build with `-e <%= projectName %>:enable_llvm_tools=True`.

Install cppcheck via conan:

```bash
cd /tmp

git clone -b testing/1.90 https://github.com/bincrafters/conan-cppcheck_installer.git

cd conan-cppcheck_installer

# NOTE: change `build_type=Debug` to `build_type=Release` in production
# NOTE: use --build=missing if you got error `ERROR: Missing prebuilt package`
CONAN_REVISIONS_ENABLED=1 \
CONAN_VERBOSE_TRACEBACK=1 \
CONAN_PRINT_RUN_COMMANDS=1 \
CONAN_LOGGING_LEVEL=10 \
GIT_SSL_NO_VERIFY=true \
    cmake -E time \
      conan create . conan/stable \
      -s build_type=Release
```

Usage (runs cmake with `-DENABLE_CPPCHECK=ON`):

```bash
# creates local build in separate folder and runs cmake targets
# NOTE: -DCLEAN_OLD="OFF" to keep generated html report
cmake -DCPPCHECK="ON" -DCLEAN_OLD="OFF" -P tools/run_tool.cmake
```

```bash
# `index.html` must exist
# find $PWD -name index.html
```

Open 'index.html' to see the results.

## For contibutors: IWYU

Make sure you use `Debug` build with `-e <%= projectName %>:enable_llvm_tools=True`

include-what-you-use (IWYU) is a project intended to optimise includes.

It will calculate the required headers and add / remove includes as appropriate.

For details, see: [https://include-what-you-use.org/](https://include-what-you-use.org/)

Usage (runs cmake with `-DENABLE_IWYU=ON`):

```bash
export CXX=clang++-10
export CC=clang-10

# creates local build in separate folder and runs cmake targets
cmake -DIWYU="ON" -DCLEAN_OLD="ON" -P tools/run_tool.cmake
```

CODESTYLE: use `// IWYU pragma: associated` in C++ source files.

NOTE: Read about IWYU Pragmas: [https://github.com/include-what-you-use/include-what-you-use/blob/master/docs/IWYUPragmas.md](https://github.com/include-what-you-use/include-what-you-use/blob/master/docs/IWYUPragmas.md)

NOTE: don't use "bits/" or "/details/*" includes, add them to mappings file (.imp)

For details, see:

* https://llvm.org/devmtg/2010-11/Silverstein-IncludeWhatYouUse.pdf
* https://github.com/include-what-you-use/include-what-you-use/tree/master/docs
* https://github.com/hdclark/Ygor/blob/master/artifacts/20180225_include-what-you-use/iwyu_how-to.txt

## For contibutors: uncrustify

We use uncrustify bacause clang-format and astyle [do not support a lot of options](https://dev.to/voins/does-anybody-know-a-good-working-c-formatting-tool-2lpi).

For details, see: [https://patrickhenson.com/2018/06/07/uncrustify-configuration.html](https://patrickhenson.com/2018/06/07/uncrustify-configuration.html)

Installation:

```bash
cd ~
git clone https://github.com/uncrustify/uncrustify.git
cd uncrustify
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
export UNCRUSTIFY_HOME=~/uncrustify/build
export PATH=$UNCRUSTIFY_HOME:$PATH
# OR sudo make install

uncrustify --version
```

Usage (runs cmake with `-DENABLE_UNCRUSTIFY=ON`):

```bash
# creates local build in separate folder and runs cmake targets
cmake -DUNCRUSTIFY="ON" -DCLEAN_OLD="ON" -P tools/run_tool.cmake
```

We use `uncrustify.cfg` file. For details, see: [https://patrickhenson.com/2018/06/07/uncrustify-configuration.html](https://patrickhenson.com/2018/06/07/uncrustify-configuration.html)

To get a list of all available options use:

```bash
uncrustify --show-config
```

Uncrustify has a lot of configurable options. You'll probably need Universal Indent GUI (in Konstantin's reply) as well to configure it: http://universalindent.sourceforge.net/

Use comments containing `/* *INDENT-OFF* */` and `/* *INDENT-ON* */` to disable processing of parts of the source file.

See `disable_processing_cmt` from `uncrustify.cfg`:

```ini
# Specify the marker used in comments to disable processing of part of the
# file.
# The comment should be used alone in one line.
#
# Default:  *INDENT-OFF*
disable_processing_cmt          = " *INDENT-OFF*"      # string

# Specify the marker used in comments to (re)enable processing in a file.
# The comment should be used alone in one line.
#
# Default:  *INDENT-ON*
enable_processing_cmt           = " *INDENT-ON*"     # string
```

You can integrate `uncrustify` with IDE:

* QT Creator: [https://doc.qt.io/qtcreator/creator-beautifier.html](https://doc.qt.io/qtcreator/creator-beautifier.html)
* Visual Studio Code: [https://marketplace.visualstudio.com/items?itemName=LaurentTreguier.uncrustify](https://marketplace.visualstudio.com/items?itemName=LaurentTreguier.uncrustify)
