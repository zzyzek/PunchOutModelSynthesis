#include "ac4_tier.hpp"


int test0(void) {
  int64_t cell, cell_count;
  int32_t tile, tile_count;
  int32_t idir;
  int32_t m, v;
  ac4_flat_t ac4_flat;
  ac4_tier4_t ac4_tier4;

  int32_t _t4[4] = {1,   8,    16,    64};
  int32_t _s4[4] = {2, 256, 65536, 65536};
  int32_t _range4[4] = { 2, 256, 1<<15, 1<<15 };

  int64_t sz;
  double sz_f, Gb;

  int _verbose = 0;

  cell_count = 8*8*4;
  tile_count = 12000;

  //cell_count = 2*3*5;
  //tile_count = 7;

  ac4_tier4.pre_init(cell_count, tile_count);
  for (tile=0; tile<tile_count; tile++) {

    //ac4_tier.init_tile_vec(tile, _t[rand()%4]);

    /*
    if (tile < (7*tile_count/8))        { ac4_tier.init_tile_vec(tile, _t[0]); }
    else if (tile < (15*tile_count/16)) { ac4_tier.init_tile_vec(tile, _t[1]); }
    else                                { ac4_tier.init_tile_vec(tile, _t[2]); }
    */

    //if (tile < (tile_count/4)) { ac4_tier4.init_tile_vec(tile, _t4[0]); }
    //else { ac4_tier4.init_tile_vec(tile, _t4[1]); }

    if      (tile < (tile_count/4))   { ac4_tier4.init_tile_vec(tile, _t4[0]); }
    else if (tile < (3*tile_count/4)) { ac4_tier4.init_tile_vec(tile, _t4[1]); }
    else if (tile < (7*tile_count/8)) { ac4_tier4.init_tile_vec(tile, _t4[2]); }
    else { ac4_tier4.init_tile_vec(tile, _t4[3]); } 



    //if (tile < (98*tile_count/100))          { ac4_tier.init_tile_vec(tile, _t[0]); }
    //else                                { ac4_tier.init_tile_vec(tile, _t[1]); }

  }
  ac4_flat.init(cell_count, tile_count);

  // setup
  //
  for (idir=0; idir<6; idir++) {
    for (tile=0; tile<tile_count; tile++) {
      //m = _s4[ ac4_tier4.m_tile_tier[tile] ];
      m = _range4[ ac4_tier4.m_tile_tier[tile] ];

      //printf("idir:%i, tile:%i, ac4_tier.m_tile_tier[%i]:%i\n",
      //    (int)idir, (int)tile, (int)tile, (int)ac4_tier.m_tile_tier[tile]);

      for (cell=0; cell<cell_count; cell++) {

        //printf("m:%i\n", (int)m);
        v = rand()%m;

        ac4_tier4.tileSupport(idir, cell, tile, v);
        ac4_flat.tileSupport(idir, cell, tile, v);

      }
    }
  }

  if (_verbose) {
    Gb = 1024.0*1024.0*1024.0;

    sz = ac4_tier4.size_estimate();
    sz_f = (double)sz / Gb;

    printf("\n----\n");
    printf("ac4_tier4: (%i,%f)\n", (int)sz, sz_f);
    ac4_tier4.print();
    printf("\n");

    sz = ac4_flat.size_estimate();
    sz_f = (double)sz / Gb;

    printf("\n----\n");
    printf("ac4_flat: (%i,%f)\n", (int)sz, sz_f);
    ac4_flat.print();
    printf("\n");

  }

  // check
  //
  for (idir=0; idir<6; idir++) {
    for (tile=0; tile<tile_count; tile++) {
      for (cell=0; cell<cell_count; cell++) {
        if (ac4_tier4.tileSupport(idir,cell,tile) != ac4_flat.tileSupport(idir,cell,tile)) {

          printf("mismatch idir:%i, tile:%i, cell:%i (%i!=%i)\n",
              idir, tile, (int)cell,
              (int)ac4_tier4.tileSupport(idir, cell, tile), (int)ac4_flat.tileSupport(idir,cell,tile)
              );

          printf("  tile:%i, tier:%i, bitsize:%i\n",
              (int)tile, ac4_tier4.m_tile_tier[tile], _t4[ ac4_tier4.m_tile_tier[tile] ]);

          return -1;
        }
      }
    }
  }

  Gb = 1024.0*1024.0*1024.0;

  sz = ac4_tier4.size_estimate();
  sz_f = (double)sz / Gb;
  printf("ac4_tier4: (%i,%f)\n", (int)sz, sz_f);

  sz = ac4_flat.size_estimate();
  sz_f = (double)sz / Gb;
  printf("ac4_flat: (%i,%f)\n", (int)sz, sz_f);

  return 0;
}

