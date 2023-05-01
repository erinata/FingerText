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

#ifndef INSERTION_DLG_H
#define INSERTION_DLG_H

#include "Dialog.h"
#include "StaticDialog.h"
#include "PluginInterface.h"

#include "debugUtils.h"

class InsertionDlg : public StaticDialog
{

public:
	InsertionDlg() : StaticDialog() {};
    
    void init(HINSTANCE hInst, NppData nppData)
	{
		_nppData = nppData;
		Window::init(hInst, nppData._nppHandle);
	};

   	void InsertionDlg::doDialog(int initialState = 1);
    TCHAR* InsertionDlg::getEditText();
    void InsertionDlg::addDockItem(wchar_t *dockItem);
    void InsertionDlg::clearList();
    bool InsertionDlg::insertSnippet();
    void InsertionDlg::clearText();
    void InsertionDlg::setTextTarget(bool fromTab);
    void InsertionDlg::setListTarget();
    bool InsertionDlg::completeSnippets();
    int InsertionDlg::getEditPos();
    void InsertionDlg::getSelectText(TCHAR* &buffer, LRESULT index = -1);
    void InsertionDlg::setDlgText(int dlg, TCHAR* showText);
    void InsertionDlg::changeMode(bool withComment);
    void InsertionDlg::changeExpand();
    void InsertionDlg::updateInsertionHint();
    void InsertionDlg::adjustTextHintPosition();

    virtual void destroy() 
    {

    };
    
protected :
	virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);

private:
	NppData			_nppData;
    HWND			_HSource;
};

#endif // INSERTION_DLG_H