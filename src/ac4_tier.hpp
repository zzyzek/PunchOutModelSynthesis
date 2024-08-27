/*
 * To the extent possible under law, the person who associated CC0 with
 * this file has waived all copyright and related or neighboring rights
 * to this file.
 *
 * You should have received a copy of the CC0 legalcode along with this
 * work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 */

#ifndef AC4_TIER_H
#define AC4_TIER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <vector>
#include <string>

//----
//     _    ____ _  _     _____ _        _  _____
//    / \  / ___| || |   |  ___| |      / \|_   _|
//   / _ \| |   | || |_  | |_  | |     / _ \ | |
//  / ___ \ |___|__   _| |  _| | |___ / ___ \| |
// /_/   \_\____|  |_|___|_|   |_____/_/   \_\_|
//                  |_____|
//----


class ac4_flat_t {
  public:

    ac4_flat_t(int64_t cell_count=0, int32_t tile_count=0) {
      m_cell_count = cell_count;
      m_tile_count = tile_count;

      if ((m_cell_count > 0) &&
          (m_tile_count > 0)) {
        m_tile_support.resize(6*cell_count*tile_count, 0);
      }
    }

    int init(int64_t cell_count, int32_t tile_count) {
      m_cell_count = cell_count;
      m_tile_count = tile_count;

      if ((m_cell_count > 0) &&
          (m_tile_count > 0)) {
        m_tile_support.resize(6*cell_count*tile_count, 0);
      }

      return 0;
    }

    void clear(void) {
      m_cell_count=-1;
      m_tile_count=-1;
      m_tile_support.clear();
    }


    //---
    //---


    int32_t tileSupport(int32_t idir, int64_t cell, int32_t tile, int32_t val=-1) {
      int64_t pos;
      pos = (idir*m_cell_count*m_tile_count) + (cell*m_tile_count) + tile;
      if (val < 0) { return m_tile_support[pos]; }
      m_tile_support[pos] = val;
      return val;
    }

    int set(ac4_flat_t *src) {
      int32_t idir;
      if (m_tile_support.size() != src->m_tile_support.size()) { return -1; }
      memcpy( &(m_tile_support[0]), &(src->m_tile_support[0]), sizeof(int16_t)*m_tile_support.size() );
      return 0;
    }

    int setCell(ac4_flat_t *src, int64_t cell) {
      int32_t idir;
      if (m_tile_support.size() != src->m_tile_support.size()) { return -1; }
      for (idir=0; idir<6; idir++) {
        memcpy( &(     m_tile_support[ (idir*m_cell_count*m_tile_count) + (cell*m_tile_count) ]),
                &(src->m_tile_support[ (idir*m_cell_count*m_tile_count) + (cell*m_tile_count) ]),
                sizeof(int16_t)*m_tile_count );
      }
      return 0;
    }

    //---
    //---

    void print(void) {
      int32_t idir, tile;
      int64_t cell;
      int32_t fold_w=32;
      int i;

      printf("m_cell_count: %i\n", (int)m_cell_count);
      printf("m_tile_count: %i\n", (int)m_tile_count);

      for (idir=0; idir<6; idir++) {
        printf("==== idir:%i ====\n", (int)idir);
        for (tile=0; tile<m_tile_count; tile++) {

          printf("tile[%i][%i]:", (int)tile, (int)m_cell_count);
          for (cell=0; cell<m_cell_count; cell++) {
            if ((cell%fold_w)==0) { printf("\n "); }
            printf(" %i", (int)tileSupport(idir, cell, tile));
          }
          printf("\n");
        }
        printf("\n");
      }
      printf("\n");

    }

    //---

    int64_t size_estimate(void) { return sizeof(int16_t)*m_tile_support.size(); }

    //---
    //---

    int64_t m_cell_count;
    int32_t m_tile_count;

    std::vector< int16_t > m_tile_support;
};


//----
//     _    ____ _  _   _____ ___ _____ ____  _  _     __  __ ____
//    / \  / ___| || | |_   _|_ _| ____|  _ \| || |   |  \/  |___ \
//   / _ \| |   | || |_  | |  | ||  _| | |_) | || |_  | |\/| | __) |
//  / ___ \ |___|__   _| | |  | || |___|  _ <|__   _| | |  | |/ __/
// /_/   \_\____|  |_|___|_| |___|_____|_| \_\  |_|___|_|  |_|_____|
//                  |_____|                      |_____|
//----


class ac4_tier4_m2_t {
  public:

    int8_t                  m_state;
    int32_t                 m_tile_init;

    int64_t                 m_cell_count;
    int32_t                 m_tile_count;

    int8_t                  m_tier_count;

    std::vector< int8_t >   m_tier_bit_size;
    std::vector< int32_t >  m_tier_tile_count;

    std::vector< int32_t >  m_tile_tier_offset;

    // each tier has subset of tiles packed together in their bit representations:
    // dir_j[
    //   cell_j[
    //     tile_{d_0,g_0}:[s_0 .. s_{t_k-1}]
    //     tile_{d_0,g_1}:[s_0 .. s_{t_k-1}]
    //     ...
    //   ]
    // ]
    //
    std::vector< std::vector< uint8_t > > m_tier_group;

    ac4_tier4_m2_t() {
      m_state = 0;
      m_tile_init = 0;

      m_tier_count = 4;

      m_tier_bit_size.clear();

      m_tier_bit_size.push_back(1);
      m_tier_bit_size.push_back(8);
      m_tier_bit_size.push_back(16);
      m_tier_bit_size.push_back(64);

      m_tier_tile_count.clear();
      m_tier_tile_count.push_back(0);
      m_tier_tile_count.push_back(0);
      m_tier_tile_count.push_back(0);
      m_tier_tile_count.push_back(0);

      m_tile_tier_offset.clear();
      m_tier_group.clear();
    }

    int pre_init(int64_t cell_count, int32_t tile_count) {
      int32_t tile;
      int32_t idir;

      m_cell_count = cell_count;
      m_tile_count = tile_count;

      m_tier_tile_count.clear();
      m_tier_tile_count.resize( m_tier_count, 0 );

      m_tile_tier_offset.clear();
      m_tile_tier_offset.resize( 2*m_tile_count, -1 );

      m_tier_group.clear();
      m_tier_group.resize( m_tier_count );

      m_tile_init = 0;

      m_state = 1;
      return 0;
    }


    // finalize structure
    //
    int init_tile_vec_fin(void) {
      int tier;
      int64_t sz_u8, t;
      int32_t idir;
      size_t sz=0;

      for (tier=0; tier<m_tier_count; tier++) {
        m_tier_group[tier].clear();
      }

      if (m_tier_tile_count[0] > 0) {

        t = 6*m_tier_tile_count[0]*m_cell_count;
        sz_u8 = t/8;
        if ((t%8) != 0) { sz_u8++; }

        m_tier_group[0].resize( sz_u8, 0 );

      }

      if (m_tier_tile_count[1] > 0) {
        sz_u8 = 6*m_tier_tile_count[1]*m_cell_count;
        m_tier_group[1].resize( sz_u8, 0 );
      }

      if (m_tier_tile_count[2] > 0) {
        sz_u8 = 6*m_tier_tile_count[2]*m_cell_count*2;
        m_tier_group[2].resize( sz_u8, 0 );
      }

      if (m_tier_tile_count[3] > 0) {
        sz_u8 = 6*m_tier_tile_count[3]*m_cell_count*8;
        m_tier_group[3].resize( sz_u8, 0 );
      }

      m_state=3;
      return 0;
    }

    // per tile function to init which tier the tile is in.
    // once all tiles have been assigned to a tier, this will
    // call init_tile_vec_fin
    //
    int init_tile_vec(int32_t tile, int8_t bitsize) {

      if (bitsize==1) {

        if (m_tile_tier_offset[2*tile] < 0) { m_tile_init++; }

        m_tile_tier_offset[2*tile+0] = 0;
        m_tile_tier_offset[2*tile+1] = m_tier_tile_count[0];
        m_tier_tile_count[0]++;
      }

      else if (bitsize==8)  {

        if (m_tile_tier_offset[2*tile] < 0) { m_tile_init++; }

        m_tile_tier_offset[2*tile+0] = 1;
        m_tile_tier_offset[2*tile+1] = m_tier_tile_count[1];
        m_tier_tile_count[1]++;
      }

      else if (bitsize==16) {

        if (m_tile_tier_offset[2*tile] < 0) { m_tile_init++; }

        m_tile_tier_offset[2*tile+0] = 2;
        m_tile_tier_offset[2*tile+1] = m_tier_tile_count[2];
        m_tier_tile_count[2]++;
      }

      else if (bitsize==64) {

        if (m_tile_tier_offset[2*tile] < 0) { m_tile_init++; }

        m_tile_tier_offset[2*tile+0] = 3;
        m_tile_tier_offset[2*tile+1] = m_tier_tile_count[3];
        m_tier_tile_count[3]++;
      }
      else { return -1; }

      if (m_tile_init==m_tile_count) {
        m_state=2;
        init_tile_vec_fin();
      }
      return 0;
    }