//---

int test0m1(void) {
  int64_t cell, cell_count;
  int32_t tile, tile_count;
  int32_t idir;
  int32_t m, v;
  ac4_flat_t ac4_flat;
  ac4_tier4_m1_t ac4_tier4;

  int32_t _t4[4] = {1,   8,    16,    64};
  int32_t _s4[4] = {2, 256, 65536, 65536};
  int32_t _range4[4] = { 2, 256, 1<<15, 1<<15 };

  int64_t sz;
  double sz_f, Gb;

  int _verbose = 0;

  cell_count = 8*8*4;
  tile_count = 12000;

  //cell_count = 2*3*5;
  //tile_count = 7;

  ac4_tier4.clear();
  ac4_tier4.pre_init(cell_count, tile_count);
  for (tile=0; tile<tile_count; tile++) {
    if      (tile < (tile_count/4))   { ac4_tier4.init_tile_vec(tile, _t4[0]); }
    else if (tile < (3*tile_count/4)) { ac4_tier4.init_tile_vec(tile, _t4[1]); }
    else if (tile < (7*tile_count/8)) { ac4_tier4.init_tile_vec(tile, _t4[2]); }
    else { ac4_tier4.init_tile_vec(tile, _t4[3]); } 
  }

  ac4_tier4.clear();
  ac4_tier4.pre_init(cell_count, tile_count);
  for (tile=0; tile<tile_count; tile++) {
    if      (tile < (tile_count/4))   { ac4_tier4.init_tile_vec(tile, _t4[0]); }
    else if (tile < (3*tile_count/4)) { ac4_tier4.init_tile_vec(tile, _t4[1]); }
    else if (tile < (7*tile_count/8)) { ac4_tier4.init_tile_vec(tile, _t4[2]); }
    else { ac4_tier4.init_tile_vec(tile, _t4[3]); } 
  }


  ac4_flat.init(cell_count, tile_count);

  // setup
  //
  for (idir=0; idir<6; idir++) {
    for (tile=0; tile<tile_count; tile++) {
      //m = _s4[ ac4_tier4.m_tile_tier_offset[2*tile] ];
      m = _range4[ ac4_tier4.m_tile_tier_offset[2*tile] ];

      //printf("idir:%i, tile:%i, ac4_tier.m_tile_tier[%i]:%i\n",
      //    (int)idir, (int)tile, (int)tile, (int)ac4_tier.m_tile_tier[tile]);

      for (cell=0; cell<cell_count; cell++) {

        //printf("m:%i\n", (int)m);
        v = rand()%m;

        ac4_tier4.tileSupport(idir, cell, tile, v);
        ac4_flat.tileSupport(idir, cell, tile, v);

      }
    }
  }

  if (_verbose) {
    Gb = 1024.0*1024.0*1024.0;

    sz = ac4_tier4.size_estimate();
    sz_f = (double)sz / Gb;

    printf("\n----\n");
    printf("ac4_tier4_m1: (%i,%f)\n", (int)sz, sz_f);
    ac4_tier4.print();
    printf("\n");

    sz = ac4_flat.size_estimate();
    sz_f = (double)sz / Gb;

    printf("\n----\n");
    printf("ac4_flat: (%i,%f)\n", (int)sz, sz_f);
    ac4_flat.print();
    printf("\n");

  }

  // check
  //
  for (idir=0; idir<6; idir++) {
    for (tile=0; tile<tile_count; tile++) {
      for (cell=0; cell<cell_count; cell++) {
        if (ac4_tier4.tileSupport(idir,cell,tile) != ac4_flat.tileSupport(idir,cell,tile)) {

          printf("mismatch idir:%i, tile:%i, cell:%i (%i!=%i)\n",
              idir, tile, (int)cell,
              (int)ac4_tier4.tileSupport(idir,cell,tile),
              (int)ac4_flat.tileSupport(idir,cell,tile)
              );
          return -1;
        }
      }
    }
  }

  Gb = 1024.0*1024.0*1024.0;

  sz = ac4_tier4.size_estimate();
  sz_f = (double)sz / Gb;
  printf("ac4_tier4_m1: (%i,%f)\n", (int)sz, sz_f);

  sz = ac4_flat.size_estimate();
  sz_f = (double)sz / Gb;
  printf("ac4_flat: (%i,%f)\n", (int)sz, sz_f);

  return 0;
}

//---

