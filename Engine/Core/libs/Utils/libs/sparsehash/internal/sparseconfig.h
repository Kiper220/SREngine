/*
 * NOTE: This file is for internal use only.
 *       Do not use these #defines in your own program!
 */

/* Namespace for Google classes */
#define GOOGLE_NAMESPACE  ::google

#if ((_MSC_VER >= 1800))

/* the location of the header defining hash functions */
#define HASH_FUN_H  <unordered_map>

#elif (defined(__MINGW32__) || defined(__MINGW64__))

#define HASH_FUN_H  <unordered_set>

#elif defined(ANDROID)
    #define HASH_FUN_H  <unordered_set>
#else /* Earlier than VSC++ 2013 */ 

/* the location of the header defining hash functions */
#define HASH_FUN_H  <hash_map>

#endif

#if (defined(__MINGW32__) || defined(__MINGW64__) || defined(ANDROID))
    #define HASH_NAMESPACE std
#else
    /* the namespace of the hash<> function */
    #define HASH_NAMESPACE  stdext
#endif

/* Define to 1 if you have the <inttypes.h> header file. */
#undef HAVE_INTTYPES_H

/* Define to 1 if the system has the type `long long'. */
#define HAVE_LONG_LONG  1

/* Define to 1 if you have the `memcpy' function. */
#define HAVE_MEMCPY  1

/* Define to 1 if you have the <stdint.h> header file. */
#undef HAVE_STDINT_H

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H  1

/* Define to 1 if the system has the type `uint16_t'. */
#undef HAVE_UINT16_T

/* Define to 1 if the system has the type `u_int16_t'. */
#undef HAVE_U_INT16_T

/* Define to 1 if the system has the type `__uint16'. */
#define HAVE___UINT16  1

#if (defined(__MINGW32__) || defined(__MINGW64__) || defined(ANDROID))
    #define SPARSEHASH_HASH HASH_NAMESPACE::hash
#else
    /* The system-provided hash function including the namespace. */
    #define SPARSEHASH_HASH  HASH_NAMESPACE::hash_compare
#endif

/* The system-provided hash function, in namespace HASH_NAMESPACE. */
#define SPARSEHASH_HASH_NO_NAMESPACE  hash_compare

/* Stops putting the code inside the Google namespace */
#define _END_GOOGLE_NAMESPACE_  }

/* Puts following code inside the Google namespace */
#define _START_GOOGLE_NAMESPACE_   namespace google {
