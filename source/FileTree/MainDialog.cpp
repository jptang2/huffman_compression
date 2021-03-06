#include "stdafx.h"
#include "MainDialog.h"
#include "Tree.h"
#include <Commdlg.h>
#include <shellapi.h>
#include <string>
#include <map>
#include <queue>
#include <algorithm>
#include <math.h>

#define symbols 256
#define MAX_WEIGHT 2147483647
typedef vector<bool> Huffman_code;
typedef struct tagHuffman_data
{
	int nfrequency;
	BYTE ch;
	DWORD dwcode;
	Huffman_code vcode;
	
	tagHuffman_data()
	{
		ch = 0;
		vcode.clear();
		nfrequency = 0;
		dwcode = 0;
	}
	tagHuffman_data(BYTE c,Huffman_code code,int n)
	{
		ch = c;
		vcode = code;
		nfrequency = n;
		dwcode = ToBinary(code);
	}

	DWORD ToBinary(vector<bool> vCode)
	{
		DWORD dw = 0;
		int n = vCode.size();
		for (int i=0;i<n;i++)
		{
			if (vCode[i] == true)			
				dw += pow(2,i);			
		}

	
		return dw;
	}
}Huffman_data; 
vector<Huffman_data> vHuffman;

bool TreeCmp(Tree* p1,Tree* p2)
{
	return p1->weight < p2->weight;
}

Tree* GetMinNode(vector<Tree*>& vTree)
{
	vector<Tree*>::iterator it;
	int nMin = MAX_WEIGHT;
	for (it=vTree.begin();it!=vTree.end();it++)
	{
		int nWeight = (*it)->weight;
		if (nWeight < nMin)
		{
			nMin = nWeight;
		}
	}

	for (it=vTree.begin();it!=vTree.end();it++)
	{
		int nWeight = (*it)->weight;
		if (nWeight == nMin)
		{
			Tree* pTree = *it;
			vTree.erase(it);
			return pTree;
		}
	}

	return NULL;
}

Tree* BuildTree(int *frequency)  
{  
	vector<Tree*> QTree;
	for (int i=0;i<symbols;i++)  
	{  
		if(frequency[i])  
			QTree.push_back(new Tree(NULL,NULL,frequency[i],(char)i));                
	}  

	sort(QTree.begin(),QTree.end(),TreeCmp);
		
	while (QTree.size()>1)  
	{  
		Tree* lc = GetMinNode(QTree);		
		Tree* rc = GetMinNode(QTree);
		Tree* parent = new Tree(lc,rc,lc->weight+rc->weight,(char)256);  
		QTree.insert(QTree.begin(),parent);  
	}  

	return QTree[0];  	
}  

void Huffman_Coding(Tree* root, Huffman_code& curcode)  
{  
	if(root->Isleaf())  
	{  
		vHuffman.push_back(Huffman_data(root->ch,curcode,root->weight));			
		return;  
	}  
	Huffman_code lcode = curcode;  
	Huffman_code rcode = curcode;  
	lcode.push_back(false);  
	rcode.push_back(true);  

	Huffman_Coding(root->left,lcode);  
	Huffman_Coding(root->right,rcode);  
}  

int HuffmanFrequencyCmp(Huffman_data p1,Huffman_data p2)
{
	return p1.nfrequency > p2.nfrequency;
}
int HuffmanAsciiCmp(Huffman_data p1,Huffman_data p2)
{
	return p2.ch > p1.ch;
}

DWORD GetCurrentCode(BYTE ch,int& nLenth)
{
	for (int i=0;i<vHuffman.size();i++)
	{
		BYTE by = vHuffman[i].ch;
		if (ch == by)
		{
			nLenth = vHuffman[i].vcode.size();
			return vHuffman[i].dwcode;
		}
	}
	return 0;
}


