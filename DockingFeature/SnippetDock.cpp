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

#include "SnippetDock.h"
#include "PluginDefinition.h"

extern NppData nppData;

BOOL CALLBACK DockingDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
        
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
					return TRUE;
				}

                case IDC_SAVE:
                {
                    saveSnippet();
                    return TRUE;
                }

                case IDC_CREATE:
                {
                    selectionToSnippet();
                    //createSnippet();
                    return TRUE;
                }

                case IDC_DELETE:
                {
                    deleteSnippet();
                    return TRUE;
                }
			}
			return FALSE;
		}

        case WM_SETFOCUS:
        {

        }

        case WM_KILLFOCUS:
        {

        }
        
        case WM_INITDIALOG:
        {
            updateMode();
            setDlgText(ID_SNIPSHOW_EDIT,TEXT("Select an item in SnippetDock to view the snippet preview here."));
            //setDlgText(IDC_LIST_TITLE, TEXT("Available Snippets"));
            
            //::SetDlgItemInt(_hSelf,IDC_LENGTH,100, false);
            //SendMessage(GetDlgItem(_hSelf, IDC_SNIPPET_LIST), LB_SETSEL , true, 1);
            return TRUE;
         
        } 


		default :
			return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
	}
}
