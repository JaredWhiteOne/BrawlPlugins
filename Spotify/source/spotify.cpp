#include <OS/OSError.h>
#include <OS/OSThread.h>
#include <VI/vi.h>
#include <memory.h>
#include <modules.h>
#include <printf.h>
#include <stdarg.h>
#include <string.h>
#include <sy_core.h>


#include "debug.h"
#include "spotify.h"
#include "https.h"
#include "jsmn.h"

#define AF_INET 0x2
#define SOCK_DGRAM 0x2
#define INADDR_ANY 0x0

#define MSG_WAITALL 0x100
#define STACK_SIZE 0x4000
char *KEYS[] = { "access_token", "token_type", "expires_in" };
char* json_token_tostr(char *js, jsmntok_t *t)
{
    js[t->end] = '\0';
    return js + t->start;
}
namespace Spotify {
    int Init()
    {
        char* result = new(Heaps::Network) char[0x2000];
        char* headerBuffer = new(Heaps::Network) char[0x1000];
        char *szUrl = "https://accounts.spotify.com/api/token";
        char* accessToken = new(Heaps::Network) char[0x80];
        long fileSize;
        char* headerIn[3];
        for(int i = 0; i < 3; i++)
        {
            headerIn[i] = new(Heaps::Network) char[128];
        }
        headerIn[0] = "grant_type=client_credentials";
        headerIn[1] = "client_id=";
        headerIn[2] = "client_secret=";
        result = HTTPSRequest::POST(599449625, szUrl, fileSize, &headerBuffer, headerIn, 3);
        jsmn_parser p;
        jsmntok_t t[7];
        jsmn_init(&p);
        int spotifyAuth = jsmn_parse(&p, result, strlen(result), t, 3);
        accessToken = json_token_tostr(result, &t[2]);

        return 0;
    }

} // namespace NetLog
