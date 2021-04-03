
static char *easyString_copyToHeap(char *at) {
    u32 length = easyString_getSizeInBytes_utf8(at);
    //NOTE(ollie): Get memory from heap
    char *result = (char *)easyPlatform_allocateMemory(sizeof(char)*(length + 1), EASY_PLATFORM_MEMORY_NONE);
    //NOTE(ollie): Copy the string
    easyPlatform_copyMemory(result, at, sizeof(char)*length);
    //NOTE(ollie): Null terminate the string
    result[length] = '\0'; //Null terminate

    return result;
}

static char *easyString_copyToBuffer(char *at, char *buffer, u32 bufferLen) {
    
    assert(easyString_getSizeInBytes_utf8(at) < bufferLen); //NOTE(ollie): Accounting for the null terminator 
    //NOTE(ollie): Copy the string
    easyPlatform_copyMemory(buffer, at, sizeof(char)*bufferLen);
    //NOTE(ollie): Null terminate the string
    buffer[bufferLen - 1] = '\0'; //Null terminate

    return buffer;
}

#define easyString_copyToArena(a, arena) easyString_copyToArena_(a, arena, easyString_getSizeInBytes_utf8(a))
char *easyString_copyToArena_(char *a, Arena *arena, int newStrLen_) {
    int newStrLen = newStrLen_ + 1; //for null terminator
    
    char *newString = (char *)pushArray(arena, newStrLen, char);

    assert(newString);
    
    newString[newStrLen - 1] = '\0';
    
    char *at = newString;
    for (int i = 0; i < (newStrLen - 1); ++i)
    {
        *at++ = a[i];
    }
    assert(newString[newStrLen - 1 ] == '\0');

    return newString;
}