void MainDialog::Notify(TNotifyUI& msg)
{
	if( msg.sType == _T("windowinit") ) 
		OnPrepare(msg);
	else if( msg.sType == _T("click") ) 
	{		
		if( msg.pSender == m_pCloseBtn ) 
		{			
			SendMessage(WM_DESTROY);		
			return; 
		}
		else if( msg.pSender == m_pMinBtn ) 
		{ 
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
			return; 
		}	
		else if( msg.pSender == m_pRestoreBtn ) 
		{ 
			SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); return; 
		}
		else if( msg.pSender == m_pCompressBtn ) 
		{ 			
			CDuiString strSource = m_pSourceEdit->GetText();			
			string szSource=strSource.GetData();	
			vHuffman.clear();

			errno_t err;
			FILE* pFile = NULL;
			err = fopen_s(&pFile,szSource.c_str(),"rb");
		//	err = fopen_s(&pFile,"r://n.txt","rb");
						
			if (NULL == pFile)
			{
				MessageBox(NULL,"please input source file to compress",_T("error"),MB_OK);
				return;
			}

			fseek(pFile,SEEK_SET,SEEK_END);
			int nFileLength=ftell(pFile);			
			fseek(pFile,SEEK_SET,SEEK_SET);
			BYTE *pFileBuffer = new BYTE[nFileLength];
			fread(pFileBuffer,sizeof(BYTE), nFileLength,pFile);						
			fclose(pFile);
					
			BYTE *pFileTemp = pFileBuffer;
			int freq[symbols] = {0};  	
			for( int n = 0; n < nFileLength; n++)
			{
				freq[*pFileTemp++]++;
			}
								
			Tree* tree = BuildTree(freq);  
			Huffman_code nullcode;  
			nullcode.clear();  
			Huffman_Coding(tree,nullcode);  			

			int nNodeCount = vHuffman.size();
			
			int nNodeSize = sizeof(DWORD)+sizeof(BYTE);
			int nDesLen = nFileLength+nNodeCount*nNodeSize;
			BYTE *pDes = new BYTE[nDesLen];
			BYTE *pDesTemp = pDes;
			memset(pDesTemp, 0, nDesLen);
			*(DWORD*)pDesTemp = nFileLength;
			pDesTemp += sizeof(DWORD);
			
			*pDesTemp = nNodeCount-1;
			pDesTemp += sizeof(BYTE);
				
			sort(vHuffman.begin(),vHuffman.end(),HuffmanFrequencyCmp);
			for(int nCount = 0; nCount < nNodeCount; nCount++)
			{	
				memcpy(pDesTemp, &vHuffman[nCount], nNodeSize);
				pDesTemp += nNodeSize;
			}

			sort(vHuffman.begin(),vHuffman.end(),HuffmanAsciiCmp);
			
			int nIndex = 0;		
			for(int i = 0; i < nFileLength; i++)
			{				
				int nLenth=0;
				DWORD code = GetCurrentCode(pFileBuffer[i],nLenth);
				*(DWORD*)(pDesTemp+(nIndex>>3)) |= code << (nIndex&7);		
				nIndex += nLenth;
			}
			nDesLen = (pDesTemp-pDes)+(nIndex+7)/8;

			CDuiString strCompress = m_pCompressEdit->GetText();			
			string szCompress=strCompress.GetData();
			FILE* pFileToSave = NULL;
			err = fopen_s(&pFile,szCompress.c_str(),"wb");
		//	err = fopen_s(&pFile,"r://n2.txt","wb");
			fwrite(pDes,nDesLen,1,pFile);
			fclose(pFile);

			SAFE_DELETE(tree);
			SAFE_DELETE(pDes);
			SAFE_DELETE(pFileBuffer);
			vHuffman.clear();

			CDuiString str;
			str.Format("compressed file saved to %s",szCompress.c_str());
			MessageBox(NULL,str ,_T("done"),MB_OK);

			return; 
		}		
		else if( msg.pSender == m_pDecompressBtn )  
		{ 					
			CDuiString strInput = m_pCompressEdit->GetText();			
			string szCompress=strInput.GetData();	
			vHuffman.clear();

			errno_t err;
			FILE* pFile = NULL;
			err = fopen_s(&pFile,szCompress.c_str(),"rb");
		//	err = fopen_s(&pFile,"r://n2.txt","rb");

			if (NULL == pFile)
			{
				MessageBox(NULL,"please input source file to decompress",_T("error"),MB_OK);
				return;
			}

			fseek(pFile,SEEK_SET,SEEK_END);
			int nFileLength=ftell(pFile);			
			fseek(pFile,SEEK_SET,SEEK_SET);
			BYTE *pFileBuffer = new BYTE[nFileLength];
			fread(pFileBuffer,sizeof(BYTE), nFileLength,pFile);						
			fclose(pFile);

			int nDesLen = *(DWORD*)pFileBuffer;
			BYTE* pDes = new BYTE[nDesLen+1];
			int nNodeCount = *(pFileBuffer+sizeof(DWORD))+1;
			int nNodeSize = sizeof(DWORD)+sizeof(BYTE);
			int	nSrcIndex = nNodeSize;
			
			for(int nCount = 0; nCount < nNodeCount; nCount++)
			{
				Huffman_data data;
				memcpy(&data, pFileBuffer+nSrcIndex, nNodeSize);
				vHuffman.push_back(data);
				nSrcIndex += nNodeSize;				
			}
							
				
			int freq[symbols] = {0};  	
			for( int n = 0; n < nNodeCount; n++)
			{
				freq[vHuffman[n].ch] = vHuffman[n].nfrequency;
			}
			
			Tree* tree = BuildTree(freq);  
			int nFinalIndex = 0;
			DWORD nCode;
			nSrcIndex <<= 3;	
			
			while(nFinalIndex < nDesLen)
			{
				nCode = (*(DWORD*)(pFileBuffer+(nSrcIndex>>3)))>>(nSrcIndex&7);		
				Tree* pTree = tree;
				while(pTree->left)	
				{	
					pTree = (nCode&1) ? pTree->right : pTree->left;
					nCode >>= 1;
					nSrcIndex++;
				}
 				pDes[nFinalIndex++] = pTree->ch;
			}

			CDuiString strDecompress = m_pDecompressEdit->GetText();			
			string szDecompress=strDecompress.GetData();
			FILE* pFileToSave = NULL;
			err = fopen_s(&pFile,szDecompress.c_str(),"wb");
		//	err = fopen_s(&pFile,"r://n3.txt","wb");
			fwrite(pDes,nDesLen,1,pFile);
			fclose(pFile);

			SAFE_DELETE(tree);
			SAFE_DELETE(pDes);
			SAFE_DELETE(pFileBuffer);			

			CDuiString str;
			str.Format("decompressed file saved to %s",szDecompress.c_str());
			MessageBox(NULL,str ,_T("done"),MB_OK);

			return; 
		}	
	}		
}



