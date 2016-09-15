/*
 *--------------------------------------------------------------------
 * Project:     VM65 - Virtual Machine/CPU emulator programming
 *                     framework.  
 *
 * File:   			GraphDisp.cpp
 *
 * Purpose: 		Implementation of GraphDisp class.
 *							The GraphDisp class emulates the graphics raster
 *							device. It handles displaying of the graphics, the
 *							events loop and the 'hardware' API to the device.
 *							The higher level abstraction layer - MemMapDev
 *							defines the actual behavior/response of the emulated
 *							device in the emulated system.
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
#include "GraphDisp.h"
#include "MKGenException.h"
#include "system.h"

#if defined(WINDOWS)
#include "wtypes.h"
#endif

namespace MKBasic {

/*
 *--------------------------------------------------------------------
 * Method:		GraphDisp()
 * Purpose:		Class default constructor.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
GraphDisp::GraphDisp()
{
	Initialize();
}

/*
 *--------------------------------------------------------------------
 * Method:		GraphDisp()
 * Purpose:		Class custom constructor.
 * Arguments:	width, height - integer, graphical display dimensions
 *														(virtual dimensions as opposed to
 *														 to actual dimensions GRAPHDISP_MAXW,
 *														 GRAPHDISP_MAXH)
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
GraphDisp::GraphDisp(int width, int height)
{
	mWidth = width;
	mHeight = height;
	Initialize();
}

/*
 *--------------------------------------------------------------------
 * Method:		~GraphDisp()
 * Purpose:		Class destructor.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
GraphDisp::~GraphDisp()
{
	mContLoop = false;
	while (mMainLoopActive);
	SDL_DestroyRenderer(mpRenderer);
	SDL_DestroyWindow(mpWindow);
	SDL_Quit();
}

/*
 *--------------------------------------------------------------------
 * Method:		Initialize()
 * Purpose:		Initialize class members, objects.
 * Arguments:	n/a
 * Returns:		n/a
 * Problems:
 *    SDL_CreateWindow with flags:
 *    SDL_WINDOW_SHOWN|SDL_WINDOW_BORDERLESS|SDL_WINDOW_RESIZABLE
 *    creates window with no title/icon/system buttons, but with
 *    a frame, which can be used to resize it. The problem is, when
 *    it is resized, it stops updating correctly.
 *--------------------------------------------------------------------
 */
void GraphDisp::Initialize()
{
	int desk_w, desk_h, winbd_top = 5, winbd_right = 5;

	mContLoop = true;
	mMainLoopActive = false;		

	mWidth 	= GRDISP_VR_X;			// virtual display width
	mHeight = GRDISP_VR_Y;		// virtual display height
	
	mPixelSizeX = GRAPHDISP_MAXW / mWidth;
	mPixelSizeY = GRAPHDISP_MAXH / mHeight;

	mWinPosX = 0;			// SDL window position coordinate X
	mWinPosY = 0;			// SDL window position coordinate Y
	mBgRgbR = 0;			// bg color, RGB red intensity
	mBgRgbG = 0;			// bg color, RGB green intensity
	mBgRgbB = 0;			// bg color, RGB blue intensity
	mFgRgbR = 0xFF;		// fg color, RGB red intensity
	mFgRgbG = 0xFF;		// fg color, RGB green intensity
	mFgRgbB = 0xFF;		// fg color, RGB blue intensity		

	mpWindow = NULL;
	mpSurface = NULL;
	mpRenderer = NULL;

	GetDesktopResolution(desk_w, desk_h);
	// Available in version > 2.0.4
	//SDL_GetWindowBordersSize(mpWindow, &winbd_top, NULL, NULL, &winbd_right);
	mWinPosX = desk_w - GRAPHDISP_MAXW - winbd_right;
	mWinPosY = winbd_top;

	SDL_Init(SDL_INIT_VIDEO);

	mpWindow = SDL_CreateWindow(
		"GraphDisp", 
		mWinPosX,
		mWinPosY,
		GRAPHDISP_MAXW, 
		GRAPHDISP_MAXH, 
		SDL_WINDOW_SHOWN|SDL_WINDOW_BORDERLESS|SDL_WINDOW_RESIZABLE
		);

	if (NULL == mpWindow) {
		throw MKGenException(SDL_GetError());
	}

  // Get window surface
  mpSurface = SDL_GetWindowSurface(mpWindow);

	// Create renderer for window
	mpRenderer = SDL_CreateRenderer(mpWindow, -1, SDL_RENDERER_SOFTWARE);
  //mpRenderer = SDL_GetRenderer(mpWindow);	

	if (NULL == mpRenderer) {
		throw MKGenException(SDL_GetError());
	}	  

  Clear();

}

