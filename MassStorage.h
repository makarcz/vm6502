/*
 *--------------------------------------------------------------------
 * Project:     VM65 - Virtual Machine/CPU emulator programming
 *                     framework.  
 *
 * File:        MassStorage.h
 *
 * Purpose:     Prototype of MassStorage class and supporting data
 *              structures, constants, enumerations and macros.
 *
 * Date:        8/16/2017
 *
 * Copyright:  (C) by Marek Karcz 2016, 2017. All rights reserved.
 *
 * Contact:    makarcz@yahoo.com
 *
 * License Agreement and Warranty:

   This software is provided with No Warranty.
   I (Marek Karcz) will not be held responsible for any damage to
   computer systems, data or user's health resulting from use.
   Please proceed responsibly and apply common sense.
   This software is provided in hope that it will be useful.
   It is free of charge for non-commercial and educational use.
   Distribution of this software in non-commercial and educational
   derivative work is permitted under condition that original
   copyright notices and comments are preserved. Some 3-rd party work
   included with this project may require separate application for
   permission from their respective authors/copyright owners.

 *--------------------------------------------------------------------
 */
#ifndef MASSSTORAGE_H
#define MASSSTORAGE_H

#include <string>

using namespace std;

namespace MKBasic {

   class MassStorage {

      public:

         // CBM 1541 format
         static const int max_numof_sectors   = 21;
         static const int sector_size         = 256;
         static const int max_numof_tracks    = 35;
         static const int num_of_images       = 10;
         constexpr static int sect_per_track[]  = 
               {21, 21, 21, 21, 21, 21, 21,
                21, 21, 21, 21, 21, 21, 21,
                21, 21, 21,
                19, 19, 19, 19, 19, 19, 19,
                18, 18, 18, 18, 18, 18,
                17, 17, 17, 17, 17};

         MassStorage();
         ~MassStorage();
         int Format (int id, string name);  // format disk image
         int Flush (int id);                // flush disk image to hard drive
         unsigned char *ReadSectorData(int id, 
                                       int track, 
                                       int sector);  // read data from sector
         int WriteSectorData (int id, 
                              int track, 
                              int sector, 
                              unsigned char *buf); // write data to sector

      private:

         // definition of a disk track structure
         struct DiskTrack {

            int num;
            int num_of_sectors;
            unsigned char data[max_numof_sectors * sector_size];

         };

         // definition of a disk image structure
         struct DiskImage {

            int         id;
            string      name;
            DiskTrack   data[max_numof_tracks];
         };         

         unsigned char  mSectorBuf[sector_size];        // buffer for sector data
         DiskImage      mDiskImages[num_of_images];     // buffer for all disk images

         void Initialize();
         int  LoadFromFile(int id, string name);        // load image from disk cache

   }; // class MassStorage

} // namespace MKBasic

#endif
