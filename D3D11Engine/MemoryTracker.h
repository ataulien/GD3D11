/* MemTracker.h - a program to track memory allocations in C or C++ */
/*    Copyright (C) 2010  Marshall Thomas               */
/*    Copyright (C) 2012 James McClain (I love the GPL)  */

/*    Version 0.1.6 9/21/2010 PRE-RELEASE DEV VERSION !!!!!   */
/*    - added support for MS __FUNCTION__ vs C99 __func__     */
/*    - unknown problem with MS error 4291 cropped up         */
/*    - added 'C' 64 bit address support ... or I think so... */
/*      will have to do more testing/inspection to see that was */
/*      done right and also works in the Microsoft case... */

/*    Version 0.1.7 9/23/2010 PRE-RELEASE DEV VERSION !!!!!   */
/*    - Clare: removed inline                                 */
/*    - Grant: bugs - rewording of reports                    */
/*                  - crash when FinalReport call too early   */
/*                    MT.h couldn't handle susequent deletes  */
/*                  - also pointer error in in MT_FreeAllMyMemory() */
/*    - Realloc() didn't have proto to call the MT function   */

/*    - Dara: use different hash function (sprintf is slow)   */
/*           -fix bugs in MT_Deallocator_is_wrong (stcmp)     */
/*           -make MT_free friendlier (don't free memory that */
/*            wasn't allocated in the first place             */
/*           -attempt to make realloc work                    */
/*           -add globals to allow delete to report all info  */
/* -use atexit to automatically report status  */
/*  right before exiting  */

/* new version underway.........changed #defines..... */
/* fix minor goofs in my malloc, calloc stuff */
/*
 * Version 0.2 11/26/2012 Open beta version.
 * - Improved output greatly. Making it eaiser for
 * students to read.
 * -Improved the way this program works with visual studio. 
 *
 */

/*Usage:*/
/*
 *   #include "MemTracker.h"      // put this at the start of your program
 *
 *   MemTrackerFinalReport();     // prints final report and frees the tool's memory
 * // called at exit via atexit()
 *
 *   MT_report_blocks_allocated();  // dumps current list of blocks allocated
 *                                  // If used, of course call this before
 *                                  //   MemTrackerFinalReport()!!!
 *                                  // this is an optional debug method
 */

/* GNU General Public License 3 */
/* This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* The prefix MT_ or simply _ are the decorators chosen for name space separation */
/* this software program is intended to be included as a .h file  */

/* LIMITATIONS:
 *    - this won't work with threads
 *    - will work with 64 bit address architecture - but total memory tracked
 *      must be <4GB.
 *    - some performance enhancements could be made, but don't appear to be
 *      necessary. However this is GPL3 licensed, so go for it if you want!
 *    - storage space of function names could be reduced, but see previous
 *      point!
 */

#ifndef MEMTRACKER_H
#define MEMTRACKER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_HASH_SZ 64  /*needs to be power of 2: 32,64,128,256...etc */

#define REDZONE_SIZE 4
#define REDZONE_STR "xxxx"

#ifdef _MSC_VER
/*Microsoft 4996 is a non-thread safe warning for various C library funcs*/
#pragma warning (disable : 4996)
/*Microsoft 4291 is not easy to understand about what happens if new fails*/
#pragma warning (disable : 4291)
/*Microsoft doesn't support __func__ or __FUNCTION in Visual Studio 6.x or older */
#if _MSC_VER <= 1200
#define __func__  "no name"
#else
/*Microsoft doesn't support C99 __func__ macro yet in Visual Studio 2008 */
#define __func__  __FUNCTION__
#endif
#endif

typedef struct _HASH_ENTRY
{
  unsigned long int address;   /* address of memory allocated           */
  unsigned int size_mem;       /* size of memory allocated (bytes)      */
  char     allocation_TLA[4];  /* MAL,CAL,RAL,NEW,VEC                   */
  unsigned int sequence_num;   /* sequential number of creation         */
  unsigned int source_line;    /* actual line in the code               */
  char*        function_name;  /* function name containing the alloc    */
  char*        file_name;      /* name of file containing the alloc     */
  unsigned int hash_allbits;   /* for re-masking and expansion later    */
  struct _HASH_ENTRY* next;    /* a simple stack                        */
  
} _HASH_ENTRY;

