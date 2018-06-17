#ifndef READERWRITERLOCK_H
#define READERWRITERLOCK_H


class ReaderWriterLock
{

public:
    ReaderWriterLock();

    void lockReading();
    void lockWriting();
    void unlockReading();
    void unlockWriting();
};

#endif // READERWRITERLOCK_H
