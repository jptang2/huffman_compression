// FileTree.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "main.h"
#include "MainDialog.h"

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/, int nCmdShow)
{
	//检测内存泄露
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	

	CPaintManagerUI::SetInstance(hInstance);

	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() );//+ _T("skin")
	CPaintManagerUI::SetResourceZip(_T("skin.zip"));

//	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("skin"));
	
	HRESULT Hr = ::CoInitialize(NULL);
	if( FAILED(Hr) ) return 0;
	
	MainDialog* pFrame = new MainDialog();
	if( pFrame == NULL ) 
		return 0;

	pFrame->Create(NULL, _T("Huffman"), 
		UI_WNDSTYLE_FRAME,
		UI_WNDSTYLE_EX_FRAME, 0, 0, 300, 500);

	pFrame->CenterWindow();
	::ShowWindow(*pFrame, SW_SHOW);
	CPaintManagerUI::MessageLoop();
	
	::CoUninitialize();
	SAFE_DELETE(pFrame);	
	return 0;
}