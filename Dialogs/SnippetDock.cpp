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

bool normalMode = true;
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
    int y;
    int x;
    
    if (normalMode)
    {
        y = 80;
    } else
    {
        y = 200;
    }
    x = 0;

    int minimumHeight = 100;

    if (height = -1)
    {
       RECT rect;
       GetWindowRect(_hSelf, &rect);
       height =  rect.bottom - rect.top;

    }
    width = 285;
    if (normalMode)
    {
        height = height - y - 140;
    } else
    {
        height = height - y - 180;
    }

    if (height < minimumHeight) height = minimumHeight;
    
    
    

    SetWindowPos(GetDlgItem(_hSelf, IDC_STATIC_TEXT),NULL,x,y,width,20,SWP_NOACTIVATE);
    SetWindowPos(GetDlgItem(_hSelf, IDC_PREVIEW_EDIT),NULL,x,y+20,width,20,SWP_NOACTIVATE);
    SetWindowPos(GetDlgItem(_hSelf, ID_SNIPSHOW_EDIT),NULL,x,y+40,width,120,SWP_NOACTIVATE);
    
    
    SetWindowPos(GetDlgItem(_hSelf, IDC_STATIC_TEXT2),NULL,x,y+160,40,20,SWP_NOACTIVATE);
    SetWindowPos(GetDlgItem(_hSelf, IDC_FILTER),NULL,x+42,y+160,width-42,20,SWP_NOACTIVATE);

    if (normalMode)
    {
        SetWindowPos(GetDlgItem(_hSelf, IDC_SNIPPET_LIST),NULL,x,y+180,width,height,SWP_NOACTIVATE);
        SetWindowPos(GetDlgItem(_hSelf, IDC_STATIC_TEXT4),NULL,x,y+160,width,height,SWP_NOACTIVATE);
        
    } else
    {
        SetWindowPos(GetDlgItem(_hSelf, IDC_SNIPPET_LIST),NULL,x,y+200,width,height,SWP_NOACTIVATE);
        SetWindowPos(GetDlgItem(_hSelf, IDC_STATIC_TEXT5),NULL,x,y+180,width,height,SWP_NOACTIVATE);
        
    }

    // repaint the window 
    //InvalidateRect (_hSelf, NULL, TRUE);
    //UpdateWindow (_hSelf);
}