    //---
    //---

    void clear(void) {
      int tier;

      m_cell_count=0;
      m_tile_count=0;
      m_tile_tier_offset.clear();
      m_tier_tile_count.clear();

      m_tier_group.clear();

    }

    int set(ac4_tier4_m2_t *src) {
      int32_t tier;

      for (tier=0; tier<m_tier_count; tier++) {
        if (m_tier_group[tier].size() != src->m_tier_group[tier].size()) { return -1; }
        m_tier_group[tier] = src->m_tier_group[tier];
      }

      return 0;
    }

    // this is pretty much the reference, but thi is 100x slower than flat
    //
    int _setCell(ac4_tier4_m2_t *src, int64_t cell) {
      int32_t idir, tile;

      for (tile=0; tile<m_tile_count; tile++) {
        for (idir=0; idir<6; idir++) {
          tileSupport(idir, cell, tile, src->tileSupport(idir, cell, tile));
        }
      }

      return 0;
    }


    // better but about 10x slower than flat
    //
    int setCell(ac4_tier4_m2_t *src, int64_t cell) {
      int32_t idir, tile_idx;

      for (tile_idx=0; tile_idx<m_tier_tile_count[0]; tile_idx++) {
        for (idir=0; idir<6; idir++) {
          tileSupport_tier0(idir, cell, tile_idx, src->tileSupport_tier0(idir, cell, tile_idx));
        }
      }

      for (tile_idx=0; tile_idx<m_tier_tile_count[1]; tile_idx++) {
        for (idir=0; idir<6; idir++) {
          tileSupport_tier1(idir, cell, tile_idx, src->tileSupport_tier1(idir, cell, tile_idx));
        }
      }

      for (tile_idx=0; tile_idx<m_tier_tile_count[2]; tile_idx++) {
        for (idir=0; idir<6; idir++) {
          tileSupport_tier2(idir, cell, tile_idx, src->tileSupport_tier2(idir, cell, tile_idx));
        }
      }

      for (tile_idx=0; tile_idx<m_tier_tile_count[3]; tile_idx++) {
        for (idir=0; idir<6; idir++) {
          tileSupport_tier3(idir, cell, tile_idx, src->tileSupport_tier3(idir, cell, tile_idx));
        }
      }

      return 0;
    }

    // kept here in case we need it but it looks like ther emight not be any need
    // for this function.
    // Note that the beginning and end bytes of tier0 will need to be masked.
    // I've been sloppy below to get some timings but the below implementation
    // without the byte masking is incorrect.
    //
    int XXXsetCell(ac4_tier4_m2_t *src, int64_t cell) {
      int32_t idir, tile, tile_idx, tier;
      int64_t c_s, c_e, pos_s, pos_e, n_byte;

      uint8_t x8, y8, p, mask;

      if ((m_cell_count != src->m_cell_count) ||
          (m_tile_count != src->m_tile_count)) {
        return -1;
      }

      for (tier=0; tier<m_tier_count; tier++) {

        if (m_tier_tile_count[tier] < 16) {
          for (tile_idx=0; tile_idx<m_tier_tile_count[tier]; tile_idx++) {
            for (idir=0; idir<6; idir++) {
              switch (tier) {
                case 0: return tileSupport_tier0(idir, cell, tile_idx, src->tileSupport(idir,cell,tile_idx)); break;
                case 1: return tileSupport_tier1(idir, cell, tile_idx, src->tileSupport(idir,cell,tile_idx)); break;
                case 2: return tileSupport_tier2(idir, cell, tile_idx, src->tileSupport(idir,cell,tile_idx)); break;
                case 3: return tileSupport_tier3(idir, cell, tile_idx, src->tileSupport(idir,cell,tile_idx)); break;
                default: return -1;
                         break;
              }
            }
          }
          continue;
        }

        c_s = 6*cell*m_tier_tile_count[tier];
        c_e = 6*((cell+1)*m_tier_tile_count[tier]);

        // UNTESTED
        // SLOPPY!!!! NEED TO MASK OUT BEGINNING AND END BYTES
        //
        if (tier == 0) {
          pos_s = c_s/8;
          pos_e = c_e/8;

          if ((c_s%8) != 0) {
            p = c_s%8;
            x8 = src->m_tier_group[0][pos_s];
            y8 = m_tier_group[0][pos_s];

            mask = (uint8_t)((uint8_t)0xff << (uint8_t)p);
            y8 = y8 & (~mask);
            x8 = x8 & mask;
            y8 = y8 | x8;
            m_tier_group[0][pos_s] = y8;

            pos_s++;
          }

          if (pos_e < m_tier_group[0].size()) {

            if ((c_e%8) != 0) {
              p = c_e%8;
              x8 = src->m_tier_group[0][pos_e];
              y8 = m_tier_group[0][pos_e];

              mask = (uint8_t)((uint8_t)0xff >> (uint8_t)p);
              x8 = x8 & (mask);
              y8 = y8 & (~mask);
              y8 = y8 | x8;
              m_tier_group[0][pos_e] = y8;
            }

          }

          n_byte = pos_e-pos_s;
        }
        else if (tier==1) {
          pos_s = c_s;
          pos_e = c_s;
          n_byte = pos_e-pos_s;
        }
        else if (tier==2) {
          pos_s = c_s*2;
          pos_e = c_e*2;
          n_byte = pos_e - pos_s;
        }
        else if (tier==3) {
          pos_s = c_s*8;
          pos_e = c_e*8;
          n_byte = pos_e - pos_s;
        }

        memcpy( &(m_tier_group[tier][pos_s]), &(src->m_tier_group[tier][pos_s]), n_byte );

      }

      //for (tile=0; tile<m_tile_count; tile++) {
      //  for (idir=0; idir<6; idir++) {
      //    tileSupport(idir, cell, tile, src->tileSupport(idir, cell, tile));
      //  }
      //}

      return 0;
    }

    //---
    //---

    int32_t tileSupport_tier0(int32_t idir, int64_t cell, int32_t tile_idx, int32_t val=-1) {
      int64_t c8, c;
      uint8_t u8, p, mask;


      c = ((idir*m_cell_count*m_tier_tile_count[0]) + (cell*m_tier_tile_count[0]) + tile_idx);
      c8  = c/8;
      p   = c%8;

      u8 = m_tier_group[0][c8];

      mask = ((uint8_t)1 << p);

      if (val < 0) { return (u8 & mask) ? 1 : 0; }
      if (val > 1) { return -1; }

      if      (val == 0)  { u8 &= ~mask; }
      else if (val == 1)  { u8 |=  mask; }

      m_tier_group[0][c8] = u8;

      return val;
    }

    int32_t tileSupport_tier1(int32_t idir, int64_t cell, int32_t tile_idx, int32_t val=-1) {
      int64_t pos;

      pos = ((idir*m_cell_count*m_tier_tile_count[1]) + (cell*m_tier_tile_count[1]) + tile_idx);

      if (val < 0) { return m_tier_group[1][pos]; }
      if (val > 255) { return -1; }
      m_tier_group[1][pos] = val;
      return val;
    }

    int32_t tileSupport_tier2(int32_t idir, int64_t cell, int32_t tile_idx, int32_t val=-1) {
      int64_t pos;
      uint16_t *v;

      pos = 2*((idir*m_cell_count*m_tier_tile_count[2]) + (cell*m_tier_tile_count[2]) + tile_idx);

      v = (uint16_t *)(&(m_tier_group[2][pos]));
      if (val < 0) { return *v; }
      if (val > 65535) { return -1; }
      *v = (uint16_t)val;
      return val;
    }

