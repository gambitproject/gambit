//
// FILE: ZFortify.cpp - A fortified shell for new and delete.
//
// $Id$
//


//     The  minimum  you  need  to  do for ZFortify is to define the symbol
// ZFORTIFY,  and  link  zfortify.o.   Each source file should also include
// "zfortify.hpp",  but  this  isn't strictly necessary.  If a file doesn't
// #include  "Fortify.hpp",  then it's allocations will still be fortified,
// however  you  will not have any source-code details in any of the output
// messages  (this will be the case for all libraries, etc, unless you have
// the source for the library and can recompile it with Fortify).
//     If  you  do  not  have  stdout  available,  you  may  wish to set an
// alternate  output  function,  or  turn  on  the  AUTOMATIC_LOGFILE.  See
// ZFortify_SetOutputFunc() and AUTOMATIC_LOGFILE, below.
//
// None  of  the  functions  in  this  file should be called directly; they
// should be called through the macros defined in ZFortify.hpp.

#ifdef ZFORTIFY
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#define  ZFORTIFY_CPP   // So ZFortify.hpp knows to not define the ZFortify macros
#include "zfortify.hpp"

// the user's options
extern wxDebugMsg(const char *fmt, ...);

#include "ufortify.hpp"   

struct Header
{
	char          *File;     // The sourcefile of the caller 
	unsigned long  Line;     // The sourceline of the caller
	size_t         Size;     // The size of the new'd block
	struct Header *Prev;     // List pointers
	struct Header *Next;     //
	int            Scope;    // Scope level this block was allocated in
	int            Checksum; // For validating the Header structure; see ChecksumHeader()
};

// Local prototypes
static int  CheckBlock(struct Header *h, char *file, unsigned long line);
static int  CheckFortification(unsigned char *ptr, unsigned char value, size_t size);
static void SetFortification(unsigned char *ptr, unsigned char value, size_t size);
static void OutputFortification(unsigned char *ptr, unsigned char value, size_t size);
static int  IsHeaderValid(struct Header *h);
static void MakeHeaderValid(struct Header *h);
static int  ChecksumHeader(struct Header *h);
static int  IsOnList(struct Header *h);
static void OutputMemory(struct Header *h);
static void OutputHeader(struct Header *h);
static void OutputLastVerifiedPoint(void);

// The default output function
static void st_DefaultOutput(const char *String)
{
	printf(String);
}

static struct Header *st_Head = 0; // Head of alloc'd memory list 
static ZFortify_OutputFuncPtr  st_Output = st_DefaultOutput; // Output function for errors 
static char st_Buffer[256];       // Temporary buffer for sprintf's 
static int st_Disabled = 0;       // If true, ZFortify is inactive 
static int st_NewFailRate = 0;    // % of the time to false fail new

static char          *st_LastVerifiedFile = "unknown";
static unsigned long  st_LastVerifiedLine = 0;

static char	         *st_Delete_File = 0;
static unsigned long  st_Delete_Line = 0;

static int            st_Scope = 0;

// operator new() - Allocates a block of memory, with extra bits for
//                  misuse protection/detection. 
// Features:
//     +  Adds the new'd memory onto ZFortify's own private list.
//        (With a checksum'd header to detect corruption of the memory list)
//     +  Places sentinals on either side of the user's memory with
//        known data in them, to detect use outside of the bounds
//        of the block
//     +  Initializes the new'd memory to some "nasty" value, so code
//        can't rely on it's contents.
//     +  Can check all sentinals on every NEW.
//     +  Can generate a warning message on a NEW fail.
//     +  Can randomly "fail" at a set fail rate
void *ZFORTIFY_STORAGE
operator new(size_t size) 
{ 
	return operator new(size, "new", 0); 
}

#ifdef ZFORTIFY_PROVIDE_ARRAY_NEW

void *ZFORTIFY_STORAGE
operator new[](size_t size) 
{ 
	return operator new(size, "new[]", 0); 
}

