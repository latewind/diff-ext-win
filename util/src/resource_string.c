/*
 * Copyright (c) 2003-2010, Sergey Zorin. All rights reserved.
 *
 * This software is distributable under the BSD license. See the terms of the
 * BSD license in the LICENSE file provided with this software.
 *
 */
#include <util/resource_string.h>

void
load_resource_string(HINSTANCE  resource, UINT string_id, TCHAR* string, int length, const TCHAR* default_value) {
  int resource_string_length;
  
  resource_string_length = LoadString(resource, string_id, string, length);
  
  if(resource_string_length == 0) {
    resource_string_length = LoadString(GetModuleHandle(0), string_id, string, length);
    
    if(resource_string_length == 0) {
      lstrcpy(string, default_value);
    }
  }
}