    int32_t tileSupport_tier3(int32_t idir, int64_t cell, int32_t tile_idx, int32_t val=-1) {
      int64_t pos;
      uint64_t *v;

      //pos = 8*((6*((cell*ttc) + tile_idx)) + idir);
      pos = 8*((idir*m_cell_count*m_tier_tile_count[3]) + (cell*m_tier_tile_count[3]) + tile_idx);


      v = (uint64_t *)(&(m_tier_group[3][pos]));
      if (val < 0) { return *v; }
      *v = (uint64_t)val;
      return val;
    }

    int32_t tileSupport(int32_t idir, int64_t cell, int32_t tile, int32_t val=-1) {
      int32_t tier, tile_idx;

      tier      = m_tile_tier_offset[2*tile];
      tile_idx  = m_tile_tier_offset[2*tile+1];

      switch (tier) {
        case 0: return tileSupport_tier0(idir, cell, tile_idx, val); break;
        case 1: return tileSupport_tier1(idir, cell, tile_idx, val); break;
        case 2: return tileSupport_tier2(idir, cell, tile_idx, val); break;
        case 3: return tileSupport_tier3(idir, cell, tile_idx, val); break;
        default: return -1;
                 break;
      }

      return -1;
    }

    void print(void) {
      int32_t idir, tile;
      int64_t cell;
      int32_t fold_w=32;
      int i;

      printf("m_state: %i\n", (int)m_state);
      printf("m_tile_init: %i\n", (int)m_tile_init);
      printf("m_cell_count: %i\n", (int)m_cell_count);
      printf("m_tile_count: %i\n", (int)m_tile_count);
      printf("m_tier_bit_size[%i]: {", (int)m_tier_bit_size.size());
      for (i=0; i<4; i++) {
        if (i>0) { printf(", "); }
        printf("%i", m_tier_bit_size[i]);
      }
      printf("}\n");
      printf("\n");

      for (idir=0; idir<6; idir++) {
        printf("==== idir:%i ====\n", (int)idir);
        for (tile=0; tile<m_tile_count; tile++) {

          for (cell=0; cell<m_cell_count; cell++) {
            if ((cell%fold_w)==0) { printf("\n "); }
            printf(" %i", (int)tileSupport(idir, cell, tile));
          }
          printf("\n");
        }
        printf("\n");
      }
      printf("\n");

    }

    int64_t size_estimate(void) {
      int i;
      int64_t sz=0;
      for (i=0; i<m_tier_group.size(); i++) {
        sz += m_tier_group[i].size();
      }
      return sz;
    }

};

//----
//     _    ____ _  _   _____ ___ _____ ____  _  _     __  __ _
//    / \  / ___| || | |_   _|_ _| ____|  _ \| || |   |  \/  / |
//   / _ \| |   | || |_  | |  | ||  _| | |_) | || |_  | |\/| | |
//  / ___ \ |___|__   _| | |  | || |___|  _ <|__   _| | |  | | |
// /_/   \_\____|  |_|___|_| |___|_____|_| \_\  |_|___|_|  |_|_|
//                  |_____|                      |_____|
//----


class ac4_tier4_m1_t {
  public:

    int8_t                  m_state;
    int32_t                 m_tile_init;

    int64_t                 m_cell_count;
    int32_t                 m_tile_count;

    int8_t                  m_tier_count;

    std::vector< int8_t >   m_tier_bit_size;
    std::vector< int32_t >  m_tier_tile_count;

    std::vector< int32_t >  m_tile_tier_offset;

    // each tier has subset of tiles packed together in their bit representations:
    // cell_j[  tile_{d_0,g_0}:[s_0 .. s_{t_k-1}] ... tile_{d_5,g_0}:[s_0 .. s_{t_k-1}]
    //          tile_{d_0,g_1}:[s_0 .. s_{t_k-1}] ... tile_{d_5,g_1}:[s_0 .. s_{t_k-1}]
    //          ...
    //       ]
    //
    std::vector< std::vector< uint8_t > > m_tier_group;

    ac4_tier4_m1_t() {
      m_state = 0;
      m_tile_init = 0;

      m_tier_count = 4;

      m_tier_bit_size.clear();

      m_tier_bit_size.push_back(1);
      m_tier_bit_size.push_back(8);
      m_tier_bit_size.push_back(16);
      m_tier_bit_size.push_back(64);

      m_tier_tile_count.clear();
      m_tier_tile_count.push_back(0);
      m_tier_tile_count.push_back(0);
      m_tier_tile_count.push_back(0);
      m_tier_tile_count.push_back(0);

      m_tile_tier_offset.clear();
      m_tier_group.clear();
    }

    int pre_init(int64_t cell_count, int32_t tile_count) {
      int32_t tile;
      int32_t idir;

      m_cell_count = cell_count;
      m_tile_count = tile_count;

      m_tier_tile_count.clear();
      m_tier_tile_count.resize( m_tier_count, 0 );

      m_tile_tier_offset.clear();
      m_tile_tier_offset.resize( 2*m_tile_count, -1 );

      m_tier_group.clear();
      m_tier_group.resize( m_tier_count );

      m_tile_init = 0;

      m_state = 1;
      return 0;
    }


    // finalize structure
    //
    int init_tile_vec_fin(void) {
      int tier;
      int64_t sz_u8, t;
      int32_t idir;
      size_t sz=0;

      for (tier=0; tier<m_tier_count; tier++) {
        m_tier_group[tier].clear();
      }

      if (m_tier_tile_count[0] > 0) {

        t = 6*m_tier_tile_count[0]*m_cell_count;
        sz_u8 = t/8;
        if ((t%8) != 0) { sz_u8++; }

        m_tier_group[0].resize( sz_u8, 0 );

      }

      if (m_tier_tile_count[1] > 0) {
        sz_u8 = 6*m_tier_tile_count[1]*m_cell_count;
        m_tier_group[1].resize( sz_u8, 0 );
      }

      if (m_tier_tile_count[2] > 0) {
        sz_u8 = 6*m_tier_tile_count[2]*m_cell_count*2;
        m_tier_group[2].resize( sz_u8, 0 );
      }

      if (m_tier_tile_count[3] > 0) {
        sz_u8 = 6*m_tier_tile_count[3]*m_cell_count*8;
        m_tier_group[3].resize( sz_u8, 0 );
      }

      m_state=3;
      return 0;
    }

    // per tile function to init which tier the tile is in.
    // once all tiles have been assigned to a tier, this will
    // call init_tile_vec_fin
    //
    int init_tile_vec(int32_t tile, int8_t bitsize) {

      if (bitsize==1) {

        if (m_tile_tier_offset[2*tile] < 0) { m_tile_init++; }

        m_tile_tier_offset[2*tile+0] = 0;
        m_tile_tier_offset[2*tile+1] = m_tier_tile_count[0];
        m_tier_tile_count[0]++;
      }

      else if (bitsize==8)  {

        if (m_tile_tier_offset[2*tile] < 0) { m_tile_init++; }

        m_tile_tier_offset[2*tile+0] = 1;
        m_tile_tier_offset[2*tile+1] = m_tier_tile_count[1];
        m_tier_tile_count[1]++;
      }

      else if (bitsize==16) {

        if (m_tile_tier_offset[2*tile] < 0) { m_tile_init++; }

        m_tile_tier_offset[2*tile+0] = 2;
        m_tile_tier_offset[2*tile+1] = m_tier_tile_count[2];
        m_tier_tile_count[2]++;
      }

      else if (bitsize==64) {

        if (m_tile_tier_offset[2*tile] < 0) { m_tile_init++; }

        m_tile_tier_offset[2*tile+0] = 3;
        m_tile_tier_offset[2*tile+1] = m_tier_tile_count[3];
        m_tier_tile_count[3]++;
      }
      else { return -1; }

      if (m_tile_init==m_tile_count) {
        m_state=2;
        init_tile_vec_fin();
      }
      return 0;
    }

    //---
    //---

    void clear(void) {
      int tier;

      m_cell_count=0;
      m_tile_count=0;
      m_tile_tier_offset.clear();
      m_tier_tile_count.clear();

      m_tier_group.clear();

    }

    int set(ac4_tier4_m1_t *src) {
      int32_t tier;

      for (tier=0; tier<m_tier_count; tier++) {
        if (m_tier_group[tier].size() != src->m_tier_group[tier].size()) { return -1; }
        m_tier_group[tier] = src->m_tier_group[tier];
      }

      return 0;
    }