typedef struct _ERRORS { 
  char* allocation_TLA; // If this is empty the deallocater was called but
 // no allocater was.
  char* deallocation_TLA; 
  unsigned int source_line;
  char* function_name;
  char* file_name;
  struct _ERRORS* next;

} _ERRORS;
typedef struct _LEAKED_LIST { // Holds memory leaks so they can be printed nicely.
  unsigned int source_line;
  int mul; // How many times has this line been called and triggered a leak.
  unsigned int size_mem;
  char* function_name;
  char* file_name;
  struct _LEAKED_LIST* next;
  
} _LEAKED_LIST;
typedef struct _HASH_TABLE
{
  unsigned int total_rows;
  unsigned int rows_used;
  unsigned int total_entries;
  unsigned int total_bytes;            /* sum of all bytes allocated */
  unsigned int next_serial_num;
  unsigned int err_total_incompatible; /*MAL,CAL,RAL uses FRE; NEW uses DEL; VEC uses DVE */
  unsigned int err_total_unallocated;  /*delete of unallocated memory attempted */
  unsigned int current_bit_mask;       /*total_rows -1 (0xF for 16 slots) */
  struct _HASH_ENTRY **hash_array;
  struct _ERRORS *error_array; /* Record Errors such as using the wrong type
  of delete or deallocating stuff that has
  already been allocated, so the errors
  can be reported at the end.*/
  struct _LEAKED_LIST *leaked_list;
  
} _HASH_TABLE;

/*****************************/
/* prototypes                */
/*****************************/
void MT_init_hash();
void MT_free_hentry (_HASH_ENTRY* ptr);
unsigned int MT_gen_hash_bits (unsigned long int address);
unsigned int MT_gen_hash_array_index (unsigned long int address);
int MT_should_double();
_HASH_ENTRY* MT_find_entry(unsigned long int address);
int MT_is_deallocater_wrong(const char* alloc_TLA, const char* dealloc_TLA);
int MT_delete_entry (unsigned long int address,  const char* TLA, int line,
    const char* func, const char* file_name);
void MT_double_hash();
int MT_add_entry(unsigned long int address, unsigned int size, const char * TLA,
unsigned int source_line,  const char* func_name,
const char* file_name);
int MT_modify_entry (unsigned long int old_address, unsigned long int new_address,
    unsigned int size,             const char *TLA,
    unsigned int source_line,      const char* function_name,
    const char* file_name);
void MT_dump_stats();
void MT_dump_hash();
_HASH_ENTRY** MT_all_entries();
int MT_compare_seq(const void* vpa, const void* vpb);
void MT_dump_he(_HASH_ENTRY *p);
int MT_report_blocks_allocated(int speak);
void MT_FreeAllMyMemory();

void MemTrackerFinalReport();

void *MT_Malloc(size_t size, int line, const char* func_name, const char* file_name);
void *MT_Calloc(size_t numMembers, size_t size, int line, const char* func_name,
const char* file_name);
void *MT_Realloc(void *ptr, size_t size, int line, const char* func_name,
const char* file_name);
void MT_Free (void* ptr, int line, const char *func, const char *file_name);

void MT_AddError(const char* allocation_TLA,const char* deallocation_TLA,unsigned int source_line,const char* function_name,const char* file_name);
void MT_AddLeak(const char* file_name,const char * function_name,unsigned int source_line,unsigned int size_mem);

/*****************************/
/* main static hash table    */
/*****************************/
static _HASH_TABLE _hashtable;

/* a hack to work around limitations with overriding the C++ delete operator */
const char *_mt_src_file;
const char *_mt_src_func;
int _mt_src_line;

void MT_init_hash()
{
  _hashtable.total_rows              = INITIAL_HASH_SZ;
  _hashtable.rows_used               = 0;
  _hashtable.total_entries           = 0;
  _hashtable.next_serial_num         = 1;
  _hashtable.total_bytes             = 0;
  _hashtable.err_total_incompatible  = 0;
  _hashtable.err_total_unallocated   = 0;
  _hashtable.current_bit_mask        = INITIAL_HASH_SZ-1;
  _hashtable.hash_array = (_HASH_ENTRY**) calloc(INITIAL_HASH_SZ, sizeof (_HASH_ENTRY*));
  _hashtable.error_array = NULL;
  _hashtable.leaked_list = NULL;
  if (!_hashtable.hash_array)
    {
      fprintf (stderr, "MemTracker: Internal Error: main hash table init error - calloc()");
      exit (2);
    }
  
  atexit(MemTrackerFinalReport);
}

