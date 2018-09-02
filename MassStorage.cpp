/*
 *--------------------------------------------------------------------
 * Project:     VM65 - Virtual Machine/CPU emulator programming
 *                     framework.  
 *
 * File:        MassStorage.cpp
 *
 * Purpose:     Implementation of MassStorage class.
 *              To emulate disk-like mass storage device for data.
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

#include <stdio.h>
#include "MassStorage.h"
#include "MKGenException.h"
#include "system.h"

namespace MKBasic {

constexpr int MassStorage::sect_per_track[];

/*
 *--------------------------------------------------------------------
 * Method:    MassStorage() 
 * Purpose:   Class default constructor.
 * Arguments: 
 * Returns:   
 *--------------------------------------------------------------------
 */
MassStorage::MassStorage()
{
  Initialize();
}

 /*
  *--------------------------------------------------------------------
  * Method:     ~MassStorage()
  * Purpose:    Class destructor.
  * Arguments: 
  * Returns:   
  *--------------------------------------------------------------------
  */
MassStorage::~MassStorage()
{

}

 /*
  *--------------------------------------------------------------------
  * Method:     Initialize()
  * Purpose:    Initialize internal variables.
  * Arguments: 
  * Returns:   
  *--------------------------------------------------------------------
  */
void MassStorage::Initialize()
{
  // TO DO: add code to initialize class

  // initialize standard disk images
  DiskTrack dtrk;
  DiskImage ditbl[] = {
    {0, "DISK0", {dtrk}},
    {1, "DISK1", {dtrk}},
    {2, "DISK2", {dtrk}},
    {3, "DISK3", {dtrk}},
    {4, "DISK4", {dtrk}},
    {5, "DISK5", {dtrk}},
    {6, "DISK6", {dtrk}},
    {7, "DISK7", {dtrk}},
    {8, "DISK8", {dtrk}},
    {9, "DISK9", {dtrk}}
  };
  for (int i=0; i < num_of_images; i++) {
    mDiskImages[i].id = ditbl[i].id;
    mDiskImages[i].name = ditbl[i].name;
    string fname = mDiskImages[i].name + "_" + to_string(mDiskImages[i].id) + ".disk";
    FILE *fp = fopen(fname.c_str(), "r");
    if (NULL == fp) {
      Format(mDiskImages[i].id, mDiskImages[i].name);
      Flush(mDiskImages[i].id);
    } else {
      LoadFromFile(mDiskImages[i].id, mDiskImages[i].name);
      fclose(fp);
    }
  }
}

 /*
  *--------------------------------------------------------------------
  * Method:     Format()
  * Purpose:    Format the disk image.
  * Arguments:  id - the number / id of the disk image to format
  *             name - the new name of the disk image
  * Returns:    int - 0 if OK, less than 0 if error.
  *--------------------------------------------------------------------
  */
int MassStorage::Format (int id, string name)
{
  int ret = 0;

  // TO DO: add code to format disk image
  DiskImage dimg;
  dimg.name = name;
  for (int track=0; track < max_numof_tracks; track++) {
    dimg.data[track].num = track+1;
    dimg.data[track].num_of_sectors = sect_per_track[track];
    if (track+1 == 18) { // BAM + directory
      dimg.data[track].data[0x00] = 0x12; // point to track 18
      dimg.data[track].data[0x01] = 0x01; // and sector 1 (directory)
      dimg.data[track].data[0x02] = 0x41; // DOS version
      dimg.data[track].data[0x03] = 0x01; // unused
      // BAM
      for (int sector = 0x04; sector < 0x90; sector += 4) {
        dimg.data[track].data[sector] = dimg.data[track].num_of_sectors;
        dimg.data[track].data[sector+1] = 0xff;
        dimg.data[track].data[sector+2] = 0xff;
        dimg.data[track].data[sector+3] = ~(0xff << (dimg.data[track].num_of_sectors - 16));
      }
    } else {

    }
  }

  return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:    Flush()
 * Purpose:   Save image data to disk file.
 * Arguments: id - the disk image id
 * Returns:   int - 0 if OK, less than 0 if error
 *--------------------------------------------------------------------
 */
int MassStorage::Flush (int id)
{
  int ret = 0;

  // TO DO: add code to flush disk image to file

  return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:    ReadSectorData()
 * Purpose:   Read data from disk's image sector.
 * Arguments: id - disk image id
 *            track - track#
 *            sector - sector#
 * Returns:   pointer to data
 *--------------------------------------------------------------------
 */
unsigned char * MassStorage::ReadSectorData(int id, 
                                            int track, 
                                            int sector)
{
  // TO DO: add code to read data

  return mSectorBuf;
}

/*
 *--------------------------------------------------------------------
 * Method:    WriteSectorData()
 * Purpose:   Write data to disk's image sector.
 * Arguments: id - the disk image id
 *            track - track#
 *            sector - sector#
 *            buf - pointer to data
 * Returns:   int - 0 if OK, less than 0 if error
 *--------------------------------------------------------------------
 */
int MassStorage::WriteSectorData (int id, 
                                  int track, 
                                  int sector, 
                                  unsigned char *buf)
{
  int ret = 0;

  // TO DO: add code to write sector data

  return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:    LoadFromFile()
 * Purpose:   Load image data from file.
 * Arguments: id - the disk image id
 *            name - disk image name
 * Returns:   int - 0 if OK, less than 0 if error
 *--------------------------------------------------------------------
 */
int  MassStorage::LoadFromFile(int id, string name)
{
  int ret = 0;

  return ret;
}

} // namespace MKBasic