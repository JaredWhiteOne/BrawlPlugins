#include "stringutils.h"
#include <mem.h>
namespace StringUtils {
    void removeChars(char* str, int indexBegin, int indexEnd) 
    {
        for (int i = 1; i <= (indexEnd - indexBegin) + 1; ++i) 
        {
            memmove(&str[indexBegin], &str[indexBegin + 1], strlen(str));
        }
    }

    int findLastIndex(char* str, char x)
    {
        int index = -1;
        for (int i = 0; i < strlen(str); i++)
        {
            if (str[i] == x)
            {
                index = i;
            }
        }
        return index;
    }
}