    // this is pretty much the reference, but thi is 100x slower than flat
    //
    int _setCell(ac4_tier4_m1_t *src, int64_t cell) {
      int32_t idir, tile;

      for (tile=0; tile<m_tile_count; tile++) {
        for (idir=0; idir<6; idir++) {
          tileSupport(idir, cell, tile, src->tileSupport(idir, cell, tile));
        }
      }

      return 0;
    }


    // better but about 10x slower than flat
    //
    int setCell(ac4_tier4_m1_t *src, int64_t cell) {
      int32_t idir, tile_idx;

      for (tile_idx=0; tile_idx<m_tier_tile_count[0]; tile_idx++) {
        for (idir=0; idir<6; idir++) {
          tileSupport_tier0(idir, cell, tile_idx, src->tileSupport_tier0(idir, cell, tile_idx));
        }
      }

      for (tile_idx=0; tile_idx<m_tier_tile_count[1]; tile_idx++) {
        for (idir=0; idir<6; idir++) {
          tileSupport_tier1(idir, cell, tile_idx, src->tileSupport_tier1(idir, cell, tile_idx));
        }
      }

      for (tile_idx=0; tile_idx<m_tier_tile_count[2]; tile_idx++) {
        for (idir=0; idir<6; idir++) {
          tileSupport_tier2(idir, cell, tile_idx, src->tileSupport_tier2(idir, cell, tile_idx));
        }
      }

      for (tile_idx=0; tile_idx<m_tier_tile_count[3]; tile_idx++) {
        for (idir=0; idir<6; idir++) {
          tileSupport_tier3(idir, cell, tile_idx, src->tileSupport_tier3(idir, cell, tile_idx));
        }
      }

      return 0;
    }

    // kept here in case we need it but it looks like ther emight not be any need
    // for this function.
    // Note that the beginning and end bytes of tier0 will need to be masked.
    // I've been sloppy below to get some timings but the below implementation
    // without the byte masking is incorrect.
    //
    int XXXsetCell(ac4_tier4_m1_t *src, int64_t cell) {
      int32_t idir, tile, tile_idx, tier;
      int64_t c_s, c_e, pos_s, pos_e, n_byte;

      uint8_t x8, y8, p, mask;

      if ((m_cell_count != src->m_cell_count) ||
          (m_tile_count != src->m_tile_count)) {
        return -1;
      }

      for (tier=0; tier<m_tier_count; tier++) {

        if (m_tier_tile_count[tier] < 16) {
          for (tile_idx=0; tile_idx<m_tier_tile_count[tier]; tile_idx++) {
            for (idir=0; idir<6; idir++) {
              switch (tier) {
                case 0: return tileSupport_tier0(idir, cell, tile_idx, src->tileSupport(idir,cell,tile_idx)); break;
                case 1: return tileSupport_tier1(idir, cell, tile_idx, src->tileSupport(idir,cell,tile_idx)); break;
                case 2: return tileSupport_tier2(idir, cell, tile_idx, src->tileSupport(idir,cell,tile_idx)); break;
                case 3: return tileSupport_tier3(idir, cell, tile_idx, src->tileSupport(idir,cell,tile_idx)); break;
                default: return -1;
                         break;
              }
            }
          }
          continue;
        }

        c_s = 6*cell*m_tier_tile_count[tier];
        c_e = 6*((cell+1)*m_tier_tile_count[tier]);

        // UNTESTED
        // SLOPPY!!!! NEED TO MASK OUT BEGINNING AND END BYTES
        //
        if (tier == 0) {
          pos_s = c_s/8;
          pos_e = c_e/8;

          if ((c_s%8) != 0) {
            p = c_s%8;
            x8 = src->m_tier_group[0][pos_s];
            y8 = m_tier_group[0][pos_s];

            mask = (uint8_t)((uint8_t)0xff << (uint8_t)p);
            y8 = y8 & (~mask);
            x8 = x8 & mask;
            y8 = y8 | x8;
            m_tier_group[0][pos_s] = y8;

            pos_s++;
          }

          if (pos_e < m_tier_group[0].size()) {

            if ((c_e%8) != 0) {
              p = c_e%8;
              x8 = src->m_tier_group[0][pos_e];
              y8 = m_tier_group[0][pos_e];

              mask = (uint8_t)((uint8_t)0xff >> (uint8_t)p);
              x8 = x8 & (mask);
              y8 = y8 & (~mask);
              y8 = y8 | x8;
              m_tier_group[0][pos_e] = y8;
            }

          }

          n_byte = pos_e-pos_s;
        }
        else if (tier==1) {
          pos_s = c_s;
          pos_e = c_s;
          n_byte = pos_e-pos_s;
        }
        else if (tier==2) {
          pos_s = c_s*2;
          pos_e = c_e*2;
          n_byte = pos_e - pos_s;
        }
        else if (tier==3) {
          pos_s = c_s*8;
          pos_e = c_e*8;
          n_byte = pos_e - pos_s;
        }

        memcpy( &(m_tier_group[tier][pos_s]), &(src->m_tier_group[tier][pos_s]), n_byte );

      }

      //for (tile=0; tile<m_tile_count; tile++) {
      //  for (idir=0; idir<6; idir++) {
      //    tileSupport(idir, cell, tile, src->tileSupport(idir, cell, tile));
      //  }
      //}

      return 0;
    }

    //---
    //---

    int32_t tileSupport_tier0(int32_t idir, int64_t cell, int32_t tile_idx, int32_t val=-1) {
      int64_t c8, c;
      uint8_t u8, p, mask;


      c = ((6*((cell*m_tier_tile_count[0]) + tile_idx)) + idir);
      c8  = c/8;
      p   = c%8;

      u8 = m_tier_group[0][c8];

      mask = ((uint8_t)1 << p);

      if (val < 0) { return (u8 & mask) ? 1 : 0; }
      if (val > 1) { return -1; }

      if      (val == 0)  { u8 &= ~mask; }
      else if (val == 1)  { u8 |=  mask; }

      m_tier_group[0][c8] = u8;

      return val;
    }

    int32_t tileSupport_tier1(int32_t idir, int64_t cell, int32_t tile_idx, int32_t val=-1) {
      int64_t pos;

      pos = (6*((cell*m_tier_tile_count[1]) + tile_idx)) + idir;

      if (val < 0) { return m_tier_group[1][pos]; }
      if (val > 255) { return -1; }
      m_tier_group[1][pos] = val;
      return val;
    }

    int32_t tileSupport_tier2(int32_t idir, int64_t cell, int32_t tile_idx, int32_t val=-1) {
      int64_t pos, ttc = 0;
      uint16_t *v;

      ttc = m_tier_tile_count[2];
      pos = 2*((6*((cell*m_tier_tile_count[2]) + tile_idx)) + idir);

      v = (uint16_t *)(&(m_tier_group[2][pos]));
      if (val < 0) { return *v; }
      if (val > 65535) { return -1; }
      *v = (uint16_t)val;
      return val;
    }

    int32_t tileSupport_tier3(int32_t idir, int64_t cell, int32_t tile_idx, int32_t val=-1) {
      int64_t pos, ttc = 0;
      uint64_t *v;

      ttc = m_tier_tile_count[3];
      pos = 8*((6*((cell*ttc) + tile_idx)) + idir);


      v = (uint64_t *)(&(m_tier_group[3][pos]));
      if (val < 0) { return *v; }
      *v = (uint64_t)val;
      return val;
    }

    int32_t tileSupport(int32_t idir, int64_t cell, int32_t tile, int32_t val=-1) {
      int32_t tier, tile_idx;

      tier      = m_tile_tier_offset[2*tile];
      tile_idx  = m_tile_tier_offset[2*tile+1];

      switch (tier) {
        case 0: return tileSupport_tier0(idir, cell, tile_idx, val); break;
        case 1: return tileSupport_tier1(idir, cell, tile_idx, val); break;
        case 2: return tileSupport_tier2(idir, cell, tile_idx, val); break;
        case 3: return tileSupport_tier3(idir, cell, tile_idx, val); break;
        default: return -1;
                 break;
      }

      return -1;
    }

