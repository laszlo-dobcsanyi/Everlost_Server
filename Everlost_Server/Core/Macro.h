#ifndef CORE_MACRO_H
#define CORE_MACRO_H

#define _WIN32_WINNT 0x0502
#define _SCL_SECURE_NO_WARNINGS

#define LOGGING
#define CHARACTERS_NUMBER 2024 + 32

inline int mod(int _value, int _modulo) { return (_value < 0 ? (_value + _modulo) % _modulo : (_value) % _modulo); };
inline int reverse(int _direction) { switch(_direction){ case 0: return 4; case 1: return 5; case 2 : return 6; case 3 : return 7; case 4: return 0; case 5: return 1; case 6: return 2; case 7: return 3; } return -1;};

#ifdef LOGGING
#include "Core\Logger.h"
#endif

#endif