int test0m2(void) {
  int64_t cell, cell_count;
  int32_t tile, tile_count;
  int32_t idir;
  int32_t m, v;
  ac4_flat_t ac4_flat;
  ac4_tier4_m2_t ac4_tier4;

  int32_t _t4[4] = {1,   8,    16,    64};
  int32_t _s4[4] = {2, 256, 65536, 65536};
  int32_t _range4[4] = { 2, 256, 1<<15, 1<<15 };

  int64_t sz;
  double sz_f, Gb;

  int _verbose = 0;

  cell_count = 8*8*4;
  tile_count = 12000;

  //cell_count = 2*3*5;
  //tile_count = 7;

  ac4_tier4.clear();
  ac4_tier4.pre_init(cell_count, tile_count);
  for (tile=0; tile<tile_count; tile++) {
    if      (tile < (tile_count/4))   { ac4_tier4.init_tile_vec(tile, _t4[0]); }
    else if (tile < (3*tile_count/4)) { ac4_tier4.init_tile_vec(tile, _t4[1]); }
    else if (tile < (7*tile_count/8)) { ac4_tier4.init_tile_vec(tile, _t4[2]); }
    else { ac4_tier4.init_tile_vec(tile, _t4[3]); } 
  }

  ac4_tier4.clear();
  ac4_tier4.pre_init(cell_count, tile_count);
  for (tile=0; tile<tile_count; tile++) {
    if      (tile < (tile_count/4))   { ac4_tier4.init_tile_vec(tile, _t4[0]); }
    else if (tile < (3*tile_count/4)) { ac4_tier4.init_tile_vec(tile, _t4[1]); }
    else if (tile < (7*tile_count/8)) { ac4_tier4.init_tile_vec(tile, _t4[2]); }
    else { ac4_tier4.init_tile_vec(tile, _t4[3]); } 
  }


  ac4_flat.init(cell_count, tile_count);

  // setup
  //
  for (idir=0; idir<6; idir++) {
    for (tile=0; tile<tile_count; tile++) {
      //m = _s4[ ac4_tier4.m_tile_tier_offset[2*tile] ];
      m = _range4[ ac4_tier4.m_tile_tier_offset[2*tile] ];

      //printf("idir:%i, tile:%i, ac4_tier.m_tile_tier[%i]:%i\n",
      //    (int)idir, (int)tile, (int)tile, (int)ac4_tier.m_tile_tier[tile]);

      for (cell=0; cell<cell_count; cell++) {

        //printf("m:%i\n", (int)m);
        v = rand()%m;

        ac4_tier4.tileSupport(idir, cell, tile, v);
        ac4_flat.tileSupport(idir, cell, tile, v);

      }
    }
  }

  if (_verbose) {
    Gb = 1024.0*1024.0*1024.0;

    sz = ac4_tier4.size_estimate();
    sz_f = (double)sz / Gb;

    printf("\n----\n");
    printf("ac4_tier4_m1: (%i,%f)\n", (int)sz, sz_f);
    ac4_tier4.print();
    printf("\n");

    sz = ac4_flat.size_estimate();
    sz_f = (double)sz / Gb;

    printf("\n----\n");
    printf("ac4_flat: (%i,%f)\n", (int)sz, sz_f);
    ac4_flat.print();
    printf("\n");

  }

  // check
  //
  for (idir=0; idir<6; idir++) {
    for (tile=0; tile<tile_count; tile++) {
      for (cell=0; cell<cell_count; cell++) {
        if (ac4_tier4.tileSupport(idir,cell,tile) != ac4_flat.tileSupport(idir,cell,tile)) {

          printf("mismatch idir:%i, tile:%i, cell:%i (%i!=%i)\n",
              idir, tile, (int)cell,
              (int)ac4_tier4.tileSupport(idir,cell,tile),
              (int)ac4_flat.tileSupport(idir,cell,tile)
              );
          return -1;
        }
      }
    }
  }

  Gb = 1024.0*1024.0*1024.0;

  sz = ac4_tier4.size_estimate();
  sz_f = (double)sz / Gb;
  printf("ac4_tier4_m1: (%i,%f)\n", (int)sz, sz_f);

  sz = ac4_flat.size_estimate();
  sz_f = (double)sz / Gb;
  printf("ac4_flat: (%i,%f)\n", (int)sz, sz_f);

  return 0;
}


//----
//----
//----