void MT_AddLeak(const char* file_name,const char * function_name,unsigned int source_line,unsigned int size_mem) {
  


  _LEAKED_LIST *ptr = NULL;

  ptr = (_LEAKED_LIST *) calloc(1,sizeof(_LEAKED_LIST));

  ptr->file_name = (char *) calloc(strlen(file_name)+1,sizeof(char));
  strcpy(ptr->file_name,file_name);

  ptr->function_name = (char *) calloc(strlen(function_name)+1,sizeof(char));
  strcpy(ptr->function_name,function_name);

  ptr->source_line = source_line;
  ptr->size_mem = size_mem;
  ptr->mul = 1;


  _LEAKED_LIST *walker = NULL;
  _LEAKED_LIST *last = NULL;
  
  if(! _hashtable.leaked_list) {// first call
    _hashtable.leaked_list = ptr;
    _hashtable.leaked_list->next = NULL;
    return;
  }
  for(walker = _hashtable.leaked_list;walker;walker = walker->next) {
    if(source_line == walker->source_line) {
      if(walker->mul == -1) {
walker->size_mem += size_mem;
      } else if(walker->size_mem == size_mem) {
walker->mul += 1;
      } else {
walker->size_mem += walker->size_mem * (walker->mul -1);
walker->mul = -1;
      }

      free(ptr); //b We didn't actually need it here.
// it's possible to check this stuff before hand and be more
// efficient, be my guest if that is what you like to do.

      return;
    }
    if(source_line < walker->source_line) {

      if(!last) { // The line is less then the first one.
ptr->next = walker;
_hashtable.leaked_list = ptr;
      } else {
last->next = ptr;
ptr->next = walker;
      }
      return;
    }

    last = walker;

  }
  // If it's the biggest line so far.
  last->next = ptr;
  ptr->next = NULL;
}
void MT_AddError(const char* allocation_TLA,const char* deallocation_TLA,unsigned int source_line,const char* function_name,const char* file_name) {

	if(!function_name)
		function_name = "unknown_function";

	if(!file_name)
		file_name = "unknown_file";

  _ERRORS *ptr = NULL;
  ptr = (_ERRORS *) calloc(1,sizeof(_ERRORS));
  if(! (allocation_TLA)) {
    // Then this is not a mismatch, it is a double deallocation call.
    ptr->allocation_TLA = NULL;
    ptr->deallocation_TLA = 
      (char *) calloc(strlen(deallocation_TLA)+1,sizeof(char));
    strcpy(ptr->deallocation_TLA,deallocation_TLA);

  } else {
    ptr->allocation_TLA = (char *) calloc(strlen(allocation_TLA)+1,sizeof(char));
    strcpy(ptr->allocation_TLA,allocation_TLA);
    ptr->deallocation_TLA = 
      (char *) calloc(strlen(deallocation_TLA)+1,sizeof(char));
    strcpy(ptr->deallocation_TLA,deallocation_TLA);

  }
  ptr->source_line = source_line;
  ptr->function_name = (char *) calloc(strlen(function_name)+1,sizeof(char));
  strcpy(ptr->function_name,function_name);
  ptr->file_name = (char *) calloc(strlen(file_name)+1,sizeof(char));
  strcpy(ptr->file_name,file_name);
  
  _ERRORS *walker = NULL;
  _ERRORS *last = NULL;
  
  if(! _hashtable.error_array) {// first call
    _hashtable.error_array = ptr;
    _hashtable.error_array->next = NULL;
    return;
  }
  for(walker = _hashtable.error_array;walker;walker = walker->next) {
    if(source_line < walker->source_line) {
      if(!last) { // The line is less then the first one.
ptr->next = walker;
_hashtable.error_array = ptr;
      } else {
last->next = ptr;
ptr->next = walker;
      }
      return;
    }
    last = walker;
  }
  // If it's the biggest line so far.
  last->next = ptr;
  ptr->next = NULL;
}


void MT_free_hentry (_HASH_ENTRY* ptr)
{
  
  if (ptr == NULL) {printf ("DIE\n"); exit(5);}
  if (ptr->function_name)
    free (ptr->function_name);
  if (ptr->file_name)
    free (ptr->file_name);

  free(ptr);
  
}

/* MT_gen_hash_bits2()
 *   Thomas Wang's 64-bit hash function - works well for integers, and is significantly
 *   faster than the DJB function since the key is not ASCII.  It is also slightly
 *   better in distributing keys.
 *   http://www.concentric.net/~Ttwang/tech/inthash.htm
 */
unsigned int MT_gen_hash_bits (unsigned long int address)
{
  address = (~address) + (address << 21); // address = (address << 21) - address - 1;
  address = address ^ (address >> 24);
  address = (address + (address << 3)) + (address << 8); // address * 265
  address = address ^ (address >> 14);
  address = (address + (address << 2)) + (address << 4); // address * 21
  address = address ^ (address >> 28);
  address = address + (address << 31);
  return (unsigned int)address;
}


unsigned int MT_gen_hash_array_index (unsigned long int address)
{
  unsigned int all_bits = MT_gen_hash_bits(address);
  return( all_bits & _hashtable.current_bit_mask );
}

int MT_should_double() /* hash table automagically doubles when needed */
{
  if(_hashtable.total_entries > _hashtable.total_rows-1) return 1;
  return 0;
}

