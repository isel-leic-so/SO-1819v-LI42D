
#include "stdafx.h"
#include <ctype.h>

FORCEINLINE
VOID ShowNibble(INT n) {
	putchar(n >= 10 ? n - 10 + 'A' : n + '0');
}

FORCEINLINE
VOID ShowByte(BYTE b) {
	ShowNibble(b >> 4);
	ShowNibble(b & 0xF);
	putchar(' ');
}

VOID ShowBuffer(PBYTE buf, INT size) {
	for (int i = 0; i < size; ++i) {
		ShowByte(buf[i]);
		if ((i + 1) % 16 == 0) {
			for (int ps = i - 16; ps < i; ps++)
				putchar(isprint(buf[ps]) ? buf[ps] : '.');
			putchar('\n');
		}
	}
	putchar('\n');
}