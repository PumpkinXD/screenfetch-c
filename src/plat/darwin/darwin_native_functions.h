#ifndef SCREENFETCH_C_DARWIN_NATIVE_FUCTIONS_H
#define SCREENFETCH_C_DARWIN_NATIVE_FUCTIONS_H

#include <stddef.h>
#include <stdint.h>
#include <uchar.h>





const uint32_t kCFStringEncodingUTF8 = 0x08000100;
const uint32_t kIOMasterPortDefault = NULL;
const uint32_t kIOReturnSuccess=0;
const uint32_t kNilOptions = 0;
















/*IDK which dylib contains these functions...*/


//returns CFArrayRef
typedef void*(*fnCFStringCreateArrayBySeparatingStrings)(
    //CFAllocatorRef
    void* alloc,
    //CFStringRef
    void* theString,
    //CFStringRef
    void* separatorString
    );

//returns CFPropertyListRef
typedef void*(*fnCFPreferencesCopyAppValue)(
    //CFStringRef
    void* key,
    //CFStringRef
    void* applicationID
);

//returns int, aarch64/amd64 mac ver
typedef int32_t(*fnCFStringGetIntValue)(
    //CFStringRef
    void* str
);

//returns const void*
typedef const void*(*fnCFArrayGetValueAtIndex)(
    //CFArrayRef
    void* theArray,
    //CFIndex
    int64_t idx
);


//returns boolean(obj-c ver.)
typedef uint8_t(*fnCFStringGetCString)(
    //CFStringRef
    void* theString,
    //char*
    char* buffer,
    //CFIndex
    int64_t bufferSize,
    //CFStringEncoding
    uint32_t encoding
);

//returns CFStringRef 
//CFSTR macro N/A in cosmo libc
typedef void*(*fnCFStringCreateWithCString)(
    //CFAllocatorRef
    void* alloc,
    //const char*
    const char *cStr,
    //CFStringEncoding
    uint32_t encoding
);

//use mac's glob/globfree?
//https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/glob.3.html


//@see https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/sysctlbyname.3.html
typedef int32_t(*fnsysctlbyname)(
        const char *name,
        void *oldp,
        size_t *oldlenp,
        void *newp,
        size_t newlen
);





//returns CFMutableDictionaryRef
typedef void*(*fnIOServiceMatching)(const char *name);

//returns kern_return_t
typedef int32_t(*fnIOServiceGetMatchingServices)(
    //mach_port_t 
    uint32_t mainPort,
     //CFDictionaryRef 
     void* matching,
    //io_iterator_t*
    uint32_t* existing
);

//returns io_object_t
typedef uint32_t(*fnIOIteratorNext)(
    //io_iterator_t 
    uint32_t iterator
);

//returns CFTypeRef
typedef void*(*fnIORegistryEntryCreateCFProperty)(
    //o_registry_entry_t 
    uint32_t entry,
    //CFStringRef
    void* key,
    // CFAllocatorRef
    void* allocator,
    //IOOptionBits 
    uint32_t options
);

//returns kern_return_t
typedef int32_t(*fnIOObjectRelease)(
    //io_object_t
    uint32_t object);





//returns const void*
typedef const void* (*fnCFDictionaryGetValue)(
    //CFDictionaryRef
    void* theDict,
    //const void*
    const void *key
);

//returns CFTypeID
typedef uint64_t(*fnCFGetTypeID)(
    //CFTypeRef
    void* cf
);

//returns CFTypeID
typedef uint64_t(*fnCFDataGetTypeID)(void);


typedef const uint8_t*(*fnCFDataGetBytePtr)(
    //CFDataRef
    void* theData
);

// void CFRelease(CFTypeRef cf);
typedef void(*fnCFRelease)(void* cf);







//TODO:https://github.com/woodruffw/screenfetch-c/blob/2d76746f07e502818051652878c334f9eb93ea6d/src/plat/darwin/detect.c#L318C3-L318C25

//CGError CGGetOnlineDisplayList(uint32_t maxDisplays, CGDirectDisplayID *onlineDisplays, uint32_t *displayCount);

//size_t CGDisplayPixelsWide(CGDirectDisplayID display);

//size_t CGDisplayPixelsHigh(CGDirectDisplayID display);




//CFIndex CFPreferencesGetAppIntegerValue(CFStringRef key, CFStringRef applicationID, Boolean *keyExistsAndHasValidFormat);









#endif