_HASH_ENTRY* MT_find_entry(unsigned long int address)
{
  unsigned int i     = MT_gen_hash_array_index(address);
  _HASH_ENTRY* mover = _hashtable.hash_array[i];
  
  while (mover)
    {
      if ( mover->address == address)
return mover;     /* found!! */
      mover = mover -> next;
    }
  return NULL;  /* not found */
}

/* MemTracker keeps a record of what kind of allocation method was
 *    used to allocate the memory in the first place.
 * The user's program must use a compatible method to deallocate the
 *   memory otherwise an error will be reported.
 *
 * The C allocation methods of malloc(), calloc(), realloc() must use free()
 *   to de-allocate that memory.
 *
 * The C++ allocation method scalar "new" must use scalar "delete".
 * The C++ allocation method of the vector form of "new" must use the
 *   vector form of "delete".
 * This is an important error as although calling free() on a C++ array
 *   of objects will indeed free memory for those objects, the destructors
 *   for those objects will not be called! Oops!
 *
 * I use TLA's (Three Letter Acronymns) to track this stuff.
 */

int MT_is_deallocater_wrong(const char* alloc_TLA, const char* dealloc_TLA)
{
  int ok =0;
  int bad=1;
  
  if (    ( strcmp(dealloc_TLA,"FRE") == 0 || strcmp(dealloc_TLA,"RAL") == 0)
 && ( strcmp(alloc_TLA,"MAL") == 0   || strcmp(alloc_TLA,"CAL") == 0
      || strcmp(alloc_TLA,"RAL") == 0)
 ) { return ok; }
  
  if (    strcmp(dealloc_TLA,"DEL") == 0 /* C++ scalar delete */
 &&   strcmp(alloc_TLA,"NEW") == 0
 ) return ok;
  
  if (    strcmp(dealloc_TLA,"VDE") == 0  /* C++ vector delete */
 &&   strcmp(alloc_TLA,"VEC") == 0
 ) return ok;
  
  _hashtable.err_total_incompatible++;
  return bad;
}

void MT_init_redzone(  void *address, unsigned long int size )
{
  char *start = (char *)address + size - REDZONE_SIZE;
  
  strncpy(start, REDZONE_STR, REDZONE_SIZE);
}

int MT_check_redzone(  void *address, unsigned long int size )
{
  char *start = (char *)address + size - REDZONE_SIZE;
  
  if(strncmp(start, REDZONE_STR, REDZONE_SIZE) != 0) return 0;
  return 1;
}

int MT_delete_entry (  unsigned long int address,  const char* TLA, int line,
      const char* func, const char* file_name )
{
  _HASH_ENTRY*  mover = NULL;
  _HASH_ENTRY*  prev  = NULL;
  unsigned int i;
  
  if (_hashtable.hash_array == NULL )
    return (1); /* Probable cause: MemTrackerFinalReport() called too early */
  /* C++ is continuing to run destructor methods */
  /* I consider that "ok" */
  
  i = MT_gen_hash_array_index(address);
  
  if (!_hashtable.hash_array[i])
    {
		 return 0;
      /* fprintf (stderr, "MemTracker: %s:%u func:%s()  Deallocator called but 0x%lX not allocated!\n", */
      /*       file_name, line, func, address); */
      MT_AddError(NULL,TLA,line,func,file_name);
      _hashtable.err_total_unallocated++;
      return 0; /* fail */
    }
  
  mover  =  _hashtable.hash_array[i];
  
  /* zap the entry on the stack if we can */
  while (mover)
    {
      if (mover->address == address)
{
 if ( !MT_check_redzone((void *)address, mover->size_mem))
   {
     fprintf (stderr, "MemTracker: %s:%u func:%s()  Heap corruption before deallocator called!\n",
      file_name, line, func);
   }
 if ( MT_is_deallocater_wrong(mover->allocation_TLA, TLA) )
   {
     /* fprintf (stderr, "MemTracker: %s:%u func:%s()  Deallocator %s not compatible with %s!\n", */
     /*       file_name, line, func, mover->allocation_TLA, TLA); */
           MT_AddError(mover->allocation_TLA,TLA,line,func,file_name);
     
     _hashtable.err_total_incompatible++;
     /* keep going..the error has been reported..user code may still run! */
     /* we let the user program do what it can */
   }
 
 _hashtable.total_bytes -= mover->size_mem;
 
 if (!prev)
   _hashtable.hash_array[i] = mover->next;
 else prev->next = mover->next;
 
 if ( !_hashtable.hash_array[i] )
   _hashtable.rows_used--;
 
 _hashtable.total_entries--;
 MT_free_hentry (mover);
 
 return 1; /* success */
}
      
      prev  = mover;
      mover = mover->next;
    }
  
  fprintf (stderr, "MemTracker: %s:%u func:%s()  Deallocator called but 0x%lX not allocated!\n",
  file_name, line, func, address);
  
  _hashtable.err_total_unallocated++;
  return 0;       /* fail */
}

