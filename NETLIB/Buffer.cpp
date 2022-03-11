#include "Buffer.h"
#include "SocketsOps.h"

#include <errno.h>
#include <sys/uio.h>

const char Buffer::kCRLF[] = "\r\n";
const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

ssize_t Buffer::readFd(int fd, int *savedErrno)
{
    // saved an ioctl()/FIONREAD call to tell how much to read
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    // when there is enough space in this buffer, don't read into extrabuf.
    // when extrabuf is used, we read 128k-1 bytes at most.
    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = readv_(fd, vec, iovcnt);
    if (n < 0)
    {
        *savedErrno = errno;
    }
    else if ((size_t)(n) <= writable)
    {
        writerIndex_ += n;
    }
    else
    {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    return n;
}

ssize_t Buffer::bufReadFd(int fd)
{
    int writable = writableBytes();
    char* writeBegin = begin() + writerIndex_;
    ssize_t n = read(fd, writeBegin, writable);
    //如果n=0,说明读完了，n<0说明出现了错误,如果n=writable,说明可能没有读完
    if(n < 0)
    {
        perror("Buffer::bufReadFd():");
    }
    else if((size_t)(n) < writable)
    {
        writerIndex_ += n;      //读完了，数据也写进buf了
    }
    else if(n == writable)  //  可能还没有读完，这时候需要增加buf的大小
    {
        writerIndex_ += n;      //先把读指针放在正确的位置
        makeSpace(65536);       //再扩大buf的大小
        bufReadFd(fd);      //再读一次
    }
    return n;
}