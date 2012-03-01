/*
    Copyright 2004 tulipfan
    Copyright 2011-2012 Regshot Team

    This file is part of Regshot.

    Regshot is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 2.1 of the License, or
    (at your option) any later version.

    Regshot is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Regshot.  If not, see <http://www.gnu.org/licenses/>.
*/

/* This file orignal coded by tulipfan
   Change function/variable name to more proper ones and fix for x64 by tianwei
*/

#include "global.h"

// 1.8.2 move definition from global.h to this place
#define MAX_INI_SKIPITEMS    100

// setup based on regshot.ini by tulipfan (tfx)
LPSTR INI_SETUP           = "Setup";
LPSTR INI_FLAG            = "Flag";
LPSTR INI_EXTDIR          = "ExtDir";
LPSTR INI_OUTDIR          = "OutDir";
LPSTR INI_SKIPREGKEY      = "SkipRegKey";
LPSTR INI_SKIPDIR         = "SkipDir";
LPSTR INI_USELONGREGHEAD  = "UseLongRegHead";  // 1.8.1 tianwei for compatible to undoreg 1.46 again

LPTSTR lpgrszRegSkipStrings;
LPTSTR lpgrszFileSkipStrings;

LPTSTR *lprgszRegSkipStrings;
LPTSTR *lprgszFileSkipStrings;

BOOL bUseLongRegHead;  // 1.8.1 for compatibility with 1.61e5 and undoreg1.46


BOOL LoadSettingsFromIni(HWND hDlg) // tfx get ini info
{
    int     i;
    LPTSTR  lpReturn;
    BYTE    nFlag;
    DWORD   cchSection;

    TCHAR   szIniKey[17];

    szIniKey[16] = 0;  // saftey NULL char

    lprgszRegSkipStrings = MYALLOC0(MAX_INI_SKIPITEMS * sizeof(LPTSTR));
    lpgrszRegSkipStrings = MYALLOC0(MAX_INI_SECTION_CHARS * sizeof(TCHAR));
    cchSection = GetPrivateProfileSection(INI_SKIPREGKEY, lpgrszRegSkipStrings, MAX_INI_SECTION_CHARS, lpRegshotIni);
    if (0 < cchSection) {
        for (i = 0; i < MAX_INI_SKIPITEMS - 1; i++) {
            _sntprintf(szIniKey, 16, TEXT("%d%s"), i, TEXT("="));
            lpReturn = FindKeyInIniSection(lpgrszRegSkipStrings, szIniKey, cchSection, _tcslen(szIniKey));
            if (NULL != lpReturn) {
                *(lprgszRegSkipStrings + i) = lpReturn;
                //dwSnapFiles++;
            } else {
                break;
            }
        }
    }

    lprgszFileSkipStrings = MYALLOC0(MAX_INI_SKIPITEMS * sizeof(LPTSTR));
    lpgrszFileSkipStrings = MYALLOC0(MAX_INI_SECTION_CHARS * sizeof(TCHAR));
    cchSection = GetPrivateProfileSection(INI_SKIPDIR, lpgrszFileSkipStrings, MAX_INI_SECTION_CHARS, lpRegshotIni);
    if (0 < cchSection) {
        for (i = 0; i < MAX_INI_SKIPITEMS - 1; i++) {
            _sntprintf(szIniKey, 16, TEXT("%d%s"), i, TEXT("="));
            lpReturn = FindKeyInIniSection(lpgrszFileSkipStrings, szIniKey, cchSection, _tcslen(szIniKey));
            if (NULL != lpReturn) {
                *(lprgszFileSkipStrings + i) = lpReturn;
                //dwSnapFiles++;
            } else {
                break;
            }
        }
    }

    nFlag = (BYTE)GetPrivateProfileInt(INI_SETUP, INI_FLAG, 1, lpRegshotIni); // default from 0 to 1 in 1.8.2 (TEXT)
    //if (nFlag != 0)
    {
        SendMessage(GetDlgItem(hDlg, IDC_RADIO1), BM_SETCHECK, (WPARAM)(nFlag & 0x01), (LPARAM)0);
        SendMessage(GetDlgItem(hDlg, IDC_RADIO2), BM_SETCHECK, (WPARAM)((nFlag & 0x01) ^ 0x01), (LPARAM)0);
        //SendMessage(GetDlgItem(hDlg,IDC_CHECKDIR),BM_SETCHECK,(WPARAM)((nFlag&0x04)>>1),(LPARAM)0); // 1.7
        SendMessage(GetDlgItem(hDlg, IDC_CHECKDIR), BM_SETCHECK, (WPARAM)((nFlag & 0x02) >> 1), (LPARAM)0);
    }
    /*else  delete in 1.8.1
    {
        SendMessage(GetDlgItem(hDlg,IDC_RADIO1),BM_SETCHECK,(WPARAM)0x01,(LPARAM)0);
        SendMessage(GetDlgItem(hDlg,IDC_RADIO2),BM_SETCHECK,(WPARAM)0x00,(LPARAM)0);
        SendMessage(GetDlgItem(hDlg,IDC_CHECKDIR),BM_SETCHECK,(WPARAM)0x00,(LPARAM)0);
    }
    */
    // added in 1.8.1 for compatibility with undoreg1.46
    bUseLongRegHead = GetPrivateProfileInt(INI_SETUP, INI_USELONGREGHEAD, 0, lpRegshotIni) != 0 ? TRUE : FALSE;

    if (GetPrivateProfileString(INI_SETUP, INI_EXTDIR, NULL, lpExtDir, MAX_PATH, lpRegshotIni) != 0) {
        SetDlgItemText(hDlg, IDC_EDITDIR, lpExtDir);
    } else {
        SetDlgItemText(hDlg, IDC_EDITDIR, lpWindowsDirName);
    }

    if (GetPrivateProfileString(INI_SETUP, INI_OUTDIR, NULL, lpOutputpath, MAX_PATH, lpRegshotIni) != 0) {
        SetDlgItemText(hDlg, IDC_EDITPATH, lpOutputpath);
    } else {
        SetDlgItemText(hDlg, IDC_EDITPATH, lpTempPath);
    }

    SendMessage(hDlg, WM_COMMAND, (WPARAM)IDC_CHECKDIR, (LPARAM)0);

    return TRUE;
}