    void print(void) {
      int32_t idir, tile;
      int64_t cell;
      int32_t fold_w=32;
      int i;

      printf("m_state: %i\n", (int)m_state);
      printf("m_tile_init: %i\n", (int)m_tile_init);
      printf("m_cell_count: %i\n", (int)m_cell_count);
      printf("m_tile_count: %i\n", (int)m_tile_count);
      printf("m_tier_bit_size[%i]: {", (int)m_tier_bit_size.size());
      for (i=0; i<4; i++) {
        if (i>0) { printf(", "); }
        printf("%i", m_tier_bit_size[i]);
      }
      printf("}\n");
      printf("\n");

      for (idir=0; idir<6; idir++) {
        printf("==== idir:%i ====\n", (int)idir);
        for (tile=0; tile<m_tile_count; tile++) {

          for (cell=0; cell<m_cell_count; cell++) {
            if ((cell%fold_w)==0) { printf("\n "); }
            printf(" %i", (int)tileSupport(idir, cell, tile));
          }
          printf("\n");
        }
        printf("\n");
      }
      printf("\n");

    }

    int64_t size_estimate(void) {
      int i;
      int64_t sz=0;
      for (i=0; i<m_tier_group.size(); i++) {
        sz += m_tier_group[i].size();
      }
      return sz;
    }

};

//----
//     _    ____ _  _   _____ ___ _____ ____  _  _
//    / \  / ___| || | |_   _|_ _| ____|  _ \| || |
//   / _ \| |   | || |_  | |  | ||  _| | |_) | || |_
//  / ___ \ |___|__   _| | |  | || |___|  _ <|__   _|
// /_/   \_\____|  |_|___|_| |___|_____|_| \_\  |_|
//                  |_____|
//----

class ac4_tier4_t {
  public:

    ac4_tier4_t() {
      m_state = 0;
      m_tile_init = 0;

      m_tier_count = 4;

      m_tier_bit_size.clear();

      m_tier_bit_size.push_back(1);
      m_tier_bit_size.push_back(8);
      m_tier_bit_size.push_back(16);
      m_tier_bit_size.push_back(64);
    }

    int pre_init(int64_t cell_count, int32_t tile_count) {
      int32_t tile;
      int32_t idir;

      m_cell_count = cell_count;
      m_tile_count = tile_count;

      for (idir=0; idir<6; idir++) {
        m_tile_support[idir].resize( m_tile_count );

        for (tile=0; tile<m_tile_count; tile++) {
          m_tile_support[idir][tile].clear();
        }
      }

      m_tile_tier.resize(m_tile_count, -1);

      m_tile_init = 0;
      m_state = 1;

      return 0;
    }


    int init_tile_vec(int32_t tile, int8_t bitsize) {
      int32_t idir;
      size_t sz=0;

      if (bitsize==1) {
        sz = m_cell_count / 8;
        if (m_cell_count%8) { sz++; }
        m_tile_tier[tile] = 0;
      }
      else if (bitsize==8)  {
        sz = m_cell_count;
        m_tile_tier[tile] = 1;
      }
      else if (bitsize==16) {
        sz = m_cell_count*2;
        m_tile_tier[tile] = 2;
      }
      else if (bitsize==64) {
        sz = m_cell_count*8;
        m_tile_tier[tile] = 3;
      }
      else { return -1; }

      for (idir=0; idir<6; idir++) {
        m_tile_support[idir][tile].resize(sz, 0);
      }

      m_tile_init++;

      if (m_tile_init==m_tile_count) { m_state=2; }

      return 0;
    }

    //---
    //---

    void clear(void) {
      m_cell_count=-1;
      m_tile_count=-1;
      m_tile_support[0].clear();
      m_tile_support[1].clear();
      m_tile_support[2].clear();
      m_tile_support[3].clear();
      m_tile_support[4].clear();
      m_tile_support[5].clear();

      m_tile_tier.clear();
    }

    int set(ac4_tier4_t *src) {
      int32_t idir, tile;

      for (idir=0; idir<6; idir++) {
        if (m_tile_support[idir].size() != src->m_tile_support[idir].size()) { return -1; }
        for (tile=0; tile<m_tile_count; tile++) {
          if (m_tile_support[idir][tile].size() != src->m_tile_support[idir][tile].size()) { return -1; }
          m_tile_support[idir][tile] = src->m_tile_support[idir][tile];
        }
      }
      return 0;
    }

    int setCell(ac4_tier4_t *src, int64_t cell) {
      int32_t idir, tile;

      for (idir=0; idir<6; idir++) {
        if (m_tile_support[idir].size() != src->m_tile_support[idir].size()) { return -1; }
        for (tile=0; tile<m_tile_count; tile++) {
          tileSupport(idir, cell, tile, src->tileSupport(idir, cell, tile));
        }
      }

      return 0;
    }

    //---
    //---

    int32_t tileSupport_tier0(int32_t idir, int64_t cell, int32_t tile, int32_t val=-1) {
      int64_t c8;
      uint8_t u8, p, mask;

      c8 = cell/8;

      //u8 = m_tile_support[idir][tile][cell/8];
      u8 = m_tile_support[idir][tile][c8];
      p = cell%8;

      mask = ((uint8_t)1 << p);

      if (val < 0) { return (u8 & mask) ? 1 : 0; }
      if (val > 1) { return -1; }

      if      (val == 0)  { u8 &= ~mask; }
      else if (val == 1)  { u8 |=  mask; }

      //m_tile_support[idir][tile][cell/8] = u8;
      m_tile_support[idir][tile][c8] = u8;

      return val;
    }

    int32_t tileSupport_tier1(int32_t idir, int64_t cell, int32_t tile, int32_t val=-1) {
      if (val < 0) { return m_tile_support[idir][tile][cell]; }
      if (val > 255) { return -1; }
      m_tile_support[idir][tile][cell] = (uint8_t)val;
      return val;
    }

    int32_t tileSupport_tier2(int32_t idir, int64_t cell, int32_t tile, int32_t val=-1) {
      uint16_t *v;
      v = (uint16_t *)(&(m_tile_support[idir][tile][0]));
      if (val < 0) { return v[cell]; }
      if (val > 65535) { return -1; }
      v[cell] = (uint16_t)val;
      return val;
    }

    int32_t tileSupport_tier3(int32_t idir, int64_t cell, int32_t tile, int32_t val=-1) {
      uint64_t *v;
      v = (uint64_t *)(&(m_tile_support[idir][tile][0]));
      if (val < 0) { return (int32_t)v[cell]; }
      v[cell] = (uint64_t)val;
      return val;
    }

    int32_t tileSupport(int32_t idir, int64_t cell, int32_t tile, int32_t val=-1) {
      int32_t tier;

      tier = m_tile_tier[tile];

      switch (tier) {
        case 0: return tileSupport_tier0(idir, cell, tile, val); break;
        case 1: return tileSupport_tier1(idir, cell, tile, val); break;
        case 2: return tileSupport_tier2(idir, cell, tile, val); break;
        case 3: return tileSupport_tier3(idir, cell, tile, val); break;
        default: return -1;
                 break;
      }

      return -1;
    }

    void print(void) {
      int32_t idir, tile;
      int64_t cell;
      int32_t fold_w=32;
      int i;

      printf("m_state: %i\n", (int)m_state);
      printf("m_tile_init: %i\n", (int)m_tile_init);
      printf("m_cell_count: %i\n", (int)m_cell_count);
      printf("m_tile_count: %i\n", (int)m_tile_count);
      printf("m_tier_bit_size[%i]: {", (int)m_tier_bit_size.size());
      for (i=0; i<4; i++) {
        if (i>0) { printf(", "); }
        printf("%i", m_tier_bit_size[i]);
      }
      printf("}\n");
      printf("\n");

      for (idir=0; idir<6; idir++) {
        printf("==== idir:%i ====\n", (int)idir);
        for (tile=0; tile<m_tile_count; tile++) {

          printf("tile[%i][%i]{%i}:", (int)tile, (int)m_cell_count, (int)m_tile_tier[tile]);
          for (cell=0; cell<m_cell_count; cell++) {
            if ((cell%fold_w)==0) { printf("\n "); }
            printf(" %i", (int)tileSupport(idir, cell, tile));
          }
          printf("\n");
        }
        printf("\n");
      }
      printf("\n");

    }

