#ifndef GRAPHDISP_H
#define GRAPHDISP_H

/*
 * Rudimentary emulation of generic raster graphics RGB display device.
 */

#include <thread>
#include <SDL.h>

using namespace std; 

namespace MKBasic {

const int GRAPHDISP_MAXW = 640;	// "real" display width or maximum virtual width
const int GRAPHDISP_MAXH = 400;	// "real" display width or maximum virtual height

class GraphDisp {

	public:

		bool mContLoop = true;
		bool mMainLoopActive = false;		

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

	private:

		int	mWidth = 320;			// virtual display width
		int mHeight = 200;		// virtual display height
		int mPixelSizeX = 2;	// virtual pixel width
		int mPixelSizeY = 2;	// virtual pixel height
		int mWinPosX = 0;			// SDL window position coordinate X
		int mWinPosY = 0;			// SDL window position coordinate Y
		int mBgRgbR = 0;			// bg color, RGB red intensity
		int mBgRgbG = 0;			// bg color, RGB green intensity
		int mBgRgbB = 0;			// bg color, RGB blue intensity
		int mFgRgbR = 0xFF;		// fg color, RGB red intensity
		int mFgRgbG = 0xFF;		// fg color, RGB green intensity
		int mFgRgbB = 0xFF;		// fg color, RGB blue intensity		
		SDL_Window *mpWindow = NULL;
		SDL_Surface *mpSurface = NULL;
		SDL_Renderer *mpRenderer = NULL;
		thread mMainLoopThread;

		void Initialize();
		void UpdateSurface();
		void Clear();
		void GetDesktopResolution(int& horizontal, int& vertical);
		void DrawLine(int x1, int y1, int x2, int y2, bool draworerase);
		void RenderPixel(int x, int y, bool set);		

}; // class GraphDisp

} // namespace MKBasic

#endif