void *ZFORTIFY_STORAGE
operator new[](size_t size, char *file, unsigned long line)
{
	return operator new(size, file, line);
}

#endif // ZFORTIFY_PROVIDE_ARRAY_NEW

void *ZFORTIFY_STORAGE
operator new(size_t size, char *file, unsigned long line)
{
	unsigned char *ptr;
	struct Header *h;

	ZFORTIFY_LOCK();

	if(st_Disabled)
	{
		ptr = (unsigned char *)malloc(size);
		ZFORTIFY_UNLOCK();
		return(ptr);
	}

#ifdef CHECK_ALL_MEMORY_ON_NEW
	ZFortify_CheckAllMemory(file, line);
#endif  

	if(size == 0)
	{
#ifdef WARN_ON_ZERO_NEW
		sprintf(st_Buffer, "\nZFortify: %s.%ld\n         new(0) attempted failed\n",
						   file, line);
		st_Output(st_Buffer);
#endif

		ZFORTIFY_UNLOCK();
		return(0);       
	}

	if(st_NewFailRate > 0)
	{
		if(rand() % 100 < st_NewFailRate)
		{
#ifdef WARN_ON_FALSE_FAIL
			sprintf(st_Buffer, "\nZFortify: %s.%ld\n         new(%ld) \"false\" failed\n",
							   file, line, (unsigned long)size);
			st_Output(st_Buffer);
#endif
			ZFORTIFY_UNLOCK();
			return(0);
		}
	}
  
#ifdef WARN_ON_SIZE_T_OVERFLOW               
	{
		size_t private_size = sizeof(struct Header) 
							+ ZFORTIFY_BEFORE_SIZE + size + ZFORTIFY_AFTER_SIZE;

		If(private_size < size) /* Check to see if the added baggage is larger than size_t */
		{
			sprintf(st_Buffer, "\nZFortify: %s.%ld\n         new(%ld) has overflowed size_t.\n",
								file, line, (unsigned long)size);
			st_Output(st_Buffer);
			ZFORTIFY_UNLOCK();
			return(0);
		}                              
	}
#endif                              

	// new the memory, including the space for the header 
  	// and fortification buffers (well, we fake it with malloc).
	ptr = (unsigned char *)malloc(sizeof(struct Header) + ZFORTIFY_BEFORE_SIZE + size + ZFORTIFY_AFTER_SIZE);
	if(!ptr)
	{
#ifdef WARN_ON_NEW_FAIL
		sprintf(st_Buffer, "\nZFortify: %s.%ld\n         new(%ld) failed\n",
							file, line, (unsigned long)size);
		st_Output(st_Buffer);
#endif

		ZFORTIFY_UNLOCK();
		return(0);       
	}

	// Initialize and validate the header
	h = (struct Header *)ptr;

	h->Size  = size;
  
	h->File  = file;
	h->Line  = line;  

	h->Next  = st_Head;
	h->Prev  = 0;
	h->Scope = st_Scope;

	if(st_Head)
	{
		st_Head->Prev = h;
		MakeHeaderValid(st_Head);  
	}

	st_Head = h;
  
	MakeHeaderValid(h);

	// Initialize the fortifications
	SetFortification(ptr + sizeof(struct Header), ZFORTIFY_BEFORE_VALUE, ZFORTIFY_BEFORE_SIZE);
	SetFortification(ptr + sizeof(struct Header) + ZFORTIFY_BEFORE_SIZE + size,
					 ZFORTIFY_AFTER_VALUE, ZFORTIFY_AFTER_SIZE);

#ifdef FILL_ON_NEW 
	// Fill the actual user memory
	SetFortification(ptr + sizeof(struct Header) + ZFORTIFY_BEFORE_SIZE, FILL_ON_NEW_VALUE, size);
#endif

	ZFORTIFY_UNLOCK();

	// We return the address of the user's memory, not the start of the block,
	// which points to our magic cookies
	return(ptr + sizeof(struct Header) + ZFORTIFY_BEFORE_SIZE);
}