BOOL SaveSettingsToIni(HWND hDlg) // tfx save settings to ini
{
    BYTE    nFlag;
    LPSTR   lpString;
    HANDLE  hTest;

    // 1.8.2, someone might not want to create a regshot.ini when there isn't one. :O
    hTest = CreateFile(lpRegshotIni, GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hTest == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    CloseHandle(hTest);

    //nFlag = (BYTE)(SendMessage(GetDlgItem(hDlg,IDC_RADIO1),BM_GETCHECK,(WPARAM)0,(LPARAM)0) // 1.7
    //  |SendMessage(GetDlgItem(hDlg,IDC_RADIO2),BM_GETCHECK,(WPARAM)0,(LPARAM)0)<<1
    //  |SendMessage(GetDlgItem(hDlg,IDC_CHECKDIR),BM_GETCHECK,(WPARAM)0,(LPARAM)0)<<2);
    nFlag = (BYTE)(SendMessage(GetDlgItem(hDlg, IDC_RADIO1), BM_GETCHECK, (WPARAM)0, (LPARAM)0) |
                   SendMessage(GetDlgItem(hDlg, IDC_CHECKDIR), BM_GETCHECK, (WPARAM)0, (LPARAM)0) << 1);

    lpString = MYALLOC0(EXTDIRLEN + 4);
    //sprintf(lpString,"%s = %d",INI_FLAG,nFlag);                   // 1.7 solokey
    //WritePrivateProfileSection(INI_SETUP,lpString,lpRegshotIni);  // 1.7 solokey, can only have one key.

    // 1.8.1
    sprintf(lpString, "%d", nFlag);
    WritePrivateProfileString(INI_SETUP, INI_FLAG, lpString, lpRegshotIni);
    sprintf(lpString, "%d", bUseLongRegHead);
    WritePrivateProfileString(INI_SETUP, INI_USELONGREGHEAD, lpString, lpRegshotIni);


    if (GetDlgItemText(hDlg, IDC_EDITDIR, lpString, (EXTDIRLEN / 2)) != 0) {
        WritePrivateProfileString(INI_SETUP, INI_EXTDIR, lpString, lpRegshotIni);
    }

    if (GetDlgItemText(hDlg, IDC_EDITPATH, lpString, MAX_PATH) != 0) {
        WritePrivateProfileString(INI_SETUP, INI_OUTDIR, lpString, lpRegshotIni);
    }

    MYFREE(lpString);
    MYFREE(lpRegshotIni);
    if (NULL != lpgrszRegSkipStrings) {
        MYFREE(lpgrszRegSkipStrings);
    }
    if (NULL != lpgrszFileSkipStrings) {
        MYFREE(lpgrszFileSkipStrings);
    }
    if (NULL != lpgrszLangSection) {
        MYFREE(lpgrszLangSection);
    }

    return TRUE;
}


BOOL IsInSkipList(LPTSTR lpszString, LPTSTR lpszSkipList[])  // tfx skip the list
{
    int i;

    // todo: it seems bypass null item. But the getsetting is get all. Is it safe without the null thing? tianwei
    for (i = 0; (MAX_INI_SKIPITEMS > i) && (NULL != lpszSkipList[i]); i++) {
        if (0 == _tcsicmp(lpszString, lpszSkipList[i])) {
            return TRUE;
        }
    }
    return FALSE;
}
