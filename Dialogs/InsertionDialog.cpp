//This file is part of FingerText, a notepad++ snippet plugin.
//
//FingerText is released under MIT License.
//
//MIT license
//
//Copyright (C) 2011 by Tom Lam
//
//Permission is hereby granted, free of charge, to any person 
//obtaining a copy of this software and associated documentation 
//files (the "Software"), to deal in the Software without 
//restriction, including without limitation the rights to use, 
//copy, modify, merge, publish, distribute, sublicense, and/or 
//sell copies of the Software, and to permit persons to whom the 
//Software is furnished to do so, subject to the following 
//conditions:
//
//The above copyright notice and this permission notice shall be 
//included in all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
//OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
//NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
//HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
//WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
//DEALINGS IN THE SOFTWARE.

#include "PluginDefinition.h"
#include "InsertionDialog.h"


WNDPROC wndProcEdit;
WNDPROC wndProcList;
int expand;

void InsertionDlg::doDialog(int initialState)
{
    if (!isCreated())
    {
        create(IDD_INSERTION_DLG);
        if (initialState==1) expand = 340;
        else expand = 0;
        changeExpand();
    }
    
    //goToCenter(); 
    
    //TODO: can change here to customize where the dialog goes when initialized
    RECT rc;
    ::GetClientRect(_hParent, &rc);
    POINT center;
    center.x = rc.left + (rc.right - rc.left)/2;
    center.y = rc.top + (rc.bottom - rc.top - expand)/2;
    ::ClientToScreen(_hParent, &center);

	int x = center.x - (_rc.right - _rc.left)/2;
	int y = center.y - (_rc.bottom - _rc.top - expand)/2;

	::SetWindowPos(_hSelf, HWND_TOP, x, y, _rc.right - _rc.left, _rc.bottom - _rc.top - expand, SWP_SHOWWINDOW);

}

void InsertionDlg::changeExpand()
{
    //TODO: consider not using button to trigger, but use up and down key
    if (expand == 340)
    {
        SendDlgItemMessage( _hSelf, IDC_BUTTON_MORE, WM_SETTEXT, 0, (LPARAM)TEXT("less"));
        expand = 0;
        setInsertionDialogState(1);
        doDialog();
    } else
    {
        SendDlgItemMessage( _hSelf, IDC_BUTTON_MORE, WM_SETTEXT, 0, (LPARAM)TEXT("more"));
        expand = 340;
        setInsertionDialogState(0);
        doDialog();
    }
    SetFocus(GetDlgItem(_hSelf, IDC_INSERTION_EDIT));
}

void InsertionDlg::addDockItem(wchar_t *dockItem)
{
    SendMessage(GetDlgItem(_hSelf, IDC_INSERTION_LIST), LB_INSERTSTRING, 0, (LPARAM)dockItem); 
}

void InsertionDlg::clearList()
{
    SendMessage(GetDlgItem(_hSelf, IDC_INSERTION_LIST), LB_RESETCONTENT, 0, 0);
}

void InsertionDlg::clearText()
{
    ::SetDlgItemText(_hSelf, IDC_INSERTION_EDIT ,TEXT(""));
    ::SetDlgItemText(_hSelf, IDC_INSERTION_PREVIEW ,TEXT(""));
}

int InsertionDlg::getEditPos()
{
    int startPos;
    int endPos;
    SendMessage(GetDlgItem(_hSelf, IDC_INSERTION_EDIT), EM_GETSEL , (WPARAM)&startPos, (LPARAM)&endPos);             
    return startPos;

}

TCHAR* InsertionDlg::getEditText()
{
    int length = SendMessage(GetDlgItem(_hSelf, IDC_INSERTION_EDIT), EM_LINELENGTH, 0, 0);             
    TCHAR* bufferWide = new TCHAR[length+1];
    ::GetDlgItemText(_hSelf, IDC_INSERTION_EDIT ,bufferWide,length+1);
    return bufferWide;
}

