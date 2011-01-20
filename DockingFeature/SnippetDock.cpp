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
			switch (wParam)
			{
				case IDOK :
				{

                    editSnipShow();
                    //updateDockItems();

                    //::MessageBox(nppData._nppHandle, TEXT("OK"), TEXT("Trace"), MB_OK);
                    //::SetDlgItemInt(_hSelf, ID_GOLINE_EDIT, 1, FALSE);
                    
					//int line = getLine();
					//if (line != -1)
					//{
					//	// Get the current scintilla
					//	 HWND curScintilla = getCurrentScintilla();
                    //
					//	::SendMessage(curScintilla, SCI_ENSUREVISIBLE, line-1, 0);
					//	::SendMessage(curScintilla, SCI_GOTOLINE, line-1, 0);
                        
					//}
					return TRUE;
				}

                case IDC_SAVE:
                {
                    saveSnippet();
                }

			}
			return FALSE;
		}

        case WM_INITDIALOG:
        {

            return TRUE;
         
        } 


		default :
			return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
	}
}
