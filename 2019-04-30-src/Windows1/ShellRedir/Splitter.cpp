#include "stdafx.h"

/* remove start and end spaces on string */
VOID Trim(char str[]) {
	int pi=0, pe = strlen(str)-1, po=0;

	while (str[pi] != 0 && str[pi] == ' ')  pi++; // start
	while (pe > pi &&  str[pe] == ' ')  pe--; // end

	while (pi <= pe) {
		str[po++] = str[pi++];
	}
	str[po] = 0;
}

INT SplitStr(LPSTR str, char delimiter, char strs[][1024], int maxSplits) {
	int splitIdx = 0, strIdx = 0, bufIdx = 0, startPart = 0;
	CHAR c;

	while (splitIdx < maxSplits && (c = str[strIdx++]) != 0) {
		if (c == delimiter) {
			strs[splitIdx][bufIdx] = 0;
			Trim(strs[splitIdx]);
			bufIdx = 0;
			splitIdx++;
		}
		else {
			strs[splitIdx][bufIdx++] = c;
		}
	}

	if (splitIdx < maxSplits) {
		strs[splitIdx][bufIdx] = 0;
		Trim(strs[splitIdx]);
		splitIdx++;
		
	}
	return splitIdx;
}