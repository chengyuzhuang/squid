#include "squid.h"

#if STORE_KEY_URL

static char key_temp_buffer[MAX_URL + 100];

const char *
storeKeyText(const char *key)
{
    return key;
}

const char *
storeKeyScan(const char *buf)
{
    return buf;
}

const cache_key *
storeKeyPrivate(const char *url, method_t method, int num)
{
    assert(num > 0);
    debug(20, 3) ("storeKeyPrivate: '%s'\n", url);
    key_temp_buffer[0] = '\0';
    snprintf(key_temp_buffer, MAX_URL + 100, "%d/%s/%s",
	num,
	RequestMethodStr[method],
	url);
    return key_temp_buffer;
}


const cache_key *
storeKeyPublic(const char *url, method_t method)
{
    debug(20, 3) ("storeKeyPublic: type=%d %s\n", method, url);
    switch (method) {
    case METHOD_GET:
	return url;
	/* NOTREACHED */
	break;
    case METHOD_POST:
    case METHOD_PUT:
    case METHOD_HEAD:
    case METHOD_CONNECT:
    case METHOD_TRACE:
	snprintf(key_temp_buffer, MAX_URL + 100, "/%s/%s", RequestMethodStr[method], url);
	return key_temp_buffer;
	/* NOTREACHED */
	break;
    default:
	fatal_dump("storeKeyPublic: Unsupported request method");
	break;
    }
    return NULL;
}

const cache_key *
storeKeyDup(const cache_key * key)
{
    meta_data.store_keys += strlen(key);
    return xstrdup(key);
}

void
storeKeyFree(const cache_key * key)
{
    meta_data.store_keys -= strlen(key);
    free((void *) key);
}

int
storeKeyHashBuckets(int nobj)
{
    if (nobj < 8192)
	return 7951;
    if (nobj < 12288)
	return 12149;
    if (nobj < 16384)
	return 16231;
    if (nobj < 32768)
	return 33493;
    return 65357;
}

#endif /* STORE_KEY_URL */