void DockingDlg::clearDock()
{
    SendMessage(GetDlgItem(_hSelf, IDC_SNIPPET_LIST), LB_RESETCONTENT, 0, 0);

    //::Button_Enable(GetDlgItem(_hSelf, IDC_SAVE), false);
    ::Button_Enable(GetDlgItem(_hSelf, IDC_INSERT), false);

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

void DockingDlg::switchDock(bool toNormal)
{
    
    //SendMessage (GetDlgItem(_hSelf, IDC_RADIO_NORMAL), BM_SETCHECK, toNormal, 0);
    //SendMessage (GetDlgItem(_hSelf, IDC_RADIO_EDITOR), BM_SETCHECK, !toNormal, 0);

    normalMode = toNormal;

    int editorButtonHeight = 20;
    int editorButtonWidth = 135;
    int firstRow = 45;
    int firstColumn = 2;

    SetWindowPos(GetDlgItem(_hSelf, IDC_STATIC_TEXT3),NULL,firstColumn                      ,25,editorButtonWidth,editorButtonHeight,SWP_NOACTIVATE);
    

    SetWindowPos(GetDlgItem(_hSelf, IDC_NEW       ),NULL,firstColumn                      ,firstRow,editorButtonWidth,editorButtonHeight,SWP_NOACTIVATE);
                                                                                          
    SetWindowPos(GetDlgItem(_hSelf, IDC_EDIT      ),NULL,firstColumn                      ,firstRow+((editorButtonHeight+2)*1),editorButtonWidth,editorButtonHeight,SWP_NOACTIVATE);
    SetWindowPos(GetDlgItem(_hSelf, IDC_SAVE      ),NULL,firstColumn+(editorButtonWidth+2),firstRow+((editorButtonHeight+2)*1),editorButtonWidth,editorButtonHeight,SWP_NOACTIVATE);
    
    SetWindowPos(GetDlgItem(_hSelf, IDC_DELETE    ),NULL,firstColumn                      ,firstRow+((editorButtonHeight+2)*2),editorButtonWidth,editorButtonHeight,SWP_NOACTIVATE);
    SetWindowPos(GetDlgItem(_hSelf, IDC_DELETEALL ),NULL,firstColumn+(editorButtonWidth+2),firstRow+((editorButtonHeight+2)*2),editorButtonWidth,editorButtonHeight,SWP_NOACTIVATE);

    SetWindowPos(GetDlgItem(_hSelf, IDC_IMPORTFILE),NULL,firstColumn                      ,firstRow+((editorButtonHeight+2)*3),editorButtonWidth,editorButtonHeight,SWP_NOACTIVATE);
    SetWindowPos(GetDlgItem(_hSelf, IDC_IMPORTURL ),NULL,firstColumn+(editorButtonWidth+2),firstRow+((editorButtonHeight+2)*3),editorButtonWidth,editorButtonHeight,SWP_NOACTIVATE);

    SetWindowPos(GetDlgItem(_hSelf, IDC_EXPORT    ),NULL,firstColumn                      ,firstRow+((editorButtonHeight+2)*4),editorButtonWidth,editorButtonHeight,SWP_NOACTIVATE);
    SetWindowPos(GetDlgItem(_hSelf, IDC_EXPORTALL ),NULL,firstColumn+(editorButtonWidth+2),firstRow+((editorButtonHeight+2)*4),editorButtonWidth,editorButtonHeight,SWP_NOACTIVATE);
        
    SetWindowPos(GetDlgItem(_hSelf, IDC_CLOSEEDITOR),NULL,2,2,285,editorButtonHeight,SWP_NOACTIVATE);

    SetWindowPos(GetDlgItem(_hSelf, IDC_OPENEDITOR),NULL,2,2,285,editorButtonHeight,SWP_NOACTIVATE);
    SetWindowPos(GetDlgItem(_hSelf, IDC_INSERT),NULL,2,30,130,30,SWP_NOACTIVATE);
    SetWindowPos(GetDlgItem(_hSelf, IDC_CREATESELECTION),NULL,135,30,130,30,SWP_NOACTIVATE);
    
    

    if (toNormal)
    {

        

        ::ShowWindow(GetDlgItem(_hSelf, IDC_CREATESELECTION),SW_SHOW);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_INSERT),SW_SHOW);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_OPENEDITOR),SW_SHOW);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_STATIC_TEXT4),SW_SHOW);

        ::ShowWindow(GetDlgItem(_hSelf, IDC_STATIC_TEXT3),SW_HIDE);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_CLOSEEDITOR),SW_HIDE);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_NEW),SW_HIDE);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_EDIT),SW_HIDE);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_SAVE),SW_HIDE);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_DELETE),SW_HIDE);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_IMPORTFILE),SW_HIDE);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_IMPORTURL),SW_HIDE);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_EXPORT),SW_HIDE);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_EXPORTALL),SW_HIDE);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_DELETEALL),SW_HIDE);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_FILTER),SW_HIDE);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_STATIC_TEXT2),SW_HIDE);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_STATIC_TEXT5),SW_HIDE);


        
        //setSnippetDockState(1);

    } else
    {
        ::ShowWindow(GetDlgItem(_hSelf, IDC_CREATESELECTION),SW_HIDE);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_INSERT),SW_HIDE);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_OPENEDITOR),SW_HIDE);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_STATIC_TEXT4),SW_HIDE);

        ::ShowWindow(GetDlgItem(_hSelf, IDC_STATIC_TEXT3),SW_SHOW);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_CLOSEEDITOR),SW_SHOW);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_NEW),SW_SHOW);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_EDIT),SW_SHOW);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_SAVE),SW_SHOW);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_DELETE),SW_SHOW);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_IMPORTFILE),SW_SHOW);
        //::ShowWindow(GetDlgItem(_hSelf, IDC_IMPORTURL),SW_SHOW);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_EXPORT),SW_SHOW);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_EXPORTALL),SW_SHOW);
        //::ShowWindow(GetDlgItem(_hSelf, IDC_DELETEALL),SW_SHOW);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_FILTER),SW_SHOW);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_STATIC_TEXT2),SW_SHOW);
        ::ShowWindow(GetDlgItem(_hSelf, IDC_STATIC_TEXT5),SW_SHOW);
        //setSnippetDockState(0);
    }


    resizeListBox();

    InvalidateRect (_hSelf, NULL, TRUE);
    UpdateWindow (_hSelf);
    //alert();
}


