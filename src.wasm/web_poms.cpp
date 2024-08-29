/*
 * To the extent possible under law, the person who associated CC0 with
 * this file has waived all copyright and related or neighboring rights
 * to this file.
 *
 * You should have received a copy of the CC0 legalcode along with this
 * work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>

#include <string>
#include <vector>

#include <emscripten.h>

#include "main_poms.hpp"

#define PRELOAD_POMS_FN "pillMortal_poms.json"

// Need a compiler defined of CUSTOM_MAIN (-D CUSTOM_MAIN)
// so that the `main` isn't used in `main_poms.cpp`  and
// main is used here.
//

// c function style to not have cpp mangle the linker
// names. emscriptn complains about not finding functions
// or the function names get mangled.
//
// Two options to compile:
//
//   with `-s` option:
//
//     -s EXPORTED_FUNCTIONS="['_custom_function', '_main']" \
//
//     Module._custom_function() and Module._main() can be called.
//
//
//   without `-s` option:
//
//     Module._custom_function_1() and Module._main() can be called
//
//
extern "C" {

// call with Module._custom_function()
//
int custom_function(void) {
  printf("custom function\n");
  return 0;
}

EMSCRIPTEN_KEEPALIVE
int mock_main(int argc, char **argv) {
  int i;

  printf("mock_main argc:%i\n", argc);
  for (i=0; i<argc; i++) {
    printf("  [%i] '%s'\n", i, argv[i]);
  }

  return 0;
}

EMSCRIPTEN_KEEPALIVE
int file_write_test(void) {
  static int _state = 0;
  std::string fn = "out.txt";
  FILE *fp;


  fp = fopen( fn.c_str(), "w" );
  if (!fp) { return -1; }
  fprintf(fp, "hello, friend (%i)\n", _state);
  fclose(fp);


  printf("out.txt");

  _state++;

  return 0;
}


EMSCRIPTEN_KEEPALIVE
int custom_function_1(void) {
  std::string x;
  int ch;

  FILE *fp;

  fp = fopen(PRELOAD_POMS_FN, "r");
  if (!fp) { printf("file load error\n"); return -1; }

  while (!feof(fp)) {
    ch = fgetc(fp);
    if (ch == EOF) { continue; }
    x += ch;
  }

  printf("custom function 1:\n%s\n", x.c_str());
  return 0;
}

}


// Here is the main entry point.
// The program will be called but with no arguments,
// so the check eblow will bail without issue.
//
// Subsequent calls can pass in command line options
// and run the program as normal.
//
// see html/js/poms_helper.js
//
int main(int argc, char **argv) {
  int i;

  if (argc <= 1) { return 0; }

  // getopt keeps state and so gets donked up when called multiple
  // times. Lets see if he can't hack our way to a soution...
  //
  optind = 1;
  opterr = 1;
  optopt = 0;
  optarg = NULL;

  return poms_main(argc, argv);
}
