/*
 * Copyright (c) 2003-2010, Sergey Zorin. All rights reserved.
 *
 * This software is distributable under the BSD license. See the terms
 * of the BSD license in the LICENSE file provided with this software.
 *
 */
#include <stdio.h>

#include <windows.h>
#include <tchar.h>

#include <shlguid.h>
#include <olectl.h>
#include <objidl.h>

#include <objbase.h>
#include <initguid.h>

#include <log/log.h>
#include <log/log_message.h>
#include <log/file_sink.h>

#include <debug/trace.h>

#include "server.h"
#include "class_factory.h"

#include <VersionHelpers.h>



 // registry key util struct
typedef struct {
	LPCTSTR subkey;
	LPCTSTR name;
	LPCTSTR value;
}
REGSTRUCT, *LPREGSTRUCT;

SERVER* SERVER::_instance = 0;
static HINSTANCE server_instance; // Handle to this DLL itself.

DEFINE_GUID(CLSID_DIFF_EXT, 0xA0482097, 0xC69D, 0x4DEC, 0x8A, 0xB6, 0xD3, 0xA2, 0x59, 0xAC, 0xC1, 0x51);

static LPCTSTR no[] = {
  TEXT("1"), TEXT("2"), TEXT("3"), TEXT("4"), TEXT("5"), TEXT("6"), TEXT("7"), TEXT("8"), TEXT("9"),
  TEXT("10"), TEXT("11"), TEXT("12"), TEXT("13"), TEXT("14"), TEXT("15"), TEXT("16"), TEXT("17"), TEXT("18"), TEXT("19"),
  TEXT("20"), TEXT("21"), TEXT("22"), TEXT("23"), TEXT("24"), TEXT("25"), TEXT("26"), TEXT("27"), TEXT("28"), TEXT("29"),
  TEXT("30"), TEXT("31"), TEXT("32"), TEXT("33"), TEXT("34"), TEXT("35"), TEXT("36"), TEXT("37"), TEXT("38"), TEXT("39"),
  TEXT("40"), TEXT("41"), TEXT("42"), TEXT("43"), TEXT("44"), TEXT("45"), TEXT("46"), TEXT("47"), TEXT("48"), TEXT("49"),
  TEXT("50"), TEXT("51"), TEXT("52"), TEXT("53"), TEXT("54"),TEXT("55"), TEXT("56"), TEXT("57"), TEXT("58"), TEXT("59"),
  TEXT("60"), TEXT("61"), TEXT("62"), TEXT("63"), TEXT("64")
};

static const unsigned int max_history_size = sizeof(no) / sizeof(no[0]);

STDAPI
DllCanUnloadNow(void) {
	HRESULT ret = S_FALSE;

	if (SERVER::instance()->refference_count() == 0) {
		ret = S_OK;
	}

	return ret;
}

extern "C" int APIENTRY
DllMain(HINSTANCE instance, DWORD reason, LPVOID /* reserved */) {
	//  char str[1024];
	//  char* reason_string[] = {"DLL_PROCESS_DETACH", "DLL_PROCESS_ATTACH", "DLL_THREAD_ATTACH", "DLL_THREAD_DETACH"};
	//  sprintf(str, "instance: %x; reason: '%s'", instance, reason_string[reason]);
	//  MessageBox(0, str, TEXT("Info"), MB_OK);  
	switch (reason) {
	case DLL_PROCESS_ATTACH:
		server_instance = instance;
		SERVER::instance()->save_history();
		break;

	case DLL_PROCESS_DETACH:
		SERVER::instance()->save_history();
		break;
	}

	return 1;
}

STDAPI
DllGetClassObject(REFCLSID rclsid, REFIID riid, void** class_object) {
	HRESULT ret = CLASS_E_CLASSNOTAVAILABLE;
	*class_object = 0;

	if (IsEqualIID(rclsid, CLSID_DIFF_EXT)) {
		CLASS_FACTORY* pcf = new CLASS_FACTORY();

		ret = pcf->QueryInterface(riid, class_object);
	}

	return ret;
}

