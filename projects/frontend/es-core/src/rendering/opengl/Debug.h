//
// Created by bkg2k on 19/04/22.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#ifdef DEBUG

  #define CHECKGL(f) (f, CheckGLErrors(#f))
  bool CheckGLErrors(const char* function);

#else

  #define CHECKGL(f) f

#endif