void MT_double_hash()
{
  _HASH_ENTRY **old_hash = _hashtable.hash_array;
  _HASH_ENTRY *mover = NULL;
  int n_rows = _hashtable.total_rows;
  int i;
  
  _hashtable.total_rows = n_rows * 2;
  _hashtable.current_bit_mask = _hashtable.total_rows -1;
  _hashtable.rows_used =0;
  
  _hashtable.hash_array =(_HASH_ENTRY**) calloc(_hashtable.total_rows, sizeof (_HASH_ENTRY*));
  if (!_hashtable.hash_array)
    {
      fprintf (stderr, "MemTracker: Internal error:  calloc failed for main hash table\n");
      exit(2);
    }
  
  for ( i=0; i<n_rows; i++)
    {
      mover = old_hash[i];
      while (mover)
{
 _HASH_ENTRY  *mover_next = mover -> next;
 unsigned int new_row = (   mover->hash_allbits
    & _hashtable.current_bit_mask);
 
 /*add to front of new stack */
 
 _HASH_ENTRY* temp = _hashtable.hash_array[new_row];
 if (!temp)
   {
     _hashtable.hash_array[new_row] = mover;
     mover -> next = NULL;
     _hashtable.rows_used++;
   }
 else
   {
     mover->next = temp;
     _hashtable.hash_array[new_row] = mover;
   }
 
 mover = mover_next;
}
    }
  free (old_hash);
}

int MT_add_entry(unsigned long int address, unsigned int size, const char * TLA,
unsigned int source_line,  const char* func_name, const char* file_name)
{
  unsigned int all_bits, i;
  _HASH_ENTRY* p_new_entry, *temp;
  
  if (!_hashtable.hash_array) MT_init_hash (); /* automagic init() */
  
  if (MT_should_double()) MT_double_hash();
  
  if ( MT_find_entry(address)  ) /* O/S or internal error */
    {
      fprintf (stderr, "MemTracker: Table Error: Address %lu already in use! %s:%u func:%s()\n",
      address, file_name, source_line, func_name);
      exit(2);
    }
  
  all_bits = MT_gen_hash_bits (address);
  i = all_bits & _hashtable.current_bit_mask;
  p_new_entry = (_HASH_ENTRY*) calloc(1, sizeof (_HASH_ENTRY) );
  if (!p_new_entry)
    {
      fprintf (stderr, "MemTracker: Internal error: calloc failed for new hash entry\n");
      exit (2);
    }
  p_new_entry->address      = address;
  p_new_entry->hash_allbits = all_bits;
  p_new_entry->sequence_num = _hashtable.next_serial_num++;
  p_new_entry->size_mem     = size;
  p_new_entry->source_line  = source_line;
  p_new_entry->next         = NULL;
  strncpy (p_new_entry->allocation_TLA, TLA,3);
  
  /* insert new entry on top of stack*/
  
  temp = _hashtable.hash_array[i];
  if (!temp)
    {
      _hashtable.rows_used++;
      _hashtable.hash_array[i] = p_new_entry;
    }
  else
    {
      p_new_entry->next = temp;
      _hashtable.hash_array[i] = p_new_entry;
    }
  
  p_new_entry->function_name =(char*) malloc(strlen(func_name)+1);
  if (!p_new_entry->function_name)
    {
      fprintf (stderr, "MemTracker: Internal error: malloc failed for function name\n");
      exit (2);
    }
  
  strcpy(p_new_entry->function_name, func_name);
  
  p_new_entry->file_name =(char*) malloc(strlen(file_name)+1);
  if (!p_new_entry->file_name)
    {
      fprintf (stderr, "MemTracker: Internal error: malloc failed for file name\n");
      exit (2);
    }
  
  strcpy(p_new_entry->file_name, file_name); 
  
  _hashtable.total_entries++;
  _hashtable.total_bytes += size;
  return 1; /* success */
}

int MT_modify_entry (unsigned long int old_address, unsigned long int new_address,
    unsigned int size,             const char* TLA,
    unsigned int source_line,      const char* function_name,
    const char* file_name)
{
  /* this is the realloc case */
  
  _HASH_ENTRY* ptr;
  
  if (! (ptr = MT_find_entry(old_address) ) )
    {
      fprintf (stderr, "MemTracker: %s:%u func:%s()  realloc called but 0x%lX not allocated!\n",
      file_name, source_line, function_name, old_address);
      return 1;  /* fail */
    }
  
  MT_delete_entry(old_address, "RAL", source_line, function_name, file_name);
  MT_add_entry(new_address, size, TLA , source_line, function_name, file_name);
  
  return 0;  /* success */
}