// operator delete() - This delete must be used for all memory allocated with
//                     the ZFortify new()
//
//   Features:
//     + Pointers are validated before attempting a delete - the pointer
//       must point to a valid new'd bit of memory.
//     + Detects attempts at deleting the same block of memory twice
//     + Can clear out memory as it is freed, to prevent code from using
//       the memory after it's been freed.
//     + Checks the sentinals of the memory being freed.
//     + Can check the sentinals of all memory.
int ZFORTIFY_STORAGE
ZFortify_PreDelete(char *file, unsigned long line)
{
	// Don't change the delete source code pointer if we already have some information,
	// cause if we're doing a delete inside a delete, the highest level delete will 
	// probably be the most useful.
	if(st_Delete_File == 0)
	{
		st_Delete_File = file;
		st_Delete_Line = line;   
	}	
	return 1;
}
 
void ZFORTIFY_STORAGE
operator delete(void *uptr)
{
	unsigned char *ptr = (unsigned char *)uptr - sizeof(struct Header) - ZFORTIFY_BEFORE_SIZE;
	struct Header *h = (struct Header *)ptr;
	char *file;
	unsigned long line;    

    // It is defined to be harmless to delete 0
	if(uptr == 0)
		return;

	ZFORTIFY_LOCK();

	if(st_Disabled)
	{
		free(uptr);
		ZFORTIFY_UNLOCK();
		return;
	}

	if(st_Delete_File)
	{
		file = st_Delete_File;
		line = st_Delete_Line;
	
		st_Delete_File = 0;
	}
	else
	{
		file = "delete";
		line = 0;
	}

#ifdef CHECK_ALL_MEMORY_ON_DELETE
	ZFortify_CheckAllMemory(file, line);
#endif  

#ifdef PARANOID_DELETE
	if(!IsOnList(h))
	{
		sprintf(st_Buffer,
				"\nZFortify: %s.%ld\n         Invalid pointer, corrupted header, or possible free twice\n",
				file, line);
		st_Output(st_Buffer);
		OutputLastVerifiedPoint();
		goto fail;
	}
#endif

	if(!CheckBlock(h, file, line))
		goto fail;
	
	// Remove the block from the list
	if(h->Prev)
	{
		if(!CheckBlock(h->Prev, file, line))
			goto fail;
	  
		h->Prev->Next = h->Next;
		MakeHeaderValid(h->Prev);
	}
	else
		st_Head = h->Next;

	if(h->Next)
	{
		if(!CheckBlock(h->Next, file, line))
			goto fail;

		h->Next->Prev = h->Prev;
		MakeHeaderValid(h->Next);    
	}    

#ifdef FILL_ON_DELETE
	// Nuke out all memory that is about to be freed
	SetFortification(ptr, FILL_ON_DELETE_VALUE, 
					 sizeof(struct Header) + ZFORTIFY_BEFORE_SIZE + h->Size + ZFORTIFY_AFTER_SIZE);
#endif               

	// And do the actual free
	free(ptr);
	ZFORTIFY_UNLOCK();
	return;

fail:
	sprintf(st_Buffer, "         delete(%p) failed\n", uptr);
	st_Output(st_Buffer);  
	ZFORTIFY_UNLOCK();
}

// ZFortify_CheckPointer() - Returns true if the uptr points to a valid
// piece of ZFortify_new()'d memory. The memory must be on the new'd
// list, and it's sentinals must be in tact.
// If anything is wrong, an error message is issued.
//
//   (Note - if ZFortify is disabled, this function always returns true).
int ZFORTIFY_STORAGE
ZFortify_CheckPointer(void *uptr, char *file, unsigned long line)
{
	unsigned char *ptr = (unsigned char *)uptr - sizeof(struct Header) - ZFORTIFY_BEFORE_SIZE;
	int r;
  
	if(st_Disabled)
		return(1);

	ZFORTIFY_LOCK();

	if(!IsOnList((struct Header *)ptr))
	{
		sprintf(st_Buffer, "\nZFortify: %s.%ld\n         Invalid pointer or corrupted header detected (%p)\n",
							file, line, uptr);
		st_Output(st_Buffer);
		ZFORTIFY_UNLOCK();
		return(0);
	}

	r = CheckBlock((struct Header *)ptr, file, line);
	ZFORTIFY_UNLOCK();
	return r;
}