STDAPI
DllRegisterServer() {
	return SERVER::instance()->do_register();
}

STDAPI
DllUnregisterServer() {
	return SERVER::instance()->do_unregister();
}

SERVER::SERVER() : _refference_count(0), _recent_files(0), _file_sink(0) {
}

SERVER::~SERVER() {
	if (_recent_files != 0) {
		delete _recent_files;
	}

	if (_file_sink != 0) {
		delete _file_sink;
	}
	//  MessageBox(0, TEXT("~SERVER"), TEXT("info"), MB_OK);
}

HINSTANCE
SERVER::handle() const {
	return server_instance;
}

void
SERVER::lock() {
	InterlockedIncrement(&_refference_count);
}

void
SERVER::release() {
	InterlockedDecrement(&_refference_count);
}

DLIST<STRING>*
SERVER::recent_files() {
	HKEY key;
	DWORD history_size = 32;
	DWORD len;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Z\\diff-ext"), 0, KEY_READ, &key) == ERROR_SUCCESS) {
		len = sizeof(DWORD);
		RegQueryValueEx(key, TEXT("history_size"), 0, NULL, (BYTE*)&history_size, &len);

		RegCloseKey(key);
	}

	if (history_size > max_history_size) {
		history_size = max_history_size;
	}

	_history_size = history_size;

	DLIST<STRING>* new_history = new DLIST<STRING>();

	if (_recent_files != 0) {
		unsigned int n = history_size;
		DLIST<STRING>::ITERATOR i = _recent_files->head();

		if (n > _recent_files->count()) {
			n = _recent_files->count();
		}

		while ((!i.done()) && (n > 0)) {
			new_history->append((*i)->data());
			i++;
			n--;
		}

		delete _recent_files;
	}
	else { // read history from the registry
		if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Z\\diff-ext\\history"), 0, KEY_READ, &key) == ERROR_SUCCESS) {
			TCHAR file[MAX_PATH];
			bool stop = false;

			for (unsigned int i = 0; !stop && (i < history_size); i++) {
				len = MAX_PATH;
				if (RegQueryValueEx(key, no[i], 0, 0, (BYTE*)file, &len) == ERROR_SUCCESS) {
					//~ char str[256];
					//~ sprintf(str, "SERVER::recent_files: len=%d", len);
					//~ LOG::instance()->debug(LOG_MESSAGE(str));
					if (len > 0) {
						new_history->append(file);
					}
				}
				else {
					stop = true;
				}
			}

			RegCloseKey(key);
		}
	}

	_recent_files = new_history;

	return _recent_files;
}

DLIST<STRING>* SERVER::list_history_files()
{
	HKEY key;
	DWORD history_size = 32;
	DWORD len;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Z\\diff-ext"), 0, KEY_READ, &key) == ERROR_SUCCESS) {
		len = sizeof(DWORD);
		RegQueryValueEx(key, TEXT("history_size"), 0, NULL, (BYTE*)&history_size, &len);

		RegCloseKey(key);
	}

	if (history_size > max_history_size) {
		history_size = max_history_size;
	}

	_history_size = history_size;
	DLIST<STRING>* new_history = new DLIST<STRING>();
	delete _recent_files;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Z\\diff-ext\\history"), 0, KEY_READ, &key) == ERROR_SUCCESS) {
		TCHAR file[MAX_PATH * 2];
		bool stop = false;

		for (unsigned int i = 0; !stop && (i < history_size); i++) {
			len = MAX_PATH * 2;
			long result = RegQueryValueEx(key, no[i], 0, 0, (BYTE*)file, &len);
			if (result == ERROR_SUCCESS) {
				//~ char str[256];
				//~ sprintf(str, "SERVER::recent_files: len=%d", len);
				//~ LOG::instance()->debug(LOG_MESSAGE(str));
				if (len > 0) {
					new_history->append(file);
				}
			}
			else {
				stop = true;
			}
		}

		RegCloseKey(key);
	}

	_recent_files = new_history;

	return _recent_files;
}