    int64_t size_estimate(void) {
      int64_t cell;
      int32_t idir, tile;
      int64_t sz=0;
      for (idir=0; idir<6; idir++) {
        for (tile=0; tile<m_tile_count; tile++) {
          sz += m_tile_support[idir][tile].size();
        }
      }
      return sz;
    }

    int8_t                  m_state;
    int32_t                 m_tile_init;

    int64_t                 m_cell_count;
    int32_t                 m_tile_count;

    std::vector< int8_t >   m_tile_tier;

    int8_t                  m_tier_count;
    std::vector< int32_t >  m_tier_bit_size;

    // m_tile_support[ DIR ][ TILE ][ CELL ]
    //
    std::vector< std::vector< uint8_t > > m_tile_support[6];
};

//---
//---

//----
//     _    ____ _  _   _____ ___ _____ ____   __
//    / \  / ___| || | |_   _|_ _| ____|  _ \ / /_
//   / _ \| |   | || |_  | |  | ||  _| | |_) | '_ \
//  / ___ \ |___|__   _| | |  | || |___|  _ <| (_) |
// /_/   \_\____|  |_|___|_| |___|_____|_| \_\\___/
//                  |_____|
//----


class ac4_tier6_t {
  public:

    ac4_tier6_t() {
      m_state = 0;
      m_tile_init = 0;

      m_tier_bit_size.push_back(1);
      m_tier_bit_size.push_back(2);
      m_tier_bit_size.push_back(4);
      m_tier_bit_size.push_back(8);
      m_tier_bit_size.push_back(16);
      m_tier_bit_size.push_back(64);

      m_tier_count = m_tier_bit_size.size();
    }

    int pre_init(int64_t cell_count, int32_t tile_count) {
      int32_t tile;
      int32_t idir;

      m_cell_count = cell_count;
      m_tile_count = tile_count;

      for (idir=0; idir<6; idir++) {
        m_tile_support[idir].resize( m_tile_count );

        for (tile=0; tile<m_tile_count; tile++) {
          m_tile_support[idir][tile].clear();
        }
      }

      m_tile_tier.resize(m_tile_count, -1);

      m_tile_init = 0;
      m_state = 1;

      return 0;
    }


    int init_tile_vec(int32_t tile, int8_t bitsize) {
      int32_t idir;
      size_t sz=0;

      if (bitsize==1) {
        sz = m_cell_count / 8;
        if (m_cell_count%8) { sz++; }
        m_tile_tier[tile] = 0;
      }
      else if (bitsize==2)  {
        sz = m_cell_count / 4;
        if (m_cell_count%4) { sz++; }
        m_tile_tier[tile] = 1;
      }
      else if (bitsize==4)  {
        sz = m_cell_count / 2;
        if (m_cell_count%2) { sz++; }
        m_tile_tier[tile] = 2;
      }
      else if (bitsize==8)  {
        sz = m_cell_count;
        m_tile_tier[tile] = 3;
      }
      else if (bitsize==16) {
        sz = m_cell_count*2;
        m_tile_tier[tile] = 4;
      }
      else if (bitsize==64) {
        sz = m_cell_count*8;
        m_tile_tier[tile] = 5;
      }
      else { return -1; }

      for (idir=0; idir<6; idir++) {
        m_tile_support[idir][tile].resize(sz, 0);
      }

      m_tile_init++;

      if (m_tile_init==m_tile_count) { m_state=2; }

      return 0;
    }

    //---
    //---

    void clear(void) {
      m_cell_count = -1;
      m_tile_count = -1;
      m_tile_support[0].clear();
      m_tile_support[1].clear();
      m_tile_support[2].clear();
      m_tile_support[3].clear();
      m_tile_support[4].clear();
      m_tile_support[5].clear();

      m_tile_init=0;

      m_tile_tier.clear();
    }

    int set(ac4_tier6_t *src) {
      int32_t idir, tile;

      for (idir=0; idir<6; idir++) {
        if (m_tile_support[idir].size() != src->m_tile_support[idir].size()) { return -1; }
        for (tile=0; tile<m_tile_count; tile++) {
          if (m_tile_support[idir][tile].size() != src->m_tile_support[idir][tile].size()) { return -1; }
          m_tile_support[idir][tile] = src->m_tile_support[idir][tile];
        }
      }
      return 0;
    }

    int setCell(ac4_tier6_t *src, int64_t cell) {
      int32_t idir, tile;

      for (idir=0; idir<6; idir++) {
        if (m_tile_support[idir].size() != src->m_tile_support[idir].size()) { return -1; }
        for (tile=0; tile<m_tile_count; tile++) {
          tileSupport(idir, cell, tile, src->tileSupport(idir, cell, tile));
        }
      }

      return 0;
    }

    //---
    //---

    int32_t tileSupport_tier0(int32_t idir, int64_t cell, int32_t tile, int32_t val=-1) {
      uint8_t u8, p, mask;

      u8 = m_tile_support[idir][tile][cell/8];
      p = cell%8;

      mask = ((uint8_t)1 << p);

      if (val < 0) { return (u8 & mask) ? 1 : 0; }
      if (val > 1) { return -1; }

      if      (val == 0)  { u8 &= ~mask; }
      else if (val == 1)  { u8 |=  mask; }

      m_tile_support[idir][tile][cell/8] = u8;

      return val;
    }

    int32_t tileSupport_tier1(int32_t idir, int64_t cell, int32_t tile, int32_t val=-1) {
      int64_t cell_q;
      uint8_t u8, p, mask, bshift, v8;

      cell_q = cell/4;

      u8 = m_tile_support[idir][tile][cell_q];
      p = cell%4;

      bshift = 2*p;

      mask = ((uint8_t)0x3 << bshift);

      if (val < 0) { return ((u8 & mask) >> bshift); }
      if (val > 3) { return -1; }

      v8 = (uint8_t)(val&0x3);
      v8 = (v8 << bshift);

      // zero out two bits where val will go
      // and add shifted 2 bit val in
      //
      u8 &= ~mask;
      u8 = u8 + v8;

      m_tile_support[idir][tile][cell_q] = u8;

      return val;
    }

    int32_t tileSupport_tier2(int32_t idir, int64_t cell, int32_t tile, int32_t val=-1) {
      int64_t cell_q;
      uint8_t u8, p, mask, bshift, v8;

      cell_q = cell/2;

      u8 = m_tile_support[idir][tile][cell_q];
      p = cell%2;

      bshift = 4*p;

      mask = ((uint8_t)0xf << bshift);

      if (val < 0) { return ((u8 & mask) >> bshift); }
      if (val > 15) { return -1; }

      v8 = (uint8_t)(val&0xf);
      v8 = (v8 << bshift);

      // zero out two bits where val will go
      // and add shifted 2 bit val in
      //
      u8 &= ~mask;
      u8 = u8 + v8;

      m_tile_support[idir][tile][cell_q] = u8;

      return val;
    }

    int32_t tileSupport_tier3(int32_t idir, int64_t cell, int32_t tile, int32_t val=-1) {
      if (val < 0) { return m_tile_support[idir][tile][cell]; }
      if (val > 255) { return -1; }
      m_tile_support[idir][tile][cell] = (uint8_t)val;
      return val;
    }

    int32_t tileSupport_tier4(int32_t idir, int64_t cell, int32_t tile, int32_t val=-1) {
      uint16_t *v;
      v = (uint16_t *)(&(m_tile_support[idir][tile][0]));
      if (val < 0) { return v[cell]; }
      if (val > 65535) { return -1; }
      v[cell] = (uint16_t)val;
      return val;
    }

    int32_t tileSupport_tier5(int32_t idir, int64_t cell, int32_t tile, int32_t val=-1) {
      uint64_t *v;
      v = (uint64_t *)(&(m_tile_support[idir][tile][0]));
      if (val < 0) { return (int32_t)v[cell]; }
      v[cell] = (uint64_t)val;
      return val;
    }

    int32_t tileSupport(int32_t idir, int64_t cell, int32_t tile, int32_t val=-1) {
      int32_t tier;

      tier = m_tile_tier[tile];

      switch (tier) {
        case 0: return tileSupport_tier0(idir, cell, tile, val); break;
        case 1: return tileSupport_tier1(idir, cell, tile, val); break;
        case 2: return tileSupport_tier2(idir, cell, tile, val); break;
        case 3: return tileSupport_tier3(idir, cell, tile, val); break;
        case 4: return tileSupport_tier4(idir, cell, tile, val); break;
        case 5: return tileSupport_tier5(idir, cell, tile, val); break;
        default: return -1;
                 break;
      }

      return -1;
    }

