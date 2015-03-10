#include "pch.h"
#include "win32ClipboardWrapper.h"
#include <stdio.h>

int clipput(char *toclipdata)
{
	char far *buffer;
	int bytes;

	HGLOBAL clipbuffer;

	// Set data buffer length to accomodate toclipdata
	bytes = strlen(toclipdata);

	// Peform the transfer to clipboard magic
	OpenClipboard(NULL);
	EmptyClipboard();
	clipbuffer = GlobalAlloc(GMEM_DDESHARE,bytes+1);
	buffer = (char far*)GlobalLock(clipbuffer);
	if (buffer == NULL)
		return GetLastError() * -1; // Do what you want to signal error
	strcpy(buffer,toclipdata);

	GlobalUnlock(clipbuffer);
	SetClipboardData(CF_TEXT,clipbuffer);
	CloseClipboard();

	// Return byte count
	return bytes; // non-negative value is success.
}

char *clipget(int &bytes)
	{
		int k;
		char *buffer=NULL;
		char *data=NULL;
		char empty[80]="<Clipboard is empty>";

		bytes = 0;

		// open the clipboard
		if (OpenClipboard(NULL))
		{
			HANDLE hData = GetClipboardData(CF_TEXT);
			char * buffer = (char*)GlobalLock(hData);
			GlobalUnlock(hData);
			CloseClipboard();

			// Return an error message
			if (buffer == NULL)
			{
				bytes = strlen(empty);
				data = (char *) malloc(bytes+1);
				strcpy(data,empty);
				bytes = bytes * -1;
			}
			// Return pointer to retrieved data
			else
			{
				bytes = strlen(buffer);
				data = (char *) malloc(bytes+1);
				strcpy(data,buffer);
			}
		}
		// Return an open clipboard failed message
		else
		{
			k = GetLastError();
			if (k < 0)
				bytes = k;
			else
				bytes = k * -1;
			sprintf(empty,"Error occurred opening clipboard - RC: %d",k);
			k = strlen(empty);
			data = (char *) malloc(k+1);
			strcpy(data,empty);
		}
		// Return pointer to data field allocated
		// It's up to the caller to free the storage
		return data;
	}