/*
 * Copyright (c) 2003-2010, Sergey Zorin. All rights reserved.
 *
 * This software is distributable under the BSD license. See the terms of the
 * BSD license in the LICENSE file provided with this software.
 *
 */
#include <util/string.h>

STRING
operator+(const STRING& s1, const STRING& s2) {
  STRING tmp(s1);
  
  tmp+=s2;
  
  return tmp;
}
