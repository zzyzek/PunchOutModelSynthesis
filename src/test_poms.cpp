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
#include <unistd.h>

#include "poms.hpp"

#include "tests/road2d_json.h"

extern char road2d[];

void show_test_types(FILE *fp) {
  fprintf(fp, "poms test types:\n");
  fprintf(fp, "\n");
  fprintf(fp, "  help (this screen)\n");
  fprintf(fp, "  road2d\n");
  fprintf(fp, "  distance_modifier\n");
  fprintf(fp, "\n");
}


int _test_poms_road2d() {
  int r;
  POMS poms;
  std::string s;

  int64_t cell;

  s = road2d;

  //poms.m_verbose = POMS_VERBOSE_DEBUG;
  r = poms.loadJSONString(s);
  printf("got: %i\n", r);

  printf("size: [%i,%i,%i]\n",
      (int)poms.m_size[0],
      (int)poms.m_size[1],
      (int)poms.m_size[2]);


  for (cell=0; cell<poms.m_cell_count; cell++) {
    poms.m_entropy[cell] = poms.rnd();
  }
  poms.entropyFilter(0.5, 1);

  poms.printDebugCellEntropy();
  poms.printDebugCellFilter();

  poms.assignRegionIDs();

  return 0;
}

int _test_poms_distance_modifier() {
  int r;
  POMS poms;
  std::string s;

  int x,y,z=0;
  int64_t cell;

  int32_t b[3][2];

  int seed_initial = 1;

  s = road2d;

  //poms.m_verbose = POMS_VERBOSE_DEBUG;
  r = poms.loadJSONString(s);
  printf("got: %i\n", r);

  poms.m_block_size[0] = 1;
  poms.m_block_size[1] = 1;
  poms.m_block_size[2] = 1;

  poms.m_soften_size[0] = 4;
  poms.m_soften_size[1] = 4;
  poms.m_soften_size[2] = 1;

  printf("size: [%i,%i,%i]\n",
      (int)poms.m_size[0],
      (int)poms.m_size[1],
      (int)poms.m_size[2]);

  //---

  if (seed_initial) {
    x = 0; y = 0; z = 0;
    cell = poms.xyz2cell(x,y,z);
    poms.setCellSize( 0, cell, 1 );
    poms.setCellTile( 0, cell, 0, 1 );

    x = 1; y = 0; z = 0;
    cell = poms.xyz2cell(x,y,z);
    poms.setCellSize( 0, cell, 1 );
    poms.setCellTile( 0, cell, 0, 1 );

    x = 2; y = 0; z = 0;
    cell = poms.xyz2cell(x,y,z);
    poms.setCellSize( 0, cell, 1 );
    poms.setCellTile( 0, cell, 0, 1 );

    x = 2; y = 1; z = 0;
    cell = poms.xyz2cell(x,y,z);
    poms.setCellSize( 0, cell, 1 );
    poms.setCellTile( 0, cell, 0, 1 );

    //--

    x = (poms.m_size[0]/2) + 3;
    y = (poms.m_size[1]/2) - 2;
    z = 0;
    cell = poms.xyz2cell(x,y,z);
    poms.setCellSize( 0, cell, 1 );
    poms.setCellTile( 0, cell, 0, 1 );

    x = (poms.m_size[0]/2) + 4;
    y = (poms.m_size[1]/2) - 2;
    z = 0;
    cell = poms.xyz2cell(x,y,z);
    poms.setCellSize( 0, cell, 1 );
    poms.setCellTile( 0, cell, 0, 1 );

    //--

    x = 2;
    y = (poms.m_size[1]/2) - 3;
    z = 0;
    cell = poms.xyz2cell(x,y,z);
    poms.setCellSize( 0, cell, 1 );
    poms.setCellTile( 0, cell, 0, 1 );
  }

  //---

  r = poms.computeDistanceModifier();
  printf("computeDistanceModifier: got:%i\n", r);
  poms.printDebugDistanceModifier();

  poms.m_block_choice_policy = POMS_BLOCK_CHOICE_WAVEFRONT;
  poms.chooseBlock( b, 0 );
  printf(">>> [%i:%i,%i:%i,%i:%i]\n",
      (int)b[0][0], (int)b[0][1],
      (int)b[1][0], (int)b[1][1],
      (int)b[2][0], (int)b[2][1]);

  //poms.printDebug();

  return 0;
}

int main(int argc, char **argv) {
  int r;
  std::string s;

  if (argc < 2) {
    fprintf(stderr, "provide test type\n\n");
    show_test_types(stderr);
    exit(-1);
  }

  s = argv[1];

  if (s == "help") {
    printf("\n");
    show_test_types(stdout);
  }
  else if (s == "road2d") {
    r = _test_poms_road2d();
  }
  else if (s == "distance_modifier") {
    r = _test_poms_distance_modifier();
  }

  if (r<0) {
    printf("ERROR\n");
  }

  if (r!=0) { exit(r); }
  exit(0);
}