bool InsertionDlg::insertSnippet()
{
    bool retVal = false;
    TCHAR* bufferWide = getEditText();
    char* buffer = new char[_tcslen(bufferWide)+1];
    buffer = toCharArray(bufferWide);

    retVal = diagActivate(buffer);


    delete [] buffer;
    delete [] bufferWide;

    return retVal;
}

void InsertionDlg::setDlgText(int dlg, TCHAR* showText)
{
    
    ::SetDlgItemText(_hSelf, dlg ,showText);

}

void InsertionDlg::getSelectText(TCHAR* &buffer, int index)
{
     
    HWND hwndList = GetDlgItem(_hSelf, IDC_INSERTION_LIST);
    if (index = -1) index = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
    if (index <= 0) index = 0;
    int length = SendMessage(hwndList, LB_GETTEXTLEN, index, 0);
    if (length >=  1)
    {

        buffer = new TCHAR[length+1];
        SendMessage(hwndList, LB_GETTEXT, index, (LPARAM)buffer);
    } else
    {
        buffer = new TCHAR[1];
        buffer[0] = '\0';
    }
}

LRESULT SubWndProcList(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT	retVal = 0;
	switch (message)
	{ 
        case WM_KEYUP:
        {          
            if (wParam == VK_TAB)
            {
                setTextTarget(true);
            } else if (wParam == VK_UP)
            {
                setTextTarget(false);
            }

            retVal = ::CallWindowProc(wndProcList, hwnd, message, wParam, lParam);
            break;
        }
		default:
			retVal = CallWindowProc(wndProcList, hwnd, message, wParam, lParam);

	}

	return retVal;
}

LRESULT SubWndProcEdit(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT	retVal = 0;
	switch (message)
	{
        //case WM_SETFOCUS:
        //{
        //    
        //    retVal = ::CallWindowProc(wndProcEdit, hwnd, message, wParam, lParam);
        //    break;
        //}
        //
        //case WM_KILLFOCUS:
        //{
        //    
        //    retVal = ::CallWindowProc(wndProcEdit, hwnd, message, wParam, lParam);
        //    break;
        //}
        //
        case WM_KEYUP:
        {          
            if (wParam == VK_TAB)
            {
                //TODO: doesn't seen right to do a round trip to plugindefinition to call the dialog, restrucuture needed.
                triggerDiagInsertion();

            } else if (wParam == VK_DOWN)
            {
                
                setListTarget();

            } else
            {
                updateInsertionDialogHint();
            }
            retVal = ::CallWindowProc(wndProcEdit, hwnd, message, wParam, lParam);


            //TODO: when the user press down and the focus is on edit control, the focus should be set to the list. In the list, if user press up and the selection is at the first selection, focus should go to the edit control
            //if (wParam == VK_DOWN)
            //{
            //    
            //
            //}
            //
            break;
        }
		default:
			retVal = CallWindowProc(wndProcEdit, hwnd, message, wParam, lParam);

	}

	return retVal;
}

bool InsertionDlg::completeSnippets()
{
    bool retVal = false;
    SendDlgItemMessage(_hSelf, IDC_INSERTION_LIST, LB_SETSEL, 0, 0);
    showPreview(false,true);
    int length = SendMessage(GetDlgItem(_hSelf, IDC_INSERTION_EDIT), EM_LINELENGTH, 0, 0);  
    SendDlgItemMessage(_hSelf, IDC_INSERTION_EDIT, EM_SETSEL, length, -1);
    //TODO: complete snippet in dialog

    return retVal;
}


void InsertionDlg::changeMode(bool withComment)
{
    //TODO: investigate why changing size is not working
    
    //if (withComment)
    //{
    //    SetWindowPos(GetDlgItem(_hSelf, IDC_INSERTION_DES),NULL,109,182,208,37,SWP_NOACTIVATE);
    //    SetWindowPos(GetDlgItem(_hSelf, IDC_INSERTION_PREVIEW),NULL,109,31,208,150,SWP_NOACTIVATE);
    //} else
    //{
    //    SetWindowPos(GetDlgItem(_hSelf, IDC_INSERTION_DES),NULL,109,217,208,2,SWP_NOACTIVATE);
    //    SetWindowPos(GetDlgItem(_hSelf, IDC_INSERTION_PREVIEW),NULL,109,31,208,185,SWP_NOACTIVATE);
    //}
}