//int DockingDlg::getCount()
//{
//    int retVal;
//    HWND hwndList = GetDlgItem(_hSelf, IDC_SNIPPET_LIST);
//    retVal = SendMessage(hwndList, LB_GETCOUNT, 0, 0);
//    return retVal;
//}

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

            if (((HIWORD(wParam) == LBN_SETFOCUS) || (HIWORD(wParam) == LBN_SELCHANGE)) && LOWORD(wParam) == IDC_SNIPPET_LIST)
            {
                showPreview();
                //refreshAnnotation();
                //keyUpdate();
                //updateDockItems();
                //::Button_Enable(GetDlgItem(_hSelf, IDC_SAVE), true);
                ::Button_Enable(GetDlgItem(_hSelf, IDC_INSERT), true);

                ::Button_Enable(GetDlgItem(_hSelf, IDC_EDIT), true);
                ::Button_Enable(GetDlgItem(_hSelf, IDC_DELETE), true);
                //::Button_Enable(GetDlgItem(_hSelf, IDC_CREATE), true);

            } else if (HIWORD(wParam) == LBN_DBLCLK && LOWORD(wParam) == IDC_SNIPPET_LIST)
            {
                if (normalMode)
                {
                    insertSnippet();
                } else
                {
                    editSnippet();
                }
                
            } else if  (((HIWORD(wParam) == EN_SETFOCUS) || (HIWORD(wParam) == EN_CHANGE)) && (LOWORD(wParam) == IDC_FILTER))
            {
                if (::GetFocus() == GetDlgItem(_hSelf, IDC_FILTER))
                {
                    int length = SendMessage(GetDlgItem(_hSelf, IDC_FILTER), EM_LINELENGTH, 0, 0);
                    if (length>0)
                    {
                        TCHAR* bufferWide = new TCHAR[length+1];
                        ::GetDlgItemText(_hSelf, IDC_FILTER ,bufferWide,length+1);
                        char* buffer = toCharArray(bufferWide);
                        bool leadingSpace = 0;
                        bool trailingSpace = 0;
                        if (buffer[0] == ' ')
                        {
                            leadingSpace = 1;
                        }
                        if (buffer[strlen(buffer)-1] == ' ')
                        {
                            trailingSpace = 1;
                        }
                       
                        char* key = new char[strlen(buffer)+2+1];
                        strcpy(key,"");
                        if (leadingSpace)
                        {
                            if (trailingSpace)
                            {
                                buffer[strlen(buffer)-1] = '\0';
                                strcat(key,buffer+1);
                            } else
                            {
                                strcat(key,buffer+1);
                                strcat(key,"%");
                            }
                        
                        } else
                        {
                            strcat(key,"%");
                        
                            if (trailingSpace)
                            {
                                buffer[strlen(buffer)-1] = '\0';
                                strcat(key,buffer);
                            } else
                            {
                                strcat(key,buffer);
                                strcat(key,"%");
                            }
                        
                        }
                        
                        
                        
                        updateDockItems(true,false,key,true,false,true);
                        delete [] buffer;
                        
                        delete [] bufferWide;
                        delete [] key;
                    } else
                    {
                        updateDockItems(true,false,"%",true,false);  
                    }
                }
                
            }


			switch (wParam)
			{

                //case IDC_RADIO_NORMAL:
                //{
                //    switchDock(true);
                //    return true;
                //}
                //
                //case IDC_RADIO_EDITOR:
                //{
                //    switchDock(false);
                //    return true;
                //}
                case IDOK:
                {
                    if (normalMode)
                    {
                        insertSnippet();
                    } else
                    {
                        editSnippet();
                    }

                }

                case IDC_INSERT:
                {
                    insertSnippet();
                    return true;
                }

        		case IDC_EDIT :
				{

                    editSnippet();
                    //updateDockItems();
					return true;
				}

                case IDC_SAVE:
                {
                    saveSnippet();
                    return true;
                }

                case IDC_CLOSEEDITOR:
                {
                    closeEditor();
                    return true;
                }

                case IDC_OPENEDITOR:
                {
                    editSnippet();
                    return true;
                }

                case IDC_NEW:
                case IDC_CREATESELECTION:
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

                case IDC_IMPORTURL:
                {
                    return true;
                }
                case IDC_IMPORTFILE:
                {
                    importSnippets();
                    return true;
                }
                case IDC_EXPORT:
                {
                    exportSnippets(false);
                    return true;
                }
                case IDC_EXPORTALL:
                {
                    exportSnippets(true);
                    return true;
                }
                case IDC_DELETEALL:
                {
                    return true;
                }

			}
			return false;
		}




        //case WM_SETFOCUS:
        //{
        //   
        //}
        //
        //case WM_KILLFOCUS:
        //{
        //
        //}
        
        case WM_INITDIALOG:
        {


            //switchDock(true);
            return true;
            //updateMode();
            //setDlgText(ID_SNIPSHOW_EDIT,TEXT("Select an item in SnippetDock to view the snippet preview here."));
            
            
            //return true;
         
        } 


		default :
			return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
	}
}
