#include <stdio.h>
#include <string>
#include <iostream>
#include <string.h>
#include "anet.h"
#include "ae.h"

int connected = 0;
char *msg = "GET / HTTP/1.0\r\nHOST: www.baidu.com\r\nAccept: */*\r\nConnection: close\r\n\r\n";
int sendPos = 0;
int sendnum = 0;

char recvMsg[20480] = {0};
int recvPos = 0;
int recvnum = 0;

void ioProc(aeEventLoop *eventLoop, int fd, void *clientData, int mask)
{
    if (connected == 0)
    {
        if ((mask & AE_WRITABLE) == AE_WRITABLE)
        {
            if (anetGetSocketError(fd) == 0)
            {
                // 建立连接成功
                connected = 1;
                std::cout << "connect success" << std::endl;
            }
            else
            {
                aeDeleteFileEvent(eventLoop, fd, AE_WRITABLE);
            }
        }
    }
    else
    {
        if ((mask & AE_WRITABLE) == AE_WRITABLE)
        {
            sendnum = strlen(msg);
            sendPos = 0;

            int err = ANET_OK;
            int nsend = anetWrite(fd, msg + sendPos, sendnum, &err);
            if (err == ANET_ERR)
            {
                std::cout << "send data error" << std::endl;
                aeDeleteFileEvent(eventLoop, fd, AE_WRITABLE);
            }
            else
            {
                sendnum -= nsend;
                sendPos += nsend;

                if (sendnum == 0)
                {
                    aeDeleteFileEvent(eventLoop, fd, AE_WRITABLE);
                    aeCreateFileEvent(eventLoop, fd, AE_READABLE, ioProc, nullptr);

                    recvPos = 0;
                    recvnum = 20480;
                }
            }
        }
        else
        {
            int err = ANET_OK;
            int nrecv = anetRead(fd, recvMsg + recvPos, recvnum, &err);
            if (err == ANET_ERR)
            {
                aeDeleteFileEvent(eventLoop, fd, AE_READABLE);
                std::cout << "read data error" << std::endl;
            }
            else
            {
                recvPos += nrecv;
                recvnum -= nrecv;

                if (nrecv == 0)
                {
                    aeDeleteFileEvent(eventLoop, fd, AE_READABLE);
                    std::cout << recvMsg << std::endl;
                }
            }

        }
    }
}

int timeProc(aeEventLoop *eventLoop, long long id, void *clientData)
{
    return AE_OK;
}

void finalizerProc(struct aeEventLoop *eventLoop, void *clientData)
{

}

int main()
{
    char err[ANET_ERR_LEN] = {0};
    aeEventLoop * loop = aeCreateEventLoop(1024);

    aeCreateTimeEvent(loop, 100, timeProc, nullptr, finalizerProc);

    int fd = anetTcpNonBlockConnect(err, AF_INET, "220.181.111.188", 80);
    aeCreateFileEvent(loop, fd, AE_WRITABLE, ioProc, nullptr);


    aeMain(loop);

    return 0;
}