int test1(void) {
  int64_t cell, cell_count;
  int32_t tile, tile_count;
  int32_t idir;
  int32_t m, v;
  ac4_flat_t ac4_flat;
  ac4_tier6_t ac4_tier6;

  int32_t _t6[6] = {1, 2,  4,   8,    16,    64};
  int32_t _s6[6] = {2, 4, 16, 256, 1<<15, 1<<15};

  int64_t sz;
  double sz_f, Gb;

  int _verbose = 0;

  cell_count = 8*8*4;
  tile_count = 12000;


  //cell_count = 2*3*5;
  //tile_count = 7;

  ac4_tier6.clear();
  ac4_tier6.pre_init(cell_count, tile_count);
  for (tile=0; tile<tile_count; tile++) {

    if      (tile < (2990*tile_count/11979)) { ac4_tier6.init_tile_vec(tile, _t6[0]); }
    else if (tile < (5800*tile_count/11979)) { ac4_tier6.init_tile_vec(tile, _t6[1]); }
    else if (tile < (9820*tile_count/11979)) { ac4_tier6.init_tile_vec(tile, _t6[2]); }
    else if (tile < (11910*tile_count/11979)) { ac4_tier6.init_tile_vec(tile, _t6[3]); }
    else if (tile < (11950*tile_count/11979)) { ac4_tier6.init_tile_vec(tile, _t6[4]); }
    else { ac4_tier6.init_tile_vec(tile, _t6[5]); }

  }
  ac4_flat.init(cell_count, tile_count);

  /*
  ac4_tier6.pre_init(cell_count, tile_count);
  for (tile=0; tile<tile_count; tile++) {
    if (tile < (2995*tile_count/11979)) {
      ac4_tier6.init_tile_vec(tile, _t6[0]);
    }
    if (tile < (5801*tile_count/11979)) {
      ac4_tier6.init_tile_vec(tile, _t6[1]);
    }
    if (tile < (9827*tile_count/11979)) {
      ac4_tier6.init_tile_vec(tile, _t6[2]);
    }
    else {
      ac4_tier6.init_tile_vec(tile, _t6[3]);
    }

  }
  ac4_flat.init(cell_count, tile_count);
  */

  // setup
  //
  for (idir=0; idir<6; idir++) {
    for (tile=0; tile<tile_count; tile++) {
      m = _s6[ ac4_tier6.m_tile_tier[tile] ];

      //printf("idir:%i, tile:%i, ac4_tier.m_tile_tier[%i]:%i\n",
      //    (int)idir, (int)tile, (int)tile, (int)ac4_tier.m_tile_tier[tile]);

      for (cell=0; cell<cell_count; cell++) {

        //printf("m:%i\n", (int)m);
        v = rand()%m;

        ac4_tier6.tileSupport(idir, cell, tile, v);
        ac4_flat.tileSupport(idir, cell, tile, v);

      }
    }
  }

  if (_verbose) {
    Gb = 1024.0*1024.0*1024.0;

    sz = ac4_tier6.size_estimate();
    sz_f = (double)sz / Gb;

    printf("\n----\n");
    printf("ac4_tier6: (%i,%f)\n", (int)sz, sz_f);
    ac4_tier6.print();
    printf("\n");

    sz = ac4_flat.size_estimate();
    sz_f = (double)sz / Gb;

    printf("\n----\n");
    printf("ac4_flat: (%i,%f)\n", (int)sz, sz_f);
    ac4_flat.print();
    printf("\n");

  }

  // check
  //
  for (idir=0; idir<6; idir++) {
    for (tile=0; tile<tile_count; tile++) {
      for (cell=0; cell<cell_count; cell++) {
        if (ac4_tier6.tileSupport(idir,cell,tile) != ac4_flat.tileSupport(idir,cell,tile)) {

          printf("mismatch idir:%i, tile:%i, cell:%i\n", idir, tile, (int)cell);
          return -1;
        }
      }
    }
  }

  Gb = 1024.0*1024.0*1024.0;

  sz = ac4_tier6.size_estimate();
  sz_f = (double)sz / Gb;
  printf("ac4_tier6: (%i,%f)\n", (int)sz, sz_f);

  sz = ac4_flat.size_estimate();
  sz_f = (double)sz / Gb;
  printf("ac4_flat: (%i,%f)\n", (int)sz, sz_f);

  return 0;
}