void MT_dump_stats()
{
  printf ("\nDump stats called\n");
  printf ("total_rows        %u\n",  _hashtable.total_rows);
  printf ("rows_used:        %u\n",  _hashtable.rows_used);
  printf ("total_entries:    %u\n",  _hashtable.total_entries);
  printf ("last_serial_num:  %u\n",  _hashtable.next_serial_num-1);
  printf ("total bytes alloc %u\n",  _hashtable.total_bytes);
  printf ("bit mask:         %X\n",  _hashtable.current_bit_mask);
}

void MT_dump_hash()
{
  unsigned int i;
  for ( i =0; i< _hashtable.total_rows; i++)
    {
      _HASH_ENTRY* mover = _hashtable.hash_array[i];
      printf ("\nrow[%u]",i);
      while (mover)
{
 printf ("n=%u:a=0x%lX:size=%u  ", mover->sequence_num,
 mover->address,
 mover->size_mem);
 mover = mover->next;
}
    }
}

_HASH_ENTRY** MT_all_entries()  /* list of all current hash table entries */
{
  _HASH_ENTRY  *mover;
  _HASH_ENTRY  **lmover;
  _HASH_ENTRY ** list;
  unsigned int i;
  
  if (!_hashtable.total_entries) return NULL;
  
  list = (_HASH_ENTRY**) calloc(_hashtable.total_entries +1, sizeof(_HASH_ENTRY*));
  if (!list)
    {
      fprintf (stderr, "MemTracker: Internal error: malloc failed in MT_all_entries()\n");
      exit(2);
    }
  
  lmover = list;
  
  for (i=0; i<_hashtable.total_rows; i++)
    {
      if (_hashtable.hash_array != NULL)
{
 mover = _hashtable.hash_array[i];
 while (mover)
   {
     *lmover++ = mover;
     mover = mover -> next;
   }
}
    }
  
  return list;
}

int MT_compare_seq(const void* vpa, const void* vpb)
{
  _HASH_ENTRY** a = (_HASH_ENTRY**)vpa;
  _HASH_ENTRY** b = (_HASH_ENTRY**)vpb;
  return ( ((*a)->sequence_num) -  ((*b)->sequence_num) );
}

void MT_dump_he(_HASH_ENTRY *p)  /* dump a hash entry */
{
  /* printf ("n=%u:a=0x%lX:size=%u:type=%s:line=%u:func=%s:file=%s\n", p->sequence_num, */
  /*  p->address, */
  /*  p->size_mem - REDZONE_SIZE, */
  /*  p->allocation_TLA, */
  /*  p->source_line, */
  /*  p->function_name, */
  /*  p->file_name); */
  
  //  printf("%s:%-3d - %u bytes lost",p->file_name,p->source_line,p->size_mem - REDZONE_SIZE);
  MT_AddLeak(p->file_name,p->function_name,p->source_line,p->size_mem - REDZONE_SIZE); 
}

int MT_report_blocks_allocated(int speak)  /* dumps sorted list by serial_num of all hash entries */
{
  _HASH_ENTRY **list  = MT_all_entries();
  _HASH_ENTRY **mover = list;
  
  qsort (list, _hashtable.total_entries, sizeof(_HASH_ENTRY**), MT_compare_seq);
  
  //  printf ("\n");
  int leakNumber = 0;
  if (mover)
    {
      while (*mover)
{
 if(speak) {
   MT_dump_he(*mover);
 }
 leakNumber++;
 
 mover++;
}
    }
  
  free(list);
  return leakNumber;
}

void MT_FreeAllMyMemory()          /* avoid embarassing leak ourselves! */
{
  _HASH_ENTRY** list   = MT_all_entries();

  _HASH_ENTRY** lmover = list;
  
  
  if (list)
    {
      while (*lmover)
{
 MT_free_hentry(*lmover);
 lmover++;
}
      free (list);
    }


  free (_hashtable.hash_array);
  _hashtable.hash_array = NULL;

  // Time to free ERRORS and LEAKED_LIST

  _ERRORS *ePtr;
  _ERRORS *toFree;
  ePtr = _hashtable.error_array;
  while(ePtr) {
    toFree = ePtr;
    ePtr = ePtr->next;
    
    if(toFree->allocation_TLA) {
      free(toFree->allocation_TLA);
    }
    if(toFree->deallocation_TLA) {
      free(toFree->deallocation_TLA);
    }
    if(toFree->function_name) {
      free(toFree->function_name);
    }
    if(toFree->file_name) {
      free(toFree->file_name);
    }
    free(toFree);
  }
    _LEAKED_LIST *lPtr;
    _LEAKED_LIST *ltoFree;
    lPtr = _hashtable.leaked_list;

    while(lPtr) {

      ltoFree = lPtr;
      
      lPtr = lPtr->next;

      if(ltoFree->file_name){
free(ltoFree->file_name);
      }
      if(ltoFree->function_name){
free(ltoFree->function_name);
      }
      free(ltoFree);
    }
}

