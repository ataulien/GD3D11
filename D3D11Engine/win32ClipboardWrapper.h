// -- http://www.daniweb.com/software-development/c/code/217173    
#pragma once
#include <windows.h>

// ---------------------------------------------------------------------------
// Copy input to Windows clipboard
// Data lines must be terminated by the CR LF pair (0xD,0xA)
// data in: line1CRLFline2CRLFline3CRLF --- Caller must format
// "this is a line\n" is not acceptable,
// "this is a line\r\n" is acceptable.
// If clipboard data shows square empty boxes at line ends in Windows,
// it is because lines are terminated by \n only.
int clipput(char *toclipdata);


// Return pointer to clipboard data and set bytes returned value
// If error occurs, set up error message, point to it, set bytes negative
// Whether successful or not, the caller SHOULD free the data
char *clipget(int &bytes);