/*
 *--------------------------------------------------------------------
 * Method:		ClearScreen()
 * Purpose:		Clear the surface. Update screen.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::ClearScreen()
{
	Clear();
  UpdateSurface();	
}

/*
 *--------------------------------------------------------------------
 * Method:		Clear()
 * Purpose:		Clear the surface.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::Clear()
{
  //Fill the surface with background color
  SDL_FillRect(mpSurface, NULL, SDL_MapRGB(mpSurface->format, 
  																				 mBgRgbR, 
  																				 mBgRgbG, 
  																				 mBgRgbB));
}

/*
 *--------------------------------------------------------------------
 * Method:		UpdateSurface()
 * Purpose:		Update window surface.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::UpdateSurface()
{
  //Update the surface
  SDL_UpdateWindowSurface(mpWindow);
}

/*
 *--------------------------------------------------------------------
 * Method:		Update()
 * Purpose:		Update window surface (public).
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::Update()
{
  //Update the surface
  UpdateSurface();
}

/*
 *--------------------------------------------------------------------
 * Method:		RenderPixel()
 * Purpose:		Set or unset pixel (scaled) on graphics display
 * 						surface.
 * Arguments:	x, y - integer, virtual pixel coordinates
 *						set - boolean, set or unset pixel
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::RenderPixel(int x, int y, bool set)
{
	SDL_Rect rtf;

	rtf.x = x * mPixelSizeX; rtf.y = y * mPixelSizeY;
	rtf.w = mPixelSizeX;
	rtf.h = mPixelSizeY;

	int rgb_r = 0, rgb_g = 0, rgb_b = 0;

	if (set) {
		rgb_r = mFgRgbR;
		rgb_g = mFgRgbG;
		rgb_b = mFgRgbB;
	} else {
		rgb_r = mBgRgbR;
		rgb_g = mBgRgbG;
		rgb_b = mBgRgbB;
	}

	// set or unset pixel
	SDL_FillRect(mpSurface, &rtf, SDL_MapRGB(mpSurface->format, rgb_r, rgb_g, rgb_b));

  //Update the surface
  SDL_UpdateWindowSurface(mpWindow);	
}

/*
 *--------------------------------------------------------------------
 * Method:    RendedChar8x8()
 * Purpose:   Draw 8x8 character from its pixel definition.	
 * Arguments: chdef - character definition in 8x8 bit matrix
 *						x, y - coordinates
 *						reversed - reversed (true) or normal (false)
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::RenderChar8x8(unsigned char chdef[8], int x, int y, bool reversed)
{
	SDL_Rect rtf;

	int rgb_r = 0, rgb_g = 0, rgb_b = 0;	
	for (int yy = y, j=0; j < 8; j++, yy++) {
		unsigned char chd = chdef[j];
		for (int xx = x, i=0; i < 8; i++, xx++) {
			bool pixset = (chd & 0x80) == 0x80;
			if (reversed) pixset = !pixset;
			rtf.x = xx * mPixelSizeX; rtf.y = yy * mPixelSizeY;
			rtf.w = mPixelSizeX;
			rtf.h = mPixelSizeY;			
			if (pixset) {
				rgb_r = mFgRgbR;
				rgb_g = mFgRgbG;
				rgb_b = mFgRgbB;
			} else {
				rgb_r = mBgRgbR;
				rgb_g = mBgRgbG;
				rgb_b = mBgRgbB;
			}
			SDL_FillRect(mpSurface, &rtf, SDL_MapRGB(mpSurface->format, rgb_r, rgb_g, rgb_b));			
			chd = chd << 1; chd &= 0xFE;
		}
	}
	SDL_UpdateWindowSurface(mpWindow);
}

/*
 *--------------------------------------------------------------------
 * Method:    CopyCharRom8x8()
 * Purpose:   Copy provided 8x8 characters table to internal buffer.
 * Arguments: pchrom - pointer to characters defintions table
 * Returns:   
 *--------------------------------------------------------------------
 */