void MemTrackerFinalReport()
{

	AllocConsole();
	freopen("CONIN$", "r",stdin);
	freopen("CONOUT$", "w",stdout);
	freopen("CONOUT$", "w",stderr);


  const char * getTLA(char *TLA);
  int size;
  size = MT_report_blocks_allocated(1);
  if(_hashtable.total_bytes != 0) {
    printf("\nYour program contains memory leaks!\n");
    printf("Total bytes lost: %d\n",_hashtable.total_bytes - REDZONE_SIZE * (size));
    printf("###########      START LIST OF LEAKS      ###########\n");
    _LEAKED_LIST *ptr2 = NULL;
    char *lastFuncName2 = (char *)"fewfewgewgewgw";
    int lastLineNum2 = -1;
    for(ptr2 = _hashtable.leaked_list;ptr2;ptr2 = ptr2->next) {
if(strcmp(lastFuncName2,ptr2->function_name) != 0) {
 printf("%s()\n",ptr2->function_name);
 lastFuncName2 = ptr2->function_name;
}
if((int)ptr2->source_line != lastLineNum2) {

 printf("  %s:%-3d - ",ptr2->file_name,ptr2->source_line);
 if(ptr2->mul == 1 || ptr2->mul == -1) {
   printf("%u bytes lost.\n",ptr2->size_mem);
 } else {
   printf("%u bytes lost %u times.\n",ptr2->size_mem,ptr2->mul);
 }

 lastLineNum2 = ptr2->source_line;
}

    }

    printf("###########       END LIST OF LEAKS       ###########\n");
  }
  

  if(_hashtable.total_bytes == 0) {
    printf("\nCongratulations, your program has no memory leaks.\n");
    if(_hashtable.error_array != NULL) {
      printf("HOWEVER, errors have been detected!\n");
    }
  }
  if(_hashtable.error_array != NULL) {

    printf("########### START LIST OF NON-LEAK ERRORS ###########\n");
      _ERRORS *ptr = NULL;
      
      char * lastFuncName = (char *)"fewagewgreqwteqw"; // setinal value.
                                                       // please don't name your function this
                                                       // a random hiku

      int lastLineNum = -1;
      for(ptr = _hashtable.error_array;ptr;ptr = ptr->next) {
if(strcmp(lastFuncName,ptr->function_name) != 0) {
 printf("%s()\n",ptr->function_name);
 lastFuncName = ptr->function_name;
}
if((int)ptr->source_line != lastLineNum) {
 printf("  %s:%-3d - ",ptr->file_name,ptr->source_line);
 if(ptr->allocation_TLA) { // it's a mismatch error 
   printf("Using \"%s\" to free an object allocated with \"%s\".\n",
  getTLA(ptr->deallocation_TLA),getTLA(ptr->allocation_TLA));
 } else { // deallocation was called on a nonallocated object.
   printf("\"%s\" was called but nothing was allocated at that time.\n",
  getTLA(ptr->deallocation_TLA));
 }
 lastLineNum = ptr->source_line;
}
      }
      printf("###########  END LIST OF NON-LEAK ERRORS  ###########\n");
    } 


  MT_FreeAllMyMemory();

  printf("\n - Memory report done, press any key to exit. - \n");
  getc(stdin);
  return;
}
const char * getTLA(char *TLA) {
  if(strcmp(TLA,"DEL") == 0) {
    return "delete";
  }else if (strcmp(TLA,"VDE") == 0) {
    return "delete[]";
  }else if (strcmp(TLA,"NEW") == 0) {
    return "new";
  }else if (strcmp(TLA,"VEC") == 0) {
    return "new[]";
  }else if (strcmp(TLA,"MAL") == 0) {
    return "malloc";
  }else if (strcmp(TLA,"CAL") == 0) {
    return "calloc";
  }else if (strcmp(TLA,"RAL") == 0) {
    return "realloc";
  }else if (strcmp(TLA,"FRE") == 0) {
    return "free";
  }
  return "WTF";
}
void *MT_Malloc(size_t size, int line, const char* func_name, const char* file_name)
{
  void *ptr;
  size += REDZONE_SIZE;
  
  if ((ptr = (void *) malloc(size)) == NULL)
    {
      perror("malloc failed to get memory!");
      exit(1);
    }
  MT_add_entry((unsigned long int)ptr, size, "MAL", line, func_name, file_name);
  MT_init_redzone(ptr, size);
  return ptr;
}

