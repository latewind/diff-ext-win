/*
 * Copyright (c) 2003-2010, Sergey Zorin. All rights reserved.
 *
 * This software is distributable under the BSD license. See the terms
 * of the BSD license in the LICENSE file provided with this software.
 *
 */

#ifndef __diff_ext_h__
#define __diff_ext_h__

#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>

#include <util/cont/dlist.h>
#include <util/string.h>
#include <util/menu.h>

// this is the actual OLE Shell context menu handler
class DIFF_EXT : public IContextMenu3, IShellExtInit {
  public:
    DIFF_EXT();
    virtual ~DIFF_EXT();

    //IUnknown members
    STDMETHODIMP QueryInterface(REFIID interface_id, void** result);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    //IContextMenu members
    STDMETHODIMP QueryContextMenu(HMENU menu, UINT index, UINT cmd_first, UINT cmd_last, UINT flags);
    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO info);
    STDMETHODIMP GetCommandString(UINT_PTR cmd, UINT flags, UINT* reserved, LPSTR name, UINT name_length);

    //IContextMenu2 members
    STDMETHODIMP HandleMenuMsg(UINT msg, WPARAM w_param, LPARAM l_param);

    //IContextMenu3 members
    STDMETHODIMP HandleMenuMsg2(UINT msg, WPARAM w_param, LPARAM l_param, LRESULT *result);

    //IShellExtInit methods
    STDMETHODIMP Initialize(LPCITEMIDLIST folder, IDataObject* subj, HKEY key);

  private:
    void diff();
    void diff3();
    void diff_with(unsigned int num);
    void diff3_with(unsigned int num);
    void diff_later();
    STRING cut_to_length(STRING, int length = 64);
    void initialize_language();

  private:
    UINT _n_files;
    STRING* _selection;    
    DLIST<STRING>* _recent_files;
    HINSTANCE _resource;
    DWORD _language;
    HWND _hwnd;
    bool _start_menu;

    MENUITEM _diff;
    MENUITEM _diff_later;
    MENUITEM _diff_with_file;
    SUBMENU _diff_with;
    MENUITEM _clear;

    ULONG  _ref_count;
};

#endif // __diff_ext_h__