bool
SERVER::tree_way_compare_supported() {
	bool result = false;
	HKEY key;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Z\\diff-ext"), 0, KEY_READ, &key) == ERROR_SUCCESS) {
		DWORD three_way_compare_supported;
		DWORD hlen = sizeof(three_way_compare_supported);

		if (RegQueryValueEx(key, TEXT("3way_compare_supported"), 0, NULL, (BYTE*)(&three_way_compare_supported), &hlen) != ERROR_SUCCESS) {
			three_way_compare_supported = 0;
		}

		result = (three_way_compare_supported != 0);
	}

	return result;
}

bool
SERVER::persistent_selection() {
	bool result = false;
	HKEY key;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Z\\diff-ext"), 0, KEY_READ, &key) == ERROR_SUCCESS) {
		DWORD persistent_selection;
		DWORD hlen = sizeof(persistent_selection);

		if (RegQueryValueEx(key, TEXT("persistent_selection"), 0, NULL, (BYTE*)(&persistent_selection), &hlen) != ERROR_SUCCESS) {
			persistent_selection = 0;
		}

		result = (persistent_selection != 0);
	}

	return result;
}

void
SERVER::save_history() const {
	HKEY key;
	DWORD len;

	if (_recent_files != 0) {
		if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Z\\diff-ext\\history"), 0, KEY_SET_VALUE, &key) == ERROR_SUCCESS) {
			len = MAX_PATH;
			int n = 0;

			DLIST<STRING>::ITERATOR i = _recent_files->head();

			while (!i.done()) {
				STRING str = (*i)->data();
				LPTSTR c_str = str;
				if (RegSetValueEx(key, no[n], 0, REG_SZ, (const BYTE*)c_str, str.size()) != ERROR_SUCCESS) {
					//          TRACE trace(__FUNCTION__, __FILE__, __LINE__, 4);
					HKEY history_entry;
					if (RegCreateKeyEx(key, no[n], 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &history_entry, 0) == ERROR_SUCCESS) {
						if (RegSetValueEx(key, no[n], 0, REG_SZ, (const BYTE*)c_str, str.size()) != ERROR_SUCCESS) {
							LPTSTR message;
							FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0,
								GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
								(LPTSTR)&message, 0, 0);
							MessageBox(0, message, TEXT("Save history after expand"), MB_OK | MB_ICONINFORMATION);

							LocalFree(message);
						}
					}
					else {
						LPTSTR message;
						FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0,
							GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
							(LPTSTR)&message, 0, 0);
						MessageBox(0, message, TEXT("Save history"), MB_OK | MB_ICONINFORMATION);

						LocalFree(message);
					}
				}
				n++;
				i++;
			}

			while (RegDeleteValue(key, no[n]) == ERROR_SUCCESS) {
				n++;
			}

			RegCloseKey(key);
		}
	}
}