void *MT_Calloc(size_t numMembers, size_t size, int line, const char* func_name,
const char *file_name)
{
  void *ptr;
  size = numMembers * size + REDZONE_SIZE;
  
  if ((ptr = (void *) malloc(size)) == NULL)
    {
      perror("calloc failed to get memory!");
      exit(1);
    }
  
  memset(ptr, 0, size);
  MT_add_entry((unsigned long int)ptr, size, "CAL", line, func_name,
      file_name);
  MT_init_redzone(ptr, size);
  return ptr;
}

void *MT_Realloc(void *ptr, size_t size, int line, const char* func_name,
const char* file_name)
{
  void *newptr;
  size += REDZONE_SIZE;
  
  if(!ptr)
    {
      if ((newptr = (void *) realloc(ptr, size)) == NULL)
{
 perror("realloc failed to resize memory!");
 exit(1);
}
      
      MT_add_entry((unsigned long int)newptr, size, "RAL", line, func_name, file_name);
    }
  else if(!MT_find_entry((unsigned long int)ptr))
    {
      fprintf (stderr, "MemTracker: %s:%u func:%s()  realloc called but 0x%lX not allocated!\n",
      file_name, line, func_name, (unsigned long int)ptr);
      return ptr;
    }
  
  else
    {
      if ((newptr = (void *) realloc(ptr, size)) == NULL)
{
 perror("realloc failed to resize memory!");
 exit(1);
}
      
      MT_modify_entry((unsigned long int) ptr, (unsigned long int)newptr, size, "RAL",
     line, func_name, file_name);
    }
  
  MT_init_redzone(newptr, size);
  return newptr;
}

void MT_Free (void* ptr, int line, const char* func, const char* file_name)
{
  if(MT_delete_entry((unsigned long int)ptr, "FRE", line, func, file_name))
    free(ptr);
  
  return;
}

/********************  C++  overrides **********************/
#ifdef __cplusplus

/*this appears to be a very slightly documented interface and */
/*the ramifications of this are unknown*/

void operator delete(void* pMem, char* pszFilename, int nLine)
{
  fprintf (stderr, "WOW, WOWIE ***********\n");
  fprintf (stderr, "This function shouldn't be called! \n");
  fprintf (stderr, "address=0x%lX name=%s, line=%d\n", (unsigned long int)pMem, pszFilename, nLine);
  free(pMem); /* http://msdn.microsoft.com/en-us/library/cxdxz3x6%28VS.80%29.aspx */
}

void* operator new(size_t size, unsigned int line,  const char* func_name,
  const char* file_name)
{
  void *ptr;
  size += REDZONE_SIZE;
  
  if ((ptr = (void *) malloc(size)) == NULL)
    {
      perror("C++ function \"new\" failed to get memory!");
      exit(1);
    }
  MT_add_entry((unsigned long int)ptr, size, "NEW", line, func_name, file_name);
  MT_init_redzone(ptr, size);
  return ptr;
}

void* operator new [] (size_t size, unsigned int line, const char* func_name,
      const char* file_name)
{
  void *ptr;
  size += REDZONE_SIZE;
  
  if ((ptr = (void *) malloc(size)) == NULL)
    {
      perror("C++ Vector form of \"new\" failed to get memory!");
      exit(1);
    }
  
  MT_add_entry((unsigned long int)ptr, size, "VEC", line, func_name, file_name);
  MT_init_redzone(ptr, size);
  return ptr;
}

/* C++ scalar delete */
/* The default delete "throws" (), I should claim the same     */
/* even if I don't throw anything! Some compilers require this */
void operator delete(void* ptr) throw ()
{
	if(!ptr)
		return;

  if(MT_delete_entry((unsigned long int)ptr ,"DEL", _mt_src_line, _mt_src_func, _mt_src_file))
    free(ptr);
}

/* C++ vector delete */
/* The default delete[] "throws" (), I should claim the same    */
/* even if I don't throw anything!  Some compilers require this */
void operator delete [] (void* ptr) throw ()
{
	if(!ptr)
		return;

  if(MT_delete_entry((unsigned long int)ptr, "VDE", _mt_src_line, _mt_src_func, _mt_src_file))
    free(ptr);
}

#define new new(__LINE__, __func__, __FILE__)
#define delete _mt_src_file=__FILE__, _mt_src_func=__func__, _mt_src_line=__LINE__, delete

#endif
/************* end of C++ specific  ************/

#define malloc(size)       MT_Malloc (size, __LINE__, __func__, __FILE__)
#define calloc(n,size)     MT_Calloc (n, size, __LINE__, __func__, __FILE__)
#define free(adr)          MT_Free   (adr, __LINE__, __func__, __FILE__)
#define realloc(ptr, size) MT_Realloc(ptr, size, __LINE__, __func__, __FILE__);
#endif