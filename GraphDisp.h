/*
 *--------------------------------------------------------------------
 * Project:     VM65 - Virtual Machine/CPU emulator programming
 *                     framework.  
 *
 * File:   			GraphDisp.h
 *
 * Purpose: 		Prototype of GraphDisp class and supporting data
 *							structures, enumerations, constants and macros.
 *
 * Date:      	8/25/2016
 *
 * Copyright:  (C) by Marek Karcz 2016. All rights reserved.
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
#ifndef GRAPHDISP_H
#define GRAPHDISP_H

/*
 * Rudimentary emulation of generic raster graphics RGB display device.
 */

#include <thread>
#include <SDL.h>

// defaults
#define GRDISP_VR_X	320
#define GRDISP_VR_Y	200
#define CHROM_8x8_SIZE	2048

using namespace std; 

namespace MKBasic {

const int GRAPHDISP_MAXW = 960; // "real" display width or maximum virtual width
const int GRAPHDISP_MAXH = 600; // "real" display width or maximum virtual height

class GraphDisp {

	public:

		bool mContLoop;	// = true;
		bool mMainLoopActive;	// = false;		

		GraphDisp();
		GraphDisp(int width, int height);
		~GraphDisp();
		void Start(GraphDisp *pgd);
		void Stop();
		void SetPixel(int x, int y);
		void ErasePixel(int x, int y);
		void DrawLine(int x1, int y1, int x2, int y2);
		void EraseLine(int x1, int y1, int x2, int y2);
		void SetBgColor(int r, int g, int b);
		void SetFgColor(int r, int g, int b);
		void Update();
		void ReadEvents();
		void ClearScreen();
		//void MainLoop();
		bool IsMainLoopActive();
		void PrintChar8x8(int code, int col, int row, bool reversed);
		void CopyCharRom8x8(unsigned char *pchrom);

	private:

		int	mWidth;								// virtual display width
		int mHeight;							// virtual display height
		int mPixelSizeX;					// virtual pixel width
		int mPixelSizeY;					// virtual pixel height
		int mWinPosX;							// SDL window position coordinate X
		int mWinPosY;							// SDL window position coordinate Y
		int mBgRgbR;							// bg color, RGB red intensity
		int mBgRgbG;							// bg color, RGB green intensity
		int mBgRgbB;							// bg color, RGB blue intensity
		int mFgRgbR;							// fg color, RGB red intensity
		int mFgRgbG;							// fg color, RGB green intensity
		int mFgRgbB;							// fg color, RGB blue intensity		
		SDL_Window *mpWindow;
		SDL_Surface *mpSurface;
		SDL_Renderer *mpRenderer;
		thread mMainLoopThread;
		unsigned char mCharROM8x8[CHROM_8x8_SIZE];

		void Initialize();
		void UpdateSurface();
		void Clear();
		void GetDesktopResolution(int& horizontal, int& vertical);
		void DrawLine(int x1, int y1, int x2, int y2, bool draworerase);
		void RenderPixel(int x, int y, bool set);
		void RenderChar8x8(unsigned char chdef[8], int x, int y, bool reversed);

}; // class GraphDisp

} // namespace MKBasic

#endif
