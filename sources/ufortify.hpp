// ufortify.hpp  -  User options for ZFortify.  This is where you configure
// ZFortify's  behaiviour  to  suit  your debugging needs.  Changes to this
// file require ZFortify.cpp to be recompiled, but nothing else.

//#define ZFORTIFY_PROVIDE_ARRAY_NEW

#define ZFORTIFY_STORAGE

#define ZFORTIFY_BEFORE_SIZE      32  // Bytes to allocate before block
#define ZFORTIFY_BEFORE_VALUE   0xA3  // Fill value before block

#define ZFORTIFY_AFTER_SIZE       32  // Bytes to allocate after block
#define ZFORTIFY_AFTER_VALUE    0xA5  // Fill value after block

#define FILL_ON_NEW                   // Nuke out malloc'd memory
#define FILL_ON_NEW_VALUE       0xA7  // Value to initialize with

#define FILL_ON_DELETE                 // free'd memory is cleared
#define FILL_ON_DELETE_VALUE     0xA9  // Value to de-initialize with

// #define CHECK_ALL_MEMORY_ON_NEW
// #define CHECK_ALL_MEMORY_ON_DELETE

// #define PARANOID_DELETE

#define WARN_ON_NEW_FAIL       // A debug is issued on a failed new
#define WARN_ON_ZERO_NEW       // A debug is issued on a new of a zero byte object
#define WARN_ON_FALSE_FAIL     // See Fortify_SetMallocFailRate()
#define WARN_ON_UNSIGNED_LONG_OVERFLOW // Watch for breaking the 32 bit limit
																			 // because of the size of the fortifications
																			 // (unlikely)
/*
#define AUTOMATIC_LOG_FILE
#define LOG_FILENAME            "fortify.log"
#define FIRST_ERROR_FUNCTION		wxDebugMsg("First Error\n")
*/
#define ZFORTIFY_LOCK()
#define ZFORTIFY_UNLOCK()