void InsertionDlg::setTextTarget(bool fromTab)
{
    int index = SendMessage(GetDlgItem(_hSelf, IDC_INSERTION_LIST), LB_GETCURSEL, 0, 0);

    if ((fromTab) || (index)==0)
    {
        int length = SendMessage(GetDlgItem(_hSelf, IDC_INSERTION_EDIT), EM_LINELENGTH, 0, 0);  
        SetFocus(GetDlgItem(_hSelf, IDC_INSERTION_EDIT));
        SendDlgItemMessage(_hSelf, IDC_INSERTION_EDIT, EM_SETSEL, length, -1);
    }
    
}

void InsertionDlg::setListTarget()
{
    int count = SendMessage(GetDlgItem(_hSelf, IDC_INSERTION_LIST), LB_GETCOUNT, 0, 0);
    if (count>0)
    {
        SetFocus(GetDlgItem(_hSelf, IDC_INSERTION_LIST));
        SendDlgItemMessage(_hSelf, IDC_INSERTION_LIST, LB_SETSEL, 0, 0);
    }
}


void InsertionDlg::updateInsertionHint()
{
    if (::GetFocus() == GetDlgItem(_hSelf, IDC_INSERTION_EDIT))
    {
       int length = SendMessage(GetDlgItem(_hSelf, IDC_INSERTION_EDIT), EM_LINELENGTH, 0, 0);
       if (length>0)
       {
          TCHAR* bufferWide = new TCHAR[length+1];
          ::GetDlgItemText(_hSelf, IDC_INSERTION_EDIT ,bufferWide,length+1);
          char* buffer = toCharArray(bufferWide);
          std::vector<std::string> vs = toVectorString(buffer,'(',2);
          delete [] buffer;
          
          char* key = new char[strlen(vs[0].c_str())+1+1];
          strcpy(key,vs[0].c_str());
          strcat(key,"%");
          updateDockItems(true,false,key,false,true);
          //::SetDlgItemText(_hSelf, IDC_INSERTION_HINT ,bufferWide);
          delete [] bufferWide;
          
          delete [] key;
       } else
       {
           updateDockItems(true,false,"%",false,true);  
           ::SetDlgItemText(_hSelf, IDC_INSERTION_HINT_HIGHLIGHT,TEXT(""));
           ::SetDlgItemText(_hSelf, IDC_INSERTION_HINT_POST ,TEXT(""));
           ::SetDlgItemText(_hSelf, IDC_INSERTION_HINT ,TEXT("Type the TriggerText of the snippet and press TAB to insert."));
           adjustTextHintPosition();
       }
    }
}

void InsertionDlg::adjustTextHintPosition()
{
    int fontWidth = 7;
    int offset = 4;
    int length1 = SendMessage(GetDlgItem(_hSelf, IDC_INSERTION_HINT), EM_LINELENGTH, 0, 0);  
    int length2 = SendMessage(GetDlgItem(_hSelf, IDC_INSERTION_HINT_HIGHLIGHT), EM_LINELENGTH, 0, 0);  
    int length3 = SendMessage(GetDlgItem(_hSelf, IDC_INSERTION_HINT_POST), EM_LINELENGTH, 0, 0);  
    
    SetWindowPos(GetDlgItem(_hSelf, IDC_INSERTION_HINT)          ,NULL,5                                                  ,35,offset+length1*fontWidth,20,SWP_NOACTIVATE);
    SetWindowPos(GetDlgItem(_hSelf, IDC_INSERTION_HINT_HIGHLIGHT),NULL,5+offset+length1*fontWidth                         ,35,offset+length2*fontWidth,20,SWP_NOACTIVATE);
    SetWindowPos(GetDlgItem(_hSelf, IDC_INSERTION_HINT_POST)     ,NULL,5+offset+length1*fontWidth+offset+length2*fontWidth,35,offset+length3*fontWidth,20,SWP_NOACTIVATE);

    InvalidateRect (_hSelf, NULL, TRUE);
    UpdateWindow (_hSelf);
            
}