HRESULT
SERVER::do_register() {
	TCHAR   class_id[MAX_PATH];
	LPWSTR  tmp_guid;
	HRESULT ret = SELFREG_E_CLASS;

	if (StringFromIID(CLSID_DIFF_EXT, &tmp_guid) == S_OK) {
#ifdef UNICODE    
		_tcsncpy(class_id, tmp_guid, MAX_PATH);
#else
		wcstombs(class_id, tmp_guid, MAX_PATH);
#endif
		CoTaskMemFree((void*)tmp_guid);

		TCHAR    subkey[MAX_PATH];
		TCHAR    server_path[MAX_PATH];
		HKEY     key;
		LRESULT  result = NOERROR;
		DWORD    dwDisp;

		GetModuleFileName(SERVER::instance()->handle(), server_path, MAX_PATH);

		REGSTRUCT entry[] = {
		  {TEXT("CLSID\\%s"), 0, TEXT("diff-ext")},
		  {TEXT("CLSID\\%s\\InProcServer32"), 0, TEXT("%s")},
		  {TEXT("CLSID\\%s\\InProcServer32"), TEXT("ThreadingModel"), TEXT("Apartment")}
		};

		for (unsigned int i = 0; (i < sizeof(entry) / sizeof(entry[0])) && (result == NOERROR); i++) {
			_stprintf(subkey, entry[i].subkey, class_id);
			result = RegCreateKeyEx(HKEY_CLASSES_ROOT, subkey, 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &key, &dwDisp);

			if (result == NOERROR) {
				TCHAR szData[MAX_PATH];

				_stprintf(szData, entry[i].value, server_path);

				result = RegSetValueEx(key, entry[i].name, 0, REG_SZ, (LPBYTE)szData, lstrlen(szData) * sizeof(TCHAR));
			}

			RegCloseKey(key);
		}

		if (result == NOERROR) {
			result = RegCreateKeyEx(HKEY_CLASSES_ROOT, TEXT("*\\shellex\\ContextMenuHandlers\\diff-ext"), 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &key, &dwDisp);

			if (result == NOERROR) {
				TCHAR szData[MAX_PATH];

				//if necessary, create the value string
				_stprintf(szData, TEXT("%s"), class_id);

				result = RegSetValueEx(key, 0, 0, REG_SZ, (LPBYTE)szData, lstrlen(szData) * sizeof(TCHAR));

				RegCloseKey(key);

				//If running on NT, register the extension as approved.

				// NT needs to have shell extensions "approved".

				// if (!IsWindows10OrGreater())
				// {
				// }

				bool m_bIsWin7 = true;
				if (m_bIsWin7) {
					result = RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"), 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &key, &dwDisp);

					if (result == NOERROR) {
						TCHAR szData1[MAX_PATH];

						lstrcpy(szData1, TEXT("diff-ext"));

						result = RegSetValueEx(key, class_id, 0, REG_SZ, (LPBYTE)szData1, lstrlen(szData1) * sizeof(TCHAR));

						RegCloseKey(key);

						ret = S_OK;
					}
					else if (result == ERROR_ACCESS_DENIED) {
						TCHAR msg[] = TEXT("Warning! You have unsuficient rights to write to a specific registry key.\n")
							TEXT("The application may work anyway, but it is adsvised to register this module ")
							TEXT("again while having administrator rights.");

						MessageBox(0, msg, TEXT("Warning"), MB_ICONEXCLAMATION);

						ret = S_OK;
					}
				}
				else {
					ret = S_OK;
				}
			}
		}
	}

	return ret;
}

HRESULT
SERVER::do_unregister() {
	TCHAR class_id[MAX_PATH];
	LPWSTR tmp_guid;
	HRESULT ret = SELFREG_E_CLASS;

	if (StringFromIID(CLSID_DIFF_EXT, &tmp_guid) == S_OK) {
#ifdef UNICODE    
		_tcsncpy(class_id, tmp_guid, MAX_PATH);
#else
		wcstombs(class_id, tmp_guid, MAX_PATH);
#endif
		CoTaskMemFree((void*)tmp_guid);

		LRESULT result = NOERROR;
		TCHAR subkey[MAX_PATH];

		REGSTRUCT entry[] = {
		  {TEXT("CLSID\\%s\\InProcServer32"), 0, 0},
		  {TEXT("CLSID\\%s"), 0, 0}
		};

		for (unsigned int i = 0; (i < sizeof(entry) / sizeof(entry[0])) && (result == NOERROR); i++) {
			_stprintf(subkey, entry[i].subkey, class_id);
			result = RegDeleteKey(HKEY_CLASSES_ROOT, subkey);
		}

		if (result == NOERROR) {
			result = RegDeleteKey(HKEY_CLASSES_ROOT, TEXT("*\\shellex\\ContextMenuHandlers\\diff-ext"));

			if (result == NOERROR) {
				//If running on NT, register the extension as approved.
				// OSVERSIONINFO  osvi;

				// osvi.dwOSVersionInfoSize = sizeof(osvi);
				// GetVersionEx(&osvi);

				// NT needs to have shell extensions "approved".
				if (true) {
					HKEY key;

					RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"), 0, KEY_ALL_ACCESS, &key);

					result = RegDeleteValue(key, class_id);

					RegCloseKey(key);

					if (result == ERROR_SUCCESS) {
						ret = S_OK;
					}
				}
				else {
					ret = S_OK;
				}
			}
		}
	}

	return ret;
}
