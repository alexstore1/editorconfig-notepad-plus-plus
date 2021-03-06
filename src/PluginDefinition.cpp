//this file is part of EditorConfig plugin for Notepad++
//
//Copyright (C)2003 Don HO <donho@altern.org>
//Copyright (C)2011 EditorConfig Team <http://editorconfig.org>
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
//

#include "PluginDefinition.hpp"
#include "menuCmdID.hpp"

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;

//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE hModule)
{
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
}

void loadConfig()
{
#ifdef UNICODE
    TCHAR fileNamet[MAX_PATH];
#endif
    char  fileName[MAX_PATH];
    HWND curScintilla;
    int which;
    int name_value_count;
    int err_num;
    editorconfig_handle eh;

    eh = editorconfig_handle_init();

    // get the file name
    ::SendMessage(nppData._nppHandle, NPPM_GETFULLCURRENTPATH, MAX_PATH,
#ifdef UNICODE
            (LPARAM)fileNamet
#else
            (LPARAM)fileName
#endif
            );
#ifdef UNICODE
    wcstombs(fileName, fileNamet, MAX_PATH);
#endif

    /* start parsing */
    if ((err_num = editorconfig_parse(fileName, eh)) != 0 &&
            /* Ignore full path error, whose error code is
             * EDITORCONFIG_PARSE_NOT_FULL_PATH */
            err_num != EDITORCONFIG_PARSE_NOT_FULL_PATH) {
        std::tstringstream err_msg;
        err_msg << TEXT("EditorConfig Error: ") << err_num;
        ::MessageBox(NULL, err_msg.str().c_str(),
                TEXT("EditorConfig"), MB_OK);
        editorconfig_handle_destroy(eh);
        return;
    }

    // Get the current scintilla
    which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0,
            (LPARAM)&which);
    if (which == -1)
        return;
    curScintilla = (which == 0) ? nppData._scintillaMainHandle :
        nppData._scintillaSecondHandle;

    struct
    {
        const char*     indent_style;
#define INDENT_SIZE_TAB (-1000) // indent_size = -1000 means indent_size = tab
        int             indent_size;
        int             tab_width;
        const char*     end_of_line;
    } ecConf; // obtained EditorConfig settings will be here

    memset(&ecConf, 0, sizeof(ecConf));

    // apply the settings
    name_value_count = editorconfig_handle_get_name_value_count(eh);

    for (int i = 0; i < name_value_count; ++i) {
        const char* name;
        const char* value;

        editorconfig_handle_get_name_value(eh, i, &name, &value);

        if (!strcmp(name, "indent_style"))
            ecConf.indent_style = value;
        else if (!strcmp(name, "tab_width"))
            ecConf.tab_width = atoi(value);
        else if (!strcmp(name, "indent_size")) {
            int     value_i = atoi(value);

            if (!strcmp(value, "tab"))
                ecConf.indent_size = INDENT_SIZE_TAB;
            else if (value_i > 0)
                ecConf.indent_size = value_i;
        }
        else if (!strcmp(name, "end_of_line"))
            ecConf.end_of_line = value;
    }

    if (ecConf.indent_style) {
        if (!strcmp(ecConf.indent_style, "tab"))
            ::SendMessage(curScintilla, SCI_SETUSETABS, (WPARAM)true, 0);
        else if (!strcmp(ecConf.indent_style, "space"))
            ::SendMessage(curScintilla, SCI_SETUSETABS, (WPARAM)false, 0);
    }
    if (ecConf.indent_size > 0) {
        ::SendMessage(curScintilla, SCI_SETINDENT,
                (WPARAM)ecConf.indent_size, 0);

        // We set the tab width here, so that this could be overrided then
        // if ecConf.tab_wdith > 0
        ::SendMessage(curScintilla, SCI_SETTABWIDTH,
                (WPARAM)ecConf.indent_size, 0);
    }

    if (ecConf.tab_width > 0)
        ::SendMessage(curScintilla, SCI_SETTABWIDTH,
                (WPARAM)ecConf.tab_width, 0);

    if (ecConf.indent_size == INDENT_SIZE_TAB)
        // set indent_size to tab_width here
        ::SendMessage(curScintilla, SCI_SETINDENT,
                (WPARAM)::SendMessage(curScintilla, SCI_GETTABWIDTH, 0, 0), 0);

    // set eol
    if (ecConf.end_of_line) {
        if (!strcmp(ecConf.end_of_line, "lf"))
            ::SendMessage(curScintilla, SCI_SETEOLMODE,
                    (WPARAM)SC_EOL_LF, 0);
        else if (!strcmp(ecConf.end_of_line, "cr"))
            ::SendMessage(curScintilla, SCI_SETEOLMODE,
                    (WPARAM)SC_EOL_CR, 0);
        else if (!strcmp(ecConf.end_of_line, "crlf"))
            ::SendMessage(curScintilla, SCI_SETEOLMODE,
                    (WPARAM)SC_EOL_CRLF, 0);
    }

    editorconfig_handle_destroy(eh);
}

void onReloadEditorConfig()
{
    loadConfig();
}
//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{

    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );
    setCommand(0, TEXT("Reload EditorConfig for this file"),
            onReloadEditorConfig, NULL, false);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
    // Don't forget to deallocate your shortcut here
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
    if (index >= nbFunc)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