void MainDialog::Init()
{
	m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("closebtn")));   
	m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("restorebtn")));
	m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("minbtn")));			
	m_pCompressBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("Btn_Compress")));
	m_pDecompressBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("Btn_Decompress")));			
	m_pSourceEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("Edit_Sourcefile")));
	m_pCompressEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("Edit_Compressedfile")));
	m_pDecompressEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("Edit_Decompressedfile")));	
}

LRESULT MainDialog::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return FALSE;

}
LRESULT MainDialog::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
	styleValue &= ~WS_CAPTION;
	::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	RECT rcClient;
	::GetClientRect(*this, &rcClient);
	::SetWindowPos(*this, NULL, rcClient.left, rcClient.top, rcClient.right - rcClient.left, \
		rcClient.bottom - rcClient.top, SWP_FRAMECHANGED);

	m_pm.Init(m_hWnd);
	CDialogBuilder builder;
	CControlUI* pRoot = builder.Create(_T("skin.xml"), (UINT)0, this, &m_pm);
	if (pRoot == NULL)
	{
	//	MessageBox(NULL,"no skin file",_T("error"),MB_OK);
	}
	ASSERT(pRoot && "Failed to parse XML");
	m_pm.AttachDialog(pRoot);
	m_pm.AddNotifier(this);
	Init();
	
	return 0;
}
LRESULT MainDialog::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}
LRESULT MainDialog::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	