void GraphDisp::CopyCharRom8x8(unsigned char *pchrom)
{
	for (int i=0; i<CHROM_8x8_SIZE; i++) {
		mCharROM8x8[i] = pchrom[i];
	}
}

/*
 *--------------------------------------------------------------------
 * Method:    PrintChar8x8()
 * Purpose:   Print 8x8 character at specified row and column.
 * Arguments: code - character code
 *            col, row - character coordinates in 8 pixel intervals
 *            reversed - color mode (reversed or nornal)
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::PrintChar8x8(int code, int col, int row, bool reversed)
{
	int x = col * 8;
	int y = row * 8;
	int n = code * 8;
	unsigned char chdef[8];
	for (int i=0; i<8; i++) {
		chdef[i] = mCharROM8x8[n+i];
	}
	RenderChar8x8(chdef, x, y, reversed);
}

/*
 *--------------------------------------------------------------------
 * Method:		SetPixel()
 * Purpose:		Set pixel (scaled) on graphics display surface.
 * Arguments: x, y - pixel coordinates
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::SetPixel(int x, int y)
{
	RenderPixel(x, y, true);
}

/*
 *--------------------------------------------------------------------
 * Method:		ErasePixel()
 * Purpose:		Unset (erase) pixel (paint with BG color) on graphics
 *            display surface.
 * Arguments: x, y - pixel coordinates
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::ErasePixel(int x, int y)
{
	RenderPixel(x, y, false);
}

/*
 *--------------------------------------------------------------------
 * Method:		DrawLine()
 * Purpose:		Draw or erase line between specified points.
 * Arguments:	x1, y1 - coordinates of first point
 *            x2, y2 - coordinates of second point
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::DrawLine(int x1, int y1, int x2, int y2, bool draworerase)
{
	int colR = mFgRgbR, colG = mFgRgbG, colB = mFgRgbB;

	if (false == draworerase) {
		colR = mBgRgbR;
		colG = mBgRgbG;
		colB = mBgRgbB;
	}

	SDL_SetRenderDrawColor(mpRenderer, colR, colG, colB, SDL_ALPHA_OPAQUE);
	SDL_RenderSetLogicalSize(mpRenderer, mWidth, mHeight);
	SDL_RenderSetScale(mpRenderer, mPixelSizeX, mPixelSizeY);
	SDL_RenderDrawLine(mpRenderer, x1, y1, x2, y2);
	SDL_RenderPresent(mpRenderer);
}

/*
 *--------------------------------------------------------------------
 * Method:		DrawLine()
 * Purpose:		Draw line between specified points.
 * Arguments:	x1, y1 - coordinates of first point
 *            x2, y2 - coordinates of second point
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::DrawLine(int x1, int y1, int x2, int y2)
{
	DrawLine(x1, y1, x2, y2, true);
}

/*
 *--------------------------------------------------------------------
 * Method:		EraseLine()
 * Purpose:		Erase line between specified points (draw with BG color)
 * Arguments:	x1, y1 - coordinates of first point
 *            x2, y2 - coordinates of second point
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::EraseLine(int x1, int y1, int x2, int y2)
{
	DrawLine(x1, y1, x2, y2, false);
}

/*
 *--------------------------------------------------------------------
 * Method:		SetBgColor()
 * Purpose:		Set background color.
 * Arguments:	r, g, b - integer, red, green and blue intensities
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::SetBgColor(int r, int g, int b)
{
	mBgRgbR = r;
	mBgRgbG = g;
	mBgRgbB = b;
}

/*
 *--------------------------------------------------------------------
 * Method:		SetFgColor()
 * Purpose:		Set foreground color.
 * Arguments:	r, g, b - integer, red, green and blue intensities
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::SetFgColor(int r, int g, int b)
{
	mFgRgbR = r;
	mFgRgbG = g;
	mFgRgbB = b;
}

/*
 *--------------------------------------------------------------------
 * Method:		MainLoop()
 * Purpose:		The main loop to process SDL events and update window.
 *            This is a global function meant to run in a thread.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void MainLoop(GraphDisp *pgd)
{
	pgd->mMainLoopActive = true;
	while (pgd->mContLoop) {
		pgd->ReadEvents();
		pgd->Update();
  }
  pgd->mMainLoopActive = false;
}

/*
 *--------------------------------------------------------------------
 * Method:		Start()
 * Purpose:		Starts MainLoop in a thread.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::Start(GraphDisp *pgd)
{
	mMainLoopThread = thread(MainLoop,pgd);
}

/*
 *--------------------------------------------------------------------
 * Method:		Stop()
 * Purpose:		Stop MainLoop thread.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::Stop()
{
	mContLoop = false;
	mMainLoopThread.join();
}

/*
 *--------------------------------------------------------------------
 * Method:		
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
bool GraphDisp::IsMainLoopActive()
{
	return mMainLoopActive;
}

/*
 *--------------------------------------------------------------------
 * Method:		ReadEvents()
 * Purpose:		Read events and perform actions when needed.	
 * Arguments:	n/a
 * Returns:		n/a
 * Problems:
 *    By blitting (copying) surface I wanted to avoid loosing already
 *    drawn pixels during windows resize, but this doesn't work.
 *--------------------------------------------------------------------
 */
