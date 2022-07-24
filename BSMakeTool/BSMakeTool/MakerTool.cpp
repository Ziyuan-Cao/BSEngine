#include "MakerTool.h"
#include "BGraphics.h"
//

namespace BSMakeTool {

	MakerTool::MakerTool()
	{
		InitializeComponent();
		//
		//TODO: Add the constructor code here
		//
		HWND RenderingHWND = (HWND)Rendering_Panel->Handle.ToPointer();
		DX_App* DXApp = new DX_App(RenderingHWND);
		
		DXApp->Initialize(Rendering_Panel->Width, Rendering_Panel->Height);

		
	}

}