    /*
    void printSummary(void) {
      int32_t idir, tile, tier;
      int64_t cell;
      int32_t fold_w=32;
      int i;

      printf("m_state: %i\n", (int)m_state);
      printf("m_tile_init: %i\n", (int)m_tile_init);
      printf("m_cell_count: %i\n", (int)m_cell_count);
      printf("m_tile_count: %i\n", (int)m_tile_count);
      printf("m_tier_bit_size[%i]: {", (int)m_tier_bit_size.size());
      for (i=0; i<4; i++) {
        if (i>0) { printf(", "); }
        printf("%i", m_tier_bit_size[i]);
      }
      printf("}\n");
      printf("\n");

      for (tier=0; tier<m_tier_count; tier++) {
        printf("  m_");
      }

    }
    */

    void print(void) {
      int32_t idir, tile;
      int64_t cell;
      int32_t fold_w=32;
      int i;

      printf("m_state: %i\n", (int)m_state);
      printf("m_tile_init: %i\n", (int)m_tile_init);
      printf("m_cell_count: %i\n", (int)m_cell_count);
      printf("m_tile_count: %i\n", (int)m_tile_count);
      printf("m_tier_bit_size[%i]: {", (int)m_tier_bit_size.size());
      for (i=0; i<4; i++) {
        if (i>0) { printf(", "); }
        printf("%i", m_tier_bit_size[i]);
      }
      printf("}\n");
      printf("\n");

      for (idir=0; idir<6; idir++) {
        printf("==== idir:%i ====\n", (int)idir);
        for (tile=0; tile<m_tile_count; tile++) {

          printf("tile[%i][%i]{%i}:", (int)tile, (int)m_cell_count, (int)m_tile_tier[tile]);
          for (cell=0; cell<m_cell_count; cell++) {
            if ((cell%fold_w)==0) { printf("\n "); }
            printf(" %i", (int)tileSupport(idir, cell, tile));
          }
          printf("\n");
        }
        printf("\n");
      }
      printf("\n");

    }

    int64_t size_estimate(void) {
      int64_t cell;
      int32_t idir, tile;
      int64_t sz=0;
      for (idir=0; idir<6; idir++) {
        for (tile=0; tile<m_tile_count; tile++) {
          sz += m_tile_support[idir][tile].size();
        }
      }
      return sz;
    }

    int8_t                  m_state;
    int32_t                 m_tile_init;

    int64_t                 m_cell_count;
    int32_t                 m_tile_count;

    std::vector< int8_t >   m_tile_tier;

    int8_t                  m_tier_count;
    std::vector< int32_t >  m_tier_bit_size;

    // m_tile_support[ DIR ][ TILE ][ CELL ]
    //
    std::vector< std::vector< uint8_t > > m_tile_support[6];
};

//----
//     _    ____ _  _   _____ ___ _____ ____   __      __  __ _
//    / \  / ___| || | |_   _|_ _| ____|  _ \ / /_    |  \/  / |
//   / _ \| |   | || |_  | |  | ||  _| | |_) | '_ \   | |\/| | |
//  / ___ \ |___|__   _| | |  | || |___|  _ <| (_) |  | |  | | |
// /_/   \_\____|  |_|___|_| |___|_____|_| \_\\___/___|_|  |_|_|
//                  |_____|                      |_____|
//----


//  ____  _   _  ____  ______   __
// | __ )| | | |/ ___|/ ___\ \ / /
// |  _ \| | | | |  _| |  _ \ V /
// | |_) | |_| | |_| | |_| | | |
// |____/ \___/ \____|\____| |_|
//

// THERES A BUG SOMEWHERE HERE, OLOZ IS FAILING WHEN USING THIS CLASS
//
class ac4_tier6_m1_t {
  public:

    int8_t                  m_state;
    int32_t                 m_tile_init;

    int64_t                 m_cell_count;
    int32_t                 m_tile_count;

    int8_t                  m_tier_count;

    std::vector< int8_t >   m_tier_bit_size;
    std::vector< int32_t >  m_tier_tile_count;

    // interleaved tile tier and tile offset in tier group
    //
    std::vector< int32_t >  m_tile_tier_offset;

    // each tier has subset of tiles packed together in their bit representations:
    // cell_j[  tile_{d_0,g_0}:[s_0 .. s_{t_k-1}] ... tile_{d_5,g_0}:[s_0 .. s_{t_k-1}]
    //          tile_{d_0,g_1}:[s_0 .. s_{t_k-1}] ... tile_{d_5,g_1}:[s_0 .. s_{t_k-1}]
    //          ...
    //       ]
    //
    std::vector< std::vector< uint8_t > > m_tier_group;

    ac4_tier6_m1_t() {
      m_state = 0;
      m_tile_init = 0;

      m_tier_count = 6;

      m_tier_bit_size.clear();

      m_tier_bit_size.push_back(1);
      m_tier_bit_size.push_back(2);
      m_tier_bit_size.push_back(4);
      m_tier_bit_size.push_back(8);
      m_tier_bit_size.push_back(16);
      m_tier_bit_size.push_back(64);

      m_tier_tile_count.clear();
      m_tier_tile_count.push_back(0);
      m_tier_tile_count.push_back(0);
      m_tier_tile_count.push_back(0);
      m_tier_tile_count.push_back(0);
      m_tier_tile_count.push_back(0);
      m_tier_tile_count.push_back(0);

      m_tile_tier_offset.clear();
      m_tier_group.clear();
    }

    int pre_init(int64_t cell_count, int32_t tile_count) {
      int32_t tile;
      int32_t idir;

      m_cell_count = cell_count;
      m_tile_count = tile_count;

      m_tier_tile_count.clear();
      m_tier_tile_count.resize( m_tier_count, 0 );

      m_tile_tier_offset.clear();
      m_tile_tier_offset.resize( 2*m_tile_count, -1 );

      m_tier_group.clear();
      m_tier_group.resize( m_tier_count );

      m_tile_init = 0;

      m_state = 1;
      return 0;
    }


    // finalize structure
    //
    int init_tile_vec_fin(void) {
      int tier;
      int64_t sz_u8, t;
      int32_t idir;
      size_t sz=0;

      int64_t _num[6] = { 1, 1, 1, 1, 2, 8 };
      int64_t _den[6] = { 8, 4, 2, 1, 1, 1 };

      for (tier=0; tier<m_tier_count; tier++) {
        m_tier_group[tier].clear();

        if (m_tier_tile_count[tier] > 0) {
          t = 6*m_tier_tile_count[tier]*m_cell_count;
          sz_u8 = _num[tier] * t / _den[tier];
          if ((t%_den[tier]) != 0) { sz_u8++; }
          m_tier_group[tier].resize( sz_u8, 0 );
        }

      }

      m_state=3;
      return 0;
    }

    // per tile function to init which tier the tile is in.
    // once all tiles have been assigned to a tier, this will
    // call init_tile_vec_fin
    //
    int init_tile_vec(int32_t tile, int8_t bitsize) {

      int i;
      int8_t _bs[6] = {1,2,4,8,16,64};

      for (i=0; i<6; i++) {
        if (_bs[i] == bitsize) {
          if (m_tile_tier_offset[2*tile] < 0) { m_tile_init++; }
          m_tile_tier_offset[2*tile+0] = i;
          m_tile_tier_offset[2*tile+1] = m_tier_tile_count[i];
          m_tier_tile_count[i]++;
          break;
        }
      }
      if (i==6) { return -1; }

      if (m_tile_init == m_tile_count) {
        m_state=2;
        init_tile_vec_fin();
      }
      return 0;
    }

    //---
    //---

    void clear(void) {
      int tier;

      m_cell_count=0;
      m_tile_count=0;
      m_tile_tier_offset.clear();
      m_tier_tile_count.clear();

      m_tile_init=0;

      m_tier_group.clear();

    }

    int set(ac4_tier6_m1_t *src) {
      int32_t tier;

      for (tier=0; tier<m_tier_count; tier++) {
        if (m_tier_group[tier].size() != src->m_tier_group[tier].size()) { return -1; }
        m_tier_group[tier] = src->m_tier_group[tier];
      }

      return 0;
    }

