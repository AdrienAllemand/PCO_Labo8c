#ifndef TST_READERWRITERCACHE_H
#define TST_READERWRITERCACHE_H

#include <QString>
#include <QtTest>
#include <QTime>

#include "readerwritercache.h"

///
/// \brief The ThreadpoolTest class
/// This class embeds all the tests of the ThreadPool
class ReaderWriterCacheTest : public QObject
{
    Q_OBJECT

public:
    ReaderWriterCacheTest();

private:

    ///
    /// \brief initTestCase Function to init the testcase.
    /// Needs to be called at the beginning of each testcase.
    void initTestCase();

private Q_SLOTS:

    void testSingleReadWrite();

    void testSingleReadWriteWithStale();

    void testNotInCache();

    void test2ReadWrite();
};

#endif // TST_READERWRITERCACHE_H