void GraphDisp::ReadEvents()
{
		SDL_Event e;
		SDL_Surface *pTmpSurface;

		while (SDL_PollEvent(&e)) {

			switch (e.type) {

				case SDL_QUIT:
					mContLoop = false;
					break;

				case SDL_WINDOWEVENT:
					if (SDL_WINDOWEVENT_RESIZED == e.window.event) {

						pTmpSurface = SDL_CreateRGBSurface(0, 
																							 mpSurface->w,
																							 mpSurface->h,
																							 mpSurface->format->BitsPerPixel,
                                  						 mpSurface->format->Rmask, 
                                  						 mpSurface->format->Gmask,
                                  						 mpSurface->format->Bmask, 
                                  						 mpSurface->format->Amask);
						SDL_SetWindowSize(mpWindow, GRAPHDISP_MAXW, GRAPHDISP_MAXH);
						mpSurface = SDL_GetWindowSurface(mpWindow);
						SDL_SetWindowPosition(mpWindow, mWinPosX, mWinPosY);
						SDL_SetSurfaceAlphaMod(pTmpSurface, 0);
						SDL_BlitSurface(pTmpSurface, 0, mpSurface, 0);						
						UpdateSurface();
						SDL_FreeSurface(pTmpSurface);						

					} else if (SDL_WINDOWEVENT_FOCUS_GAINED == e.window.event) {

						SDL_RaiseWindow(mpWindow);
						
					}
					break;

				default:
					break;
			}

		}
}

/*
 *--------------------------------------------------------------------
 * Method:		
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void GraphDisp::GetDesktopResolution(int& horizontal, int& vertical)
{
#if defined(WINDOWS)	
   RECT desktop;
   // Get a handle to the desktop window
   const HWND hDesktop = GetDesktopWindow();
   // Get the size of screen to the variable desktop
   GetWindowRect(hDesktop, &desktop);
   // The top left corner will have coordinates (0,0)
   // and the bottom right corner will have coordinates
   // (horizontal, vertical)
   horizontal = desktop.right;
   vertical = desktop.bottom;
#else
   horizontal = GRAPHDISP_MAXW;
   vertical = GRAPHDISP_MAXH;
#endif   
}

} // namespace MKBasic
