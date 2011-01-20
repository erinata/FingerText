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
    }

    void editSnipShow()
    {
        //HWND hwndList = GetDlgItem(_hSelf, ID_SNIPSHOW_EDIT);
        
        ::SetDlgItemText(_hSelf, ID_SNIPSHOW_EDIT,TEXT("Snippet Preview here...\r\nSnippet Preview here...\r\nSnippet Preview here...\r\n"));
    }
    
    void disableSaveSnippet()
    {
        HWND hwndButton = GetDlgItem(_hSelf, IDC_SAVE);
        ::Button_Enable(hwndButton, false);

    }

    int getLength() 
    {
        BOOL isSuccessful;
        int length = ::GetDlgItemInt(_hSelf, IDC_LENGTH, &isSuccessful, false);
        if (!isSuccessful || length<10)
        {
            length=10;
            ::SetDlgItemInt(_hSelf,IDC_LENGTH,10, false);
        }
        return length;
    }

protected :
    
	virtual BOOL CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);

private :

    

};

#endif //SNIPPET_DLG_H