// 	for (int i=0;i<m_vItem.size();i++)
// 	{
// 		SAFE_DELETE(m_vItem[i]);
// 	}

	::PostQuitMessage(0L);	
	return 0;
}
LRESULT MainDialog::OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if( ::IsIconic(*this) ) bHandled = FALSE;
	return (wParam == 0) ? TRUE : FALSE;
}
LRESULT MainDialog::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}
LRESULT MainDialog::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}
LRESULT MainDialog::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
    ::ScreenToClient(*this, &pt);

    RECT rcClient;
    ::GetClientRect(*this, &rcClient);

    RECT rcCaption = m_pm.GetCaptionRect();
    if( pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
        && pt.y >= rcCaption.top && pt.y < rcCaption.bottom ) 
	{
            CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
            if( pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0 && 
                _tcscmp(pControl->GetClass(), _T("OptionUI")) != 0 &&
                _tcscmp(pControl->GetClass(), _T("TextUI")) != 0
				)
                return HTCAPTION;
    }

    return HTCLIENT;
}
LRESULT MainDialog::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SIZE szRoundCorner = m_pm.GetRoundCorner();
	if( !::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0) ) {
		RECT rcClient;
		::GetClientRect(*this, &rcClient);
		HRGN hRgn = ::CreateRoundRectRgn(rcClient.left, rcClient.top, rcClient.right + 1, rcClient.bottom + 1, szRoundCorner.cx, szRoundCorner.cy);
		::SetWindowRgn(*this, hRgn, TRUE);
		::DeleteObject(hRgn);
	}

	bHandled = FALSE;
	return 0;
}
LRESULT MainDialog::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	MONITORINFO oMonitor = {};
	oMonitor.cbSize = sizeof(oMonitor);
	::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
	CDuiRect rcWork = oMonitor.rcWork;
	rcWork.Offset(-rcWork.left, -rcWork.top);

	LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
	lpMMI->ptMaxPosition.x	= rcWork.left;
	lpMMI->ptMaxPosition.y	= rcWork.top;
	lpMMI->ptMaxSize.x		= rcWork.right;
	lpMMI->ptMaxSize.y		= rcWork.bottom;

	bHandled = FALSE;
	return 0;
}

LRESULT MainDialog::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if( wParam == SC_CLOSE )
	{
		::PostQuitMessage(0L);
		bHandled = TRUE;
		return 0;
	}
	BOOL bZoomed = ::IsZoomed(*this);
	LRESULT lRes = __super::HandleMessage(uMsg, wParam, lParam);
	if( ::IsZoomed(*this) != bZoomed )
	{
		if( !bZoomed )
		{
			CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));
			if( pControl ) pControl->SetVisible(false);
			pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));
			if( pControl ) pControl->SetVisible(true);
		}
		else 
		{
			CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));
			if( pControl ) pControl->SetVisible(true);
			pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));
			if( pControl ) pControl->SetVisible(false);
		}
	}
	return lRes;
}

LRESULT MainDialog::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	switch( uMsg ) 
	{
		case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
		case WM_CLOSE:         lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
		case WM_DESTROY:       lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
		case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
		case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
		case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
		case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
		case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
		case WM_GETMINMAXINFO: lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
		case WM_SYSCOMMAND:    lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
		
		default:
			bHandled = FALSE;
	}
	if( bHandled ) return lRes;
	if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}
