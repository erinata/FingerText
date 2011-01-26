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
///////////////////////////////////////////////////////
//
//This file is modified from the NppPluginTemplate provided by Don Ho. 
//The License of the template is shown below. 
//
//////////////////////////////////////////////////////
//this file is part of notepad++
//Copyright (C)2003 Don HO ( donho@altern.org )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef SNIPPET_DLG_H
#define SNIPPET_DLG_H


#include "DockingDlgInterface.h"
#include "resource.h"
#include <WindowsX.h>

class DockingDlg : public DockingDlgInterface
{

public :
	DockingDlg() : DockingDlgInterface(IDD_SNIPPET_DOCK){};

    virtual void display(bool toShow = true) const {
        DockingDlgInterface::display(toShow);
        if (toShow)
            ::SetFocus(::GetDlgItem(_hSelf, IDC_SNIPPET_LIST));
    };

	void setParent(HWND parent2set){
		_hParent = parent2set;
	};

    
    void messageDialog(HWND nppHandle, LPCWSTR message)
    {
        ::MessageBox(nppHandle, message, TEXT("Trace"), MB_OK);
    }

    void addDockItem(wchar_t *dockItem)
    {
        SendMessage(GetDlgItem(_hSelf, IDC_SNIPPET_LIST), LB_INSERTSTRING, 0, (LPARAM)dockItem); 
    }

    void clearDock()
    {
        HWND hwndList = GetDlgItem(_hSelf, IDC_SNIPPET_LIST);
        SendMessage(hwndList, LB_RESETCONTENT, 0, 0);

        //::Button_Enable(GetDlgItem(_hSelf, IDC_SAVE), false);
        ::Button_Enable(GetDlgItem(_hSelf, IDC_EDIT), false);
        ::Button_Enable(GetDlgItem(_hSelf, IDC_DELETE), false);
        //::Button_Enable(GetDlgItem(_hSelf, IDC_CREATE), false);
    }

    void editSnipShow(TCHAR* showText)
    {
        //HWND hwndList = GetDlgItem(_hSelf, ID_SNIPSHOW_EDIT);
        ::SetDlgItemText(_hSelf, ID_SNIPSHOW_EDIT,showText);
    }
    

    void setEditMode()
    {
        ::SetDlgItemText(_hSelf, IDC_LIST_TITLE,TEXT("Available Snippets"));

    }

    //int getLength() 
    //{
    //    BOOL isSuccessful;
    //    int length = ::GetDlgItemInt(_hSelf, IDC_LENGTH, &isSuccessful, false);
    //    
    //    if (!isSuccessful || length<10)
    //    {
    //        length=10;
    //        ::SetDlgItemInt(_hSelf,IDC_LENGTH,length, false);
    //    } else if (length>999)
    //    {
    //        length=999;
    //        ::SetDlgItemInt(_hSelf,IDC_LENGTH,length, false);
    //    }
    //    
    //    return length;
    //}

    int getSelection()
    {
        int retVal;
        HWND hwndList = GetDlgItem(_hSelf, IDC_SNIPPET_LIST);
//        retVal = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
        retVal = SendMessage(hwndList, LB_GETANCHORINDEX, 0, 0);
        
        return retVal;

    }

    int getCount()
    {
        int retVal;
        HWND hwndList = GetDlgItem(_hSelf, IDC_SNIPPET_LIST);
        retVal = SendMessage(hwndList, LB_GETCOUNT, 0, 0);
        return retVal;
    }

    //void disableSnippetButtons()
    //{
    //
    //}
    //void enableSnippetButtons()
    //{
    //
    //}
    //
protected :
    
	virtual BOOL CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);

private :

    

};

#endif //SNIPPET_DLG_H