// ZFortify_SetOutputFunc(ZFortify_OutputFuncPtr Output) - Sets the function used to
// output all error and diagnostic messages by ZFortify. The output function 
// takes a single unsigned char * argument, and must be able to handle newlines.
//     The function returns the old pointer.
ZFortify_OutputFuncPtr ZFORTIFY_STORAGE
ZFortify_SetOutputFunc(ZFortify_OutputFuncPtr Output)
{
	ZFortify_OutputFuncPtr Old = st_Output;

	st_Output = Output;
  
	return(Old);
}

// ZFortify_SetNewFailRate(int Percent) - ZFortify_new() will make the
// new attempt fail this Percent of the time, even if the memory is
// available. Useful to "stress-test" an application. Returns the old
//  value. The fail rate defaults to 0.
int ZFORTIFY_STORAGE
ZFortify_SetNewFailRate(int Percent)
{
	int Old = st_NewFailRate;
  
	st_NewFailRate = Percent;
  
	return(Old);
}

 
// ZFortify_CheckAllMemory() - Checks the sentinals of all new'd memory.
// Returns the number of blocks that failed. 
//
//  (If ZFortify is disabled, this function always returns 0).
int ZFORTIFY_STORAGE
ZFortify_CheckAllMemory(char *file, unsigned long line)
{
	struct Header *curr = st_Head;
	int count = 0;

	if(st_Disabled)
		return(0);

	ZFORTIFY_LOCK();

	while(curr)
	{
		if(!CheckBlock(curr, file, line))
			count++;

		curr = curr->Next;      
	}
  
	if(count == 0)
	{
		st_LastVerifiedFile = file;
		st_LastVerifiedLine = line;
	}	

	ZFORTIFY_UNLOCK();
	return(count);
}

// ZFortify_EnterScope - enters a new ZFortify scope level. 
// returns the new scope level.
int ZFORTIFY_STORAGE
ZFortify_EnterScope(char */*file*/, unsigned long /*line*/)
{
	return(++st_Scope);
}

// ZFortify_LeaveScope - leaves a ZFortify scope level,
// also prints a memory dump of all non-freed memory that was allocated
// during the scope being exited.
int ZFORTIFY_STORAGE
ZFortify_LeaveScope(char *file, unsigned long line)
{
	struct Header *curr = st_Head;
	int count = 0;
	unsigned long size = 0;

	if(st_Disabled)
		return(0);

	ZFORTIFY_LOCK();

	st_Scope--;
	while(curr)
	{
		if(curr->Scope > st_Scope)
		{
			if(count == 0)
			{
				sprintf(st_Buffer, "\nZFortify: Memory Dump at %s.%ld\n", file, line);
				st_Output(st_Buffer);
				OutputLastVerifiedPoint();
				sprintf(st_Buffer, "%11s %8s %s\n", "Address", "Size", "Allocator");
				st_Output(st_Buffer);
			}	
								
			OutputHeader(curr);
			count++;
			size += curr->Size;
		}

		curr = curr->Next;      
	}

	if(count)
	{
		sprintf(st_Buffer, "%11s %8ld bytes overhead\n", "and",
						(unsigned long)(count * (sizeof(struct Header) + ZFORTIFY_BEFORE_SIZE + ZFORTIFY_AFTER_SIZE)));
						st_Output(st_Buffer);

		sprintf(st_Buffer,"%11s %8ld bytes in %d blocks\n", "total", size, count);
		st_Output(st_Buffer);
	}

	ZFORTIFY_UNLOCK();
	return(count);
}

