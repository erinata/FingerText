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

#ifndef SNIPPET_DOCK_H
#define SNIPPET_DOCK_H

#include "Dialog.h"
#include "DockingDlgInterface.h"
#include <WindowsX.h>

class DockingDlg : public DockingDlgInterface
{
    public :
        DockingDlg() : DockingDlgInterface(IDD_SNIPPET_DOCK){};

        // Function for all docking dialogs
        virtual void display(bool toShow = true);
    	void setParent(HWND parent2set);
    
        // Custom functions
        void addDockItem(wchar_t *dockItem);
        void resizeListBox(int height=-1,int width=-1);
        void clearDock();
        void setDlgText(int dlg, TCHAR* showText);
        void toggleSave(bool buttonOn);
        int getSelection();
        //void setSelction();
        void DockingDlg::setupHotspotCombo();
        void DockingDlg::switchInsertMode(int insert = 1);
        //int getCount();
        void DockingDlg::updateSnippetCount(wchar_t* count);
        void DockingDlg::insertHotspot();
        void DockingDlg::switchDock(bool toNormal = true);
        void DockingDlg::selectSnippetList(int selection);
        int DockingDlg::searchSnippetList(wchar_t* key);
        int DockingDlg::getTopIndex();
        void DockingDlg::setTopIndex(int index);
        void DockingDlg::getSelectText(TCHAR* &buffer, int index = -1);

    protected :
        
    	virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
    
    private :

};

#endif //SNIPPET_DOCK_H
