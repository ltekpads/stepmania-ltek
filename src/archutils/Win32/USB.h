/* WindowsFileIO - Windows device I/O. */

#ifndef WIN32_USB_H
#define WIN32_USB_H

#include <vector>
#include <windows.h>

enum WindowsFileMode
{
	WINDOWS_READ = 1,
	WINDOWS_WRITE = 2
};

class WindowsFileIO
{
public:
	WindowsFileIO();
	~WindowsFileIO();
	bool Open( RString sPath, int iBlockSize, WindowsFileMode mode);
	bool IsOpen() const;

	/* Nonblocking read.  size must always be the same.  Returns the number of bytes
	 * read, or 0. */
	int read( void *p );
	bool write(const char* buffer, int length);
	static int read_several( const vector<WindowsFileIO *> &sources, void *p, int &actual, float timeout );

private:
	void queue_read();
	int finish_read( void *p );

	HANDLE m_Handle;
	OVERLAPPED m_Overlapped;
	char *m_pBuffer;
	int m_iBlockSize;
};

enum USBMode
{
	USB_READ = 1,
	USB_WRITE = 2,
};

inline USBMode operator&(USBMode a, USBMode b)
{
	return static_cast<USBMode>(static_cast<int>(a) & static_cast<int>(b));
}

inline USBMode operator|(USBMode a, USBMode b)
{
	return static_cast<USBMode>(static_cast<int>(a) | static_cast<int>(b));
}

/* WindowsFileIO - Windows USB I/O */
class USBDevice
{
public:
	int GetPadEvent();
	bool Open( int iVID, int iPID, int iBlockSize, int iNum, void (*pfnInit)(HANDLE), USBMode mode);
	bool IsOpen() const;

	WindowsFileIO m_IO;
};

template<typename T> struct HidReport {
	char reportType;
	T data;
};

#endif

/*
 * (c) 2002-2005 Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
