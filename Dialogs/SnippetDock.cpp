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
#include "SnippetDock.h"


//extern NppData nppData;

// Function for all docking dialogs
void DockingDlg::display(bool toShow) 
{
    DockingDlgInterface::display(toShow);
    if (toShow) ::SetFocus(::GetDlgItem(_hSelf, IDC_SNIPPET_LIST));
}

void DockingDlg::setParent(HWND parent2set)
{
	_hParent = parent2set;
}

// Custom functions
void DockingDlg::addDockItem(wchar_t *dockItem)
{
    SendMessage(GetDlgItem(_hSelf, IDC_SNIPPET_LIST), LB_INSERTSTRING, 0, (LPARAM)dockItem); 
}

void DockingDlg::resizeListBox(int height,int width)
{
    //TODO: less hardcoding......make snippet dock size configurable
    height = height - 255;
    if (height < 100) height = 100;
    SetWindowPos(GetDlgItem(_hSelf, IDC_SNIPPET_LIST),NULL,0,250,285,height,SWP_NOMOVE & SWP_NOACTIVATE);
}

void DockingDlg::clearDock()
{
    HWND hwndList = GetDlgItem(_hSelf, IDC_SNIPPET_LIST);
    SendMessage(hwndList, LB_RESETCONTENT, 0, 0);

    //::Button_Enable(GetDlgItem(_hSelf, IDC_SAVE), false);
    ::Button_Enable(GetDlgItem(_hSelf, IDC_EDIT), false);
    ::Button_Enable(GetDlgItem(_hSelf, IDC_DELETE), false);
    //::Button_Enable(GetDlgItem(_hSelf, IDC_CREATE), false);
}

void DockingDlg::setDlgText(int dlg, TCHAR* showText)
{
    ::SetDlgItemText(_hSelf, dlg ,showText);

}

void DockingDlg::toggleSave(bool buttonOn)
{
    ::Button_Enable(GetDlgItem(_hSelf, IDC_SAVE), buttonOn);
}


int DockingDlg::getSelection()
{
    int retVal;
    HWND hwndList = GetDlgItem(_hSelf, IDC_SNIPPET_LIST);
    retVal = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
    retVal = SendMessage(hwndList, LB_GETANCHORINDEX, 0, 0);
    
    return retVal;

}

void DockingDlg::getSelectText(TCHAR* &buffer, int index)
{
     
    HWND hwndList = GetDlgItem(_hSelf, IDC_SNIPPET_LIST);
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

void DockingDlg::setSelction()
{
    HWND hwndList = GetDlgItem(_hSelf, IDC_SNIPPET_LIST);
    SendMessage(hwndList, LB_SETANCHORINDEX, 0, 0);
}

int DockingDlg::getCount()
{
    int retVal;
    HWND hwndList = GetDlgItem(_hSelf, IDC_SNIPPET_LIST);
    retVal = SendMessage(hwndList, LB_GETCOUNT, 0, 0);
    return retVal;
}

BOOL CALLBACK DockingDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
        case WM_SIZE:
        {
            //TODO: this is triggering too many times esp when npp is starting up, some efficiency improvment can be done
            resizeListBox(HIWORD(lParam),LOWORD(lParam));
            //alert();
        }

		case WM_COMMAND : 
		{

            if  (HIWORD(wParam) == LBN_SELCHANGE && LOWORD(wParam) == IDC_SNIPPET_LIST)
            {
                showPreview();
              
            } else if (HIWORD(wParam) == LBN_SETFOCUS && LOWORD(wParam) == IDC_SNIPPET_LIST)
            {
                //refreshAnnotation();
                //keyUpdate();
                //updateDockItems();
                //::Button_Enable(GetDlgItem(_hSelf, IDC_SAVE), true);
                ::Button_Enable(GetDlgItem(_hSelf, IDC_EDIT), true);
                ::Button_Enable(GetDlgItem(_hSelf, IDC_DELETE), true);
                //::Button_Enable(GetDlgItem(_hSelf, IDC_CREATE), true);

            } else if (HIWORD(wParam) == LBN_DBLCLK && LOWORD(wParam) == IDC_SNIPPET_LIST)
            {
                editSnippet();
                //insertSnippet();
            } 


			switch (wParam)
			{
        		case IDC_EDIT :
				{

                    editSnippet();
                    //editSnipShow();
                    //updateDockItems();
					return true;
				}

                case IDC_SAVE:
                {
                    saveSnippet();
                    return true;
                }

                case IDC_CREATE:
                {
                    selectionToSnippet();
                    //createSnippet();
                    return true;
                }

                case IDC_DELETE:
                {
                    deleteSnippet();
                    return true;
                }
			}
			return false;
		}




        case WM_SETFOCUS:
        {

        }

        case WM_KILLFOCUS:
        {

        }
        
        case WM_INITDIALOG:
        {
            
            //updateMode();
            //setDlgText(ID_SNIPSHOW_EDIT,TEXT("Select an item in SnippetDock to view the snippet preview here."));
            
            
            //return true;
         
        } 


		default :
			return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
	}
}
