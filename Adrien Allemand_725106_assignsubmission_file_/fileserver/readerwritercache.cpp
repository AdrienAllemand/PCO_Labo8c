#include "readerwritercache.h"
#include "option.h"
#include <QDateTime>
#include <QList>
#include <QDebug>

ReaderWriterCache::ReaderWriterCache(int invalidationDelaySec, int staleDelaySec):
    invalidationDelaySec(invalidationDelaySec), staleDelaySec(staleDelaySec)
{
}

ReaderWriterCache::~ReaderWriterCache()
{

void ReaderWriterCache::putResponse(Response &response) {
}

Option<Response> ReaderWriterCache::tryGetCachedResponse(Request &request) {
    Option<Response> res = Option<Response>::none();
    return res;
}