    int setCell(ac4_tier6_m1_t *src, int64_t cell) {
      int32_t idir, tile_idx;

      if ((m_cell_count != src->m_cell_count) ||
          (m_tile_count != src->m_tile_count)) {
        return -1;
      }

      for (tile_idx=0; tile_idx<m_tier_tile_count[0]; tile_idx++) {
        for (idir=0; idir<6; idir++) {
          tileSupport_tier0(idir, cell, tile_idx, src->tileSupport_tier0(idir, cell, tile_idx));
        }
      }

      for (tile_idx=0; tile_idx<m_tier_tile_count[1]; tile_idx++) {
        for (idir=0; idir<6; idir++) {
          tileSupport_tier1(idir, cell, tile_idx, src->tileSupport_tier1(idir, cell, tile_idx));
        }
      }


      for (tile_idx=0; tile_idx<m_tier_tile_count[2]; tile_idx++) {
        for (idir=0; idir<6; idir++) {
          tileSupport_tier2(idir, cell, tile_idx, src->tileSupport_tier2(idir, cell, tile_idx));
        }
      }

      for (tile_idx=0; tile_idx<m_tier_tile_count[3]; tile_idx++) {
        for (idir=0; idir<6; idir++) {
          tileSupport_tier3(idir, cell, tile_idx, src->tileSupport_tier3(idir, cell, tile_idx));
        }
      }

      for (tile_idx=0; tile_idx<m_tier_tile_count[4]; tile_idx++) {
        for (idir=0; idir<6; idir++) {
          tileSupport_tier4(idir, cell, tile_idx, src->tileSupport_tier4(idir, cell, tile_idx));
        }
      }

      for (tile_idx=0; tile_idx<m_tier_tile_count[5]; tile_idx++) {
        for (idir=0; idir<6; idir++) {
          tileSupport_tier5(idir, cell, tile_idx, src->tileSupport_tier5(idir, cell, tile_idx));
        }
      }

      return 0;
    }

    int _setCell(ac4_tier6_m1_t *src, int64_t cell) {
      int32_t idir, tile;

      if ((m_cell_count != src->m_cell_count) ||
          (m_tile_count != src->m_tile_count)) {
        return -1;
      }

      for (tile=0; tile<m_tile_count; tile++) {
        for (idir=0; idir<6; idir++) {
          tileSupport(idir, cell, tile, src->tileSupport(idir, cell, tile));
        }
      }

      return 0;
    }

    //---
    //---

    int32_t tileSupport_tier0(int32_t idir, int64_t cell, int32_t tile_idx, int32_t val=-1) {
      int64_t c8, c;
      uint8_t u8, p, mask;

      c = ((6*((cell*m_tier_tile_count[0]) + tile_idx)) + idir);
      c8  = c/8;
      p   = c%8;

      u8 = m_tier_group[0][c8];

      mask = ((uint8_t)1 << p);

      if (val < 0) { return (u8 & mask) ? 1 : 0; }
      if (val > 1) { return -1; }

      if      (val == 0)  { u8 &= ~mask; }
      else if (val == 1)  { u8 |=  mask; }

      m_tier_group[0][c8] = u8;

      return val;
    }

    int32_t tileSupport_tier1(int32_t idir, int64_t cell, int32_t tile_idx, int32_t val=-1) {
      int64_t c8, c;
      uint8_t u8, v8, p, mask, bshift;

      c = ((6*((cell*m_tier_tile_count[1]) + tile_idx)) + idir);
      c8  = c/4;
      p   = c%4;

      u8 = m_tier_group[1][c8];

      bshift = 2*p;

      mask = ((uint8_t)0x3 << bshift);

      if (val < 0) { return ((u8 & mask) >> bshift); }
      if (val > 3) { return -1; }

      v8 = (uint8_t)(val&0x3);
      v8 = (v8 << bshift);

      // zero out two bits where val will go
      // and add shifted 2 bit val in
      //
      u8 &= ~mask;
      u8 = u8 + v8;

      m_tier_group[1][c8] = u8;

      return val;
    }

    int32_t tileSupport_tier2(int32_t idir, int64_t cell, int32_t tile_idx, int32_t val=-1) {
      int64_t c8, c;
      uint8_t u8, v8, p, mask, bshift;

      c = ((6*((cell*m_tier_tile_count[2]) + tile_idx)) + idir);
      c8  = c/2;
      p   = c%2;

      u8 = m_tier_group[2][c8];

      bshift = 4*p;

      mask = ((uint8_t)0xf << bshift);

      if (val < 0) { return ((u8 & mask) >> bshift); }
      if (val > 15) { return -1; }

      v8 = (uint8_t)(val&0xf);
      v8 = (v8 << bshift);

      // zero out two bits where val will go
      // and add shifted 2 bit val in
      //
      u8 &= ~mask;
      u8 = u8 + v8;

      m_tier_group[2][c8] = u8;

      return val;
    }


    int32_t tileSupport_tier3(int32_t idir, int64_t cell, int32_t tile_idx, int32_t val=-1) {
      int64_t pos;

      pos = (6*((cell*m_tier_tile_count[3]) + tile_idx)) + idir;

      if (val < 0) { return m_tier_group[3][pos]; }
      if (val > 255) { return -1; }
      m_tier_group[3][pos] = val;
      return val;
    }

    int32_t tileSupport_tier4(int32_t idir, int64_t cell, int32_t tile_idx, int32_t val=-1) {
      int64_t pos;
      uint16_t *v;

      pos = 2*((6*((cell*m_tier_tile_count[4]) + tile_idx)) + idir);

      v = (uint16_t *)(&(m_tier_group[4][pos]));
      if (val < 0) { return *v; }
      if (val > 65535) { return -1; }
      *v = (uint16_t)val;
      return val;
    }

    int32_t tileSupport_tier5(int32_t idir, int64_t cell, int32_t tile_idx, int32_t val=-1) {
      int64_t pos, ttc = 0;
      uint64_t *v;

      ttc = m_tier_tile_count[5];
      pos = 8*((6*((cell*ttc) + tile_idx)) + idir);

      v = (uint64_t *)(&(m_tier_group[5][pos]));
      if (val < 0) { return *v; }
      *v = (uint64_t)val;
      return val;
    }

    int32_t tileSupport(int32_t idir, int64_t cell, int32_t tile, int32_t val=-1) {
      int32_t tier, tile_idx;

      tier      = m_tile_tier_offset[2*tile];
      tile_idx  = m_tile_tier_offset[2*tile+1];

      switch (tier) {
        case 0: return tileSupport_tier0(idir, cell, tile_idx, val); break;
        case 1: return tileSupport_tier1(idir, cell, tile_idx, val); break;
        case 2: return tileSupport_tier2(idir, cell, tile_idx, val); break;
        case 3: return tileSupport_tier3(idir, cell, tile_idx, val); break;
        case 4: return tileSupport_tier4(idir, cell, tile_idx, val); break;
        case 5: return tileSupport_tier5(idir, cell, tile_idx, val); break;
        default: return -1; break;
      }

      return -1;
    }

    void print(void) {
      int32_t idir, tile;
      int64_t cell;
      int32_t fold_w=32;
      int i;

      printf("m_state: %i\n", (int)m_state);
      printf("m_tile_init: %i\n", (int)m_tile_init);
      printf("m_cell_count: %i\n", (int)m_cell_count);
      printf("m_tile_count: %i\n", (int)m_tile_count);
      printf("m_tier_bit_size[%i]: {", (int)m_tier_bit_size.size());
      for (i=0; i<4; i++) {
        if (i>0) { printf(", "); }
        printf("%i", m_tier_bit_size[i]);
      }
      printf("}\n");
      printf("\n");

      for (idir=0; idir<6; idir++) {
        printf("==== idir:%i ====\n", (int)idir);
        for (tile=0; tile<m_tile_count; tile++) {

          for (cell=0; cell<m_cell_count; cell++) {
            if ((cell%fold_w)==0) { printf("\n "); }
            printf(" %i", (int)tileSupport(idir, cell, tile));
          }
          printf("\n");
        }
        printf("\n");
      }
      printf("\n");

    }

    int64_t size_estimate(void) {
      int i;
      int64_t sz=0;
      for (i=0; i<m_tier_group.size(); i++) {
        sz += m_tier_group[i].size();
      }
      return sz;
    }

};


#endif