int test1m(void) {
  int64_t cell, cell_count;
  int32_t tile, tile_count;
  int32_t idir;
  int32_t m, v;
  ac4_flat_t ac4_flat;
  ac4_tier6_m1_t ac4_tier6;

  int32_t _t6[6]      = {1, 2,  4,   8,    16,    64};
  int32_t _s6[6]      = {2, 4, 16, 256, 1<<15, 1<<15};
  int32_t _range6[6]  = {2, 4, 16, 256, 1<<15, 1<<15};

  int64_t sz;
  double sz_f, Gb;

  int _verbose = 0;

  cell_count = 8*8*4;
  tile_count = 12000;

  ac4_tier6.pre_init(cell_count, tile_count);
  for (tile=0; tile<tile_count; tile++) {

    if (tile < (2995*tile_count/11979)) { ac4_tier6.init_tile_vec(tile, _t6[0]); }
    else if (tile < (5801*tile_count/11979)) { ac4_tier6.init_tile_vec(tile, _t6[1]); }
    else if (tile < (9827*tile_count/11979)) { ac4_tier6.init_tile_vec(tile, _t6[2]); }
    else if (tile < (11900*tile_count/11979)) { ac4_tier6.init_tile_vec(tile, _t6[3]); }
    else if (tile < (11950*tile_count/11979)) { ac4_tier6.init_tile_vec(tile, _t6[4]); }
    else { ac4_tier6.init_tile_vec(tile, _t6[5]); }

  }

  ac4_tier6.clear();
  ac4_tier6.pre_init(cell_count, tile_count);
  for (tile=0; tile<tile_count; tile++) {

    if      (tile < (2990*tile_count/11979)) { ac4_tier6.init_tile_vec(tile, _t6[0]); }
    else if (tile < (5800*tile_count/11979)) { ac4_tier6.init_tile_vec(tile, _t6[1]); }
    else if (tile < (9820*tile_count/11979)) { ac4_tier6.init_tile_vec(tile, _t6[2]); }
    else if (tile < (11910*tile_count/11979)) { ac4_tier6.init_tile_vec(tile, _t6[3]); }
    else if (tile < (11950*tile_count/11979)) { ac4_tier6.init_tile_vec(tile, _t6[4]); }
    else { ac4_tier6.init_tile_vec(tile, _t6[5]); }

  }


  ac4_flat.init(cell_count, tile_count);


  // setup
  //
  for (idir=0; idir<6; idir++) {
    for (tile=0; tile<tile_count; tile++) {
      m = _range6[ ac4_tier6.m_tile_tier_offset[2*tile] ];

      //printf("idir:%i, tile:%i, ac4_tier.m_tile_tier[%i]:%i\n",
      //    (int)idir, (int)tile, (int)tile, (int)ac4_tier.m_tile_tier[tile]);

      for (cell=0; cell<cell_count; cell++) {

        //printf("m:%i\n", (int)m);
        v = rand()%m;

        ac4_tier6.tileSupport(idir, cell, tile, v);
        ac4_flat.tileSupport(idir, cell, tile, v);

      }
    }
  }

  if (_verbose) {
    Gb = 1024.0*1024.0*1024.0;

    sz = ac4_tier6.size_estimate();
    sz_f = (double)sz / Gb;

    printf("\n----\n");
    printf("ac4_tier6_m1: (%i,%f)\n", (int)sz, sz_f);
    ac4_tier6.print();
    printf("\n");

    sz = ac4_flat.size_estimate();
    sz_f = (double)sz / Gb;

    printf("\n----\n");
    printf("ac4_flat: (%i,%f)\n", (int)sz, sz_f);
    ac4_flat.print();
    printf("\n");

  }

  // check
  //
  for (idir=0; idir<6; idir++) {
    for (tile=0; tile<tile_count; tile++) {
      for (cell=0; cell<cell_count; cell++) {
        if (ac4_tier6.tileSupport(idir,cell,tile) != ac4_flat.tileSupport(idir,cell,tile)) {

          printf("mismatch idir:%i, tile:%i, cell:%i (%i!=%i)\n", idir, tile, (int)cell,
              (int)ac4_tier6.tileSupport(idir,cell,tile), (int)ac4_flat.tileSupport(idir,cell,tile));
          return -1;
        }
      }
    }
  }

  Gb = 1024.0*1024.0*1024.0;

  sz = ac4_tier6.size_estimate();
  sz_f = (double)sz / Gb;
  printf("ac4_tier6_m1: (%i,%f)\n", (int)sz, sz_f);

  sz = ac4_flat.size_estimate();
  sz_f = (double)sz / Gb;
  printf("ac4_flat: (%i,%f)\n", (int)sz, sz_f);

  return 0;
}

int main(int argc, char **argv) {
  int r;


  r = test0();
  printf("# test0: %s\n", (r==0) ? "pass" : "FAIL");

  r = test0m1();
  printf("# test0m1: %s\n", (r==0) ? "pass" : "FAIL");

  r = test0m2();
  printf("# test0m2: %s\n", (r==0) ? "pass" : "FAIL");

  r = test1();
  printf("# test1: %s\n", (r==0) ? "pass" : "FAIL");

  r = test1m();
  printf("# test1m: %s\n", (r==0) ? "pass" : "FAIL");


}