INT_PTR CALLBACK InsertionDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{

        case WM_CTLCOLORSTATIC:
        {
            //if ((HWND)lParam == GetDlgItem(_hSelf, IDC_INSERTION_HINT_HIGHLIGHT)) 
            //{
            //    SetBkMode((HDC)wParam,TRANSPARENT);
            //    SetTextColor((HDC)wParam, RGB(0,0,0));
            //    return (BOOL)CreateSolidBrush (GetSysColor(COLOR_MENU));
            //        
            //} else 
            if (((HWND)lParam == GetDlgItem(_hSelf, IDC_INSERTION_HINT)) || ((HWND)lParam == GetDlgItem(_hSelf, IDC_INSERTION_HINT_POST)))
            {
                SetBkMode((HDC)wParam,TRANSPARENT);
                SetTextColor((HDC)wParam, RGB(128,128,128));
                return (BOOL)CreateSolidBrush (GetSysColor(COLOR_MENU));

            }
            return false;
        }

        case WM_INITDIALOG:
		{
            //::SetTextColor((HDC)GetDlgItem(_hSelf, IDC_INSERTION_HINT_HIGHLIGHT),RGB(255,0,0));

            wndProcEdit = (WNDPROC)SetWindowLongPtr(GetDlgItem(_hSelf, IDC_INSERTION_EDIT), GWLP_WNDPROC, (LONG_PTR)SubWndProcEdit);
            wndProcList = (WNDPROC)SetWindowLongPtr(GetDlgItem(_hSelf, IDC_INSERTION_LIST), GWLP_WNDPROC, (LONG_PTR)SubWndProcList);

            HFONT hFont=CreateFont (14, 0, 0, 0, FW_BLACK, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Consolas");
            SendMessage (GetDlgItem(_hSelf, IDC_INSERTION_HINT_HIGHLIGHT), WM_SETFONT, WPARAM (hFont), TRUE);
            

            adjustTextHintPosition();

			return true;
		}
        
        case WM_ACTIVATE:
        {

            if ((wParam == false) && (isVisible()))
            {
                clearList();
                clearText();
            
                display(false);
            } else
            {
                ::SetDlgItemText(_hSelf, IDC_INSERTION_HINT_HIGHLIGHT,TEXT(""));
                ::SetDlgItemText(_hSelf, IDC_INSERTION_HINT_POST ,TEXT(""));
                ::SetDlgItemText(_hSelf, IDC_INSERTION_HINT ,TEXT("Type the TriggerText of the snippet and press TAB to insert."));
                updateDockItems(true,false,"%",false,true);  
                adjustTextHintPosition();
            }
            return true;
        }

		case WM_COMMAND: 
		{
            if  (HIWORD(wParam) == LBN_SELCHANGE && LOWORD(wParam) == IDC_INSERTION_LIST)
            {
                showPreview(false,true);
              
            } else if (HIWORD(wParam) == LBN_DBLCLK && LOWORD(wParam) == IDC_INSERTION_LIST)
            {
                //insertSnippet();
            } 


            if  (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_INSERTION_EDIT)
            {
                //updateInsertionHint();
                return true;

            }
            

			switch (wParam)
			{
				case IDOK:
                    insertSnippet();
                    clearList();
                    clearText();
                    display(false);
                    return true;
				case IDCANCEL:
                    clearList();
                    clearText();
                	display(false);
					return true;
                case IDC_BUTTON_MORE :
                     changeExpand();
                     return true;
				default:
                    return false;
			}
		}
        
        default:
            //return false;
            return StaticDialog::dlgProc(_HSource, message, wParam, lParam);
	}
	
}