// ZFortify_OutputAllMemory() - Outputs the entire list of currently
// new'd memory. For each new'd block is output it's Address,
// Size, and the SourceFile and Line that allocated it.
//
// If there is no memory on the list, this function outputs nothing.
//
// It returns the number of blocks on the list, unless ZFortify has been
// disabled, in which case it always returns 0.
int ZFORTIFY_STORAGE
ZFortify_OutputAllMemory(char *file, unsigned long line)
{
	struct Header *curr = st_Head;
	int count = 0;
	unsigned long size = 0;

	if(st_Disabled)
		return(0);

	ZFORTIFY_LOCK();

	if(curr)
	{
		sprintf(st_Buffer, "\nZFortify: Memory Dump at %s.%ld\n", file, line);
		st_Output(st_Buffer);
		OutputLastVerifiedPoint();
		sprintf(st_Buffer, "%11s %8s %s\n", "Address", "Size", "Allocator");
		st_Output(st_Buffer);
								
		while(curr)
		{
			OutputHeader(curr);
			count++;
			size += curr->Size;
			curr = curr->Next;      
		}
					 
		sprintf(st_Buffer, "%11s %8ld bytes overhead\n", "and", 
			(unsigned long)(count * (sizeof(struct Header) + ZFORTIFY_BEFORE_SIZE + ZFORTIFY_AFTER_SIZE)));
		st_Output(st_Buffer);

		sprintf(st_Buffer,"%11s %8ld bytes in %d blocks\n", "total", size, count);  
		st_Output(st_Buffer);
	}
  
	ZFORTIFY_UNLOCK();
	return(count);
}

// ZFortify_DumpAllMemory(Scope) - Outputs the entire list of currently
// new'd memory within the specified scope. For each new'd block is output
// it's Address, Size, the SourceFile and Line that allocated it, a hex dump
// of the contents of the memory and an ascii dump of printable characters.
//
// If there is no memory on the list, this function outputs nothing.
//
// It returns the number of blocks on the list, unless ZFortify has been
// disabled, in which case it always returns 0.
int ZFORTIFY_STORAGE
ZFortify_DumpAllMemory(int scope, char *file, unsigned long line)
{
	struct Header *curr = st_Head;
	int count = 0;
	unsigned long size = 0;

	if(st_Disabled)
		return(0);

	ZFORTIFY_LOCK();

	while(curr)
	{
		if(curr->Scope >= scope)
		{
			if(count == 0)
			{
				sprintf(st_Buffer, "\nZFortify: Memory Dump at %s.%ld\n", file, line);
				st_Output(st_Buffer);
				OutputLastVerifiedPoint();
				sprintf(st_Buffer, "%11s %8s %s\n", "Address", "Size", "Allocator");
				st_Output(st_Buffer);
			}

			OutputHeader(curr);
			OutputMemory(curr);
			st_Output("\n");
			count++;
			size += curr->Size;
		}

		curr = curr->Next;
	}

	if(count)
	{
		sprintf(st_Buffer, "%11s %8ld bytes overhead\n", "and",
						(unsigned long)(count * (sizeof(struct Header) + ZFORTIFY_BEFORE_SIZE + ZFORTIFY_AFTER_SIZE)));
						st_Output(st_Buffer);

		sprintf(st_Buffer,"%11s %8ld bytes in %d blocks\n", "total", size, count);
		st_Output(st_Buffer);
	}

	ZFORTIFY_UNLOCK();
	return(count);
}

// ZFortify_Disable() - This function provides a mechanism to disable ZFortify
// without recompiling all the sourcecode. It can only be called, though,
// when there is no memory on the ZFortify new'd list. (Ideally, at the 
// start of the program before any memory has been allocated). If you
// call this function when there IS memory on the ZFortify new'd list,
// it will issue an error, and ZFortify will not be disabled.
int ZFORTIFY_STORAGE
ZFortify_Disable(char *file, unsigned long line)
{
	int result;
	ZFORTIFY_LOCK();

	if(st_Head)
	{
		sprintf(st_Buffer, "ZFortify: %s.%d\n", file, line);
		st_Output(st_Buffer);
		st_Output("         ZFortify_Disable failed\n");
		st_Output("         (because there is memory on the ZFortify memory list)\n");
	
		ZFortify_OutputAllMemory(file, line);
		result = 0;            
	}
	else
	{
		st_Disabled = 1;
		result = 1;
	}
  
	ZFORTIFY_UNLOCK();
	return(result);
}

