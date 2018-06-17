#include <QString>
#include <QtTest>
#include <QTime>

#include "readerwritercache.h"
#include "tst_readerwriterchache.h"


ReaderWriterCacheTest::ReaderWriterCacheTest()
{
}


void ReaderWriterCacheTest::initTestCase()
{
}




void ReaderWriterCacheTest::testSingleReadWrite()
{
    initTestCase();
    ReaderWriterCache cache(1,5);

    Request req("This is a request", "clientId");
    Response resp(req, "response");

    cache.putResponse(resp);

    {
        Option<Response> newResp = cache.tryGetCachedResponse(req);
        QVERIFY2(newResp.hasValue(), "Response not found");
        QVERIFY2(newResp.value().getResponse() == resp.getResponse(), "Response not identical to original");
    }
}


void ReaderWriterCacheTest::testSingleReadWriteWithStale()
{
    initTestCase();
    ReaderWriterCache cache(1,5);

    Request req("This is a request", "clientId");
    Response resp(req, "response");

    cache.putResponse(resp);

    {
        Option<Response> newResp = cache.tryGetCachedResponse(req);
        QVERIFY2(newResp.hasValue(), "Response not found");
        QVERIFY2(newResp.value().getResponse() == resp.getResponse(), "Response not identical to original");
    }

    // Wait for 8 seconds (stale time is 5 seconds)
    QThread::usleep(8000000);
    {
        Option<Response> newResp = cache.tryGetCachedResponse(req);
        QVERIFY2(!newResp.hasValue(), "Response should not be in the cache");
    }
}

void ReaderWriterCacheTest::testNotInCache()
{
    initTestCase();
    ReaderWriterCache cache(1,5);

    Request req("This is a request", "clientId");
    Request req2("This is a request another", "clientId");
    Response resp(req, "response");

    cache.putResponse(resp);

    {
        Option<Response> newResp = cache.tryGetCachedResponse(req2);
        QVERIFY2(!newResp.hasValue(), "Response should not be in the cache");
    }
}

void ReaderWriterCacheTest::test2ReadWrite()
{
    initTestCase();
    ReaderWriterCache cache(1,10);

    Request req1("req1", "id1");
    Request req2("req2", "id2");
    Response resp1(req1, "response1");
    Response resp2(req2, "response2");

    cache.putResponse(resp1);
    cache.putResponse(resp2);

    {
        Option<Response> newResp = cache.tryGetCachedResponse(req1);
        QVERIFY2(newResp.hasValue(), "Response not found");
        QVERIFY2(newResp.value().getResponse() == resp1.getResponse(), "Response not identical to original");
    }

    {
        Option<Response> newResp = cache.tryGetCachedResponse(req2);
        QVERIFY2(newResp.hasValue(), "Response not found");
        QVERIFY2(newResp.value().getResponse() == resp2.getResponse(), "Response not identical to original");
    }
}

QTEST_APPLESS_MAIN(ReaderWriterCacheTest)