// Check a block's header and fortifications.
static int
CheckBlock(struct Header *h, char *file, unsigned long line)
{   
	unsigned char *ptr = (unsigned char *)h;
	int result = 1;

	if(!IsHeaderValid(h))
	{
		sprintf(st_Buffer, "\nZFortify: %s.%ld\n         Invalid pointer or corrupted header detected (%p)\n",
							file, line, ptr + sizeof(struct Header) + ZFORTIFY_BEFORE_SIZE);
		st_Output(st_Buffer);
		OutputLastVerifiedPoint();
		return(0);
	}

	if(!CheckFortification(ptr + sizeof(struct Header), ZFORTIFY_BEFORE_VALUE, ZFORTIFY_BEFORE_SIZE))
	{
		sprintf(st_Buffer, "\nZFortify: %s.%ld\n         Memory overrun detected before block\n",
							file, line);
		st_Output(st_Buffer);

		sprintf(st_Buffer,"         (%p,%ld,%s.%ld)\n", ptr + sizeof(struct Header) + ZFORTIFY_BEFORE_SIZE,
						  (unsigned long)h->Size, h->File, h->Line);
		st_Output(st_Buffer);
	
		OutputFortification(ptr + sizeof(struct Header), ZFORTIFY_BEFORE_VALUE, ZFORTIFY_BEFORE_SIZE);
		OutputLastVerifiedPoint();
		result = 0;
	}

	if(!CheckFortification(ptr + sizeof(struct Header) + ZFORTIFY_BEFORE_SIZE + h->Size,
							ZFORTIFY_AFTER_VALUE, ZFORTIFY_AFTER_SIZE))
	{
		sprintf(st_Buffer, "\nZFortify: %s.%ld\n         Memory overrun detected after block\n",
							file, line);
		st_Output(st_Buffer);

		sprintf(st_Buffer,"         (%p,%ld,%s.%ld)\n",
						ptr + sizeof(struct Header) + ZFORTIFY_BEFORE_SIZE,
						(unsigned long)h->Size, h->File, h->Line);
		st_Output(st_Buffer);

		OutputFortification(ptr + sizeof(struct Header) + ZFORTIFY_BEFORE_SIZE + h->Size,
							ZFORTIFY_AFTER_VALUE, ZFORTIFY_AFTER_SIZE);
		OutputLastVerifiedPoint();
		result = 0;
	}
 
	return(result);    
}

// Checks if the _size_ bytes from _ptr_ are all set to _value_
static int 
CheckFortification(unsigned char *ptr, unsigned char value, size_t size)
{
	while(size--)
		if(*ptr++ != value)
			return(0);

	return(1);      
}

// Set the _size_ bytes from _ptr_ to _value_.
static void 
SetFortification(unsigned char *ptr, unsigned char value, size_t size)
{
	memset(ptr, value, size);
}

// Output the corrupted section of the fortification
static void
OutputFortification(unsigned char *ptr, unsigned char /*value*/, size_t size)
{
	unsigned long offset, column;
	char	ascii[17];

	st_Output("Address     Offset Data");

	offset = 0;
	column = 0;

	while(offset < size)
	{
		if(column == 0)
		{
			sprintf(st_Buffer, "\n%8p %8d ", ptr, offset);
			st_Output(st_Buffer);
		}

		sprintf(st_Buffer, "%02x ", *ptr);
		st_Output(st_Buffer);

		ascii[ column ] = isprint( *ptr ) ? char(*ptr) : char(' ');
		ascii[ column + 1 ] = '\0';

		ptr++;
		offset++;
		column++;

		if(column == 16)
		{
			st_Output( "   \"" );
			st_Output( ascii );
			st_Output( "\"" );
			column = 0;
		}
	}

	if ( column != 0 )
	{
		while ( column ++ < 16 )
		{
			st_Output( "   " );
		}
		st_Output( "   \"" );
		st_Output( ascii );
		st_Output( "\"" );
	}

	st_Output("\n");
}

// Returns true if the supplied pointer does indeed point to a real Header
static int 
IsHeaderValid(struct Header *h)                                
{
	return(!ChecksumHeader(h));
}

// Updates the checksum to make the header valid
static void 
MakeHeaderValid(struct Header *h)
{
	h->Checksum = 0;
	h->Checksum = -ChecksumHeader(h);
}

// Calculate (and return) the checksum of the header. (Including the Checksum
// variable itself. If all is well, the checksum returned by this function should
// be 0.
static int 
ChecksumHeader(struct Header *h)
{
	int c, checksum, *p;
  
	for(c = 0, checksum = 0, p = (int *)h; c < sizeof(struct Header)/sizeof(int); c++)
		checksum += *p++;  
	
	return(checksum);
}                  

// Examines the new'd list to see if the given header is on it.
static int 
IsOnList(struct Header *h)
{
	struct Header *curr;
  
	curr = st_Head;
	while(curr)
	{
		if(curr == h)
			return(1);
	  
		curr = curr->Next;
	}

	return(0);
}

// Output the header...
static void
OutputHeader(struct Header *h)
{
	sprintf(st_Buffer, "%11p %8ld %s.%ld (%d)\n",
						(unsigned char*)h + sizeof(struct Header) + ZFORTIFY_BEFORE_SIZE,
						(unsigned long)h->Size,
						h->File, h->Line, h->Scope);
	st_Output(st_Buffer);
}

// Hex and ascii dump the memory
static void
OutputMemory(struct Header *h)
{
	OutputFortification((unsigned char*)h + sizeof(struct Header) + ZFORTIFY_BEFORE_SIZE,
						0, h->Size);
}

static void 
OutputLastVerifiedPoint()
{
	sprintf(st_Buffer, "\nLast Verified point: %s.%ld\n", 
					  st_LastVerifiedFile,
					  st_LastVerifiedLine);
	st_Output(st_Buffer);
}

#ifdef AUTOMATIC_LOG_FILE
// Automatic log file stuff!

// AutoLogFile class. There can only ever be ONE of these 
// instantiated! It is a static class, which means that
// it's constructor will be called at program initialization,
// and it's destructor will be called at program termination.
// We don't know if the other static class objects have been
// constructed/destructed yet, but this pretty much the best
// we can do with standard C++ language features.
class AutoLogFile
{
	static FILE *fp;
	static int   written_something;

public:
	AutoLogFile()
	{
		written_something = 0;
		ZFortify_SetOutputFunc(AutoLogFile::Output);
		ZFortify_EnterScope("Program Initialization",0);
	}

	static void Output(const char *s)
	{
		if(written_something == 0)
		{
			FIRST_ERROR_FUNCTION;
			fp = fopen(LOG_FILENAME, "w");
			if(fp)
			{
				wxDebugMsg("ZFortify log started at\n");
				time_t t;
				time(&t);
				fprintf(fp, "ZFortify log started at %s\n", ctime(&t));
				written_something = 1;
			}	
		}

		if(fp)
		{
			fputs(s, fp);
			fflush(fp);
		}	
	}
	
	~AutoLogFile()
	{
		ZFortify_LeaveScope("Program Termination",0);
		if(fp)
		{
			time_t t;
			time(&t);
			fprintf(fp, "\nZFortify log closed at %s\n", ctime(&t));
			fclose(fp);
			fp = 0;
		}	
	}
};

FILE *AutoLogFile::fp = 0;
int   AutoLogFile::written_something = 0;

static AutoLogFile Abracadabra;

#endif // AUTOMATIC_LOG_FILE
#endif // ZFORTIFY

