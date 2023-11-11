#include "Dispatcher.h"
#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define Max 520
struct EpollData
{
    int epfd;
    struct epoll_event* events;
};
static void* epollInit();
static int epollAdd(struct Channel* channel, struct EventLoop* evLoop);
static int epollRemove(struct Channel* channel, struct EventLoop* evLoop);
static int epollModify(struct Channel* channel, struct EventLoop* evLoop);
static int epollDispatch(struct EventLoop* evLoop, int timeout); // 单位: s
static int epollClear(struct EventLoop* evLoop);
static int epollCtl(struct Channel* channel, struct EventLoop* evLoop, int op);

struct Dispatcher EpollDispatcher = {
    epollInit,
    epollAdd,
    epollRemove,
    epollModify,
    epollDispatch,
    epollClear
};

static void* epollInit()
{
    struct EpollData* data = (struct EpollData*)malloc(sizeof(struct EpollData));
    data->epfd = epoll_create(10);
    if (data->epfd == -1)
    {
        perror("epoll_create");
        exit(0);
    }
    data->events = (struct epoll_event*)calloc(Max, sizeof(struct epoll_event));

    return data;
}

static int epollCtl(struct Channel* channel, struct EventLoop* evLoop, int op)
{
    struct EpollData* data = (struct EpollData*)evLoop->dispatcherData;
    struct epoll_event ev;
    ev.data.fd = channel->fd;
    int events = 0;
    if (channel->events & ReadEvent)
    {
        events |= EPOLLIN;
    }
    if (channel->events & WriteEvent)
    {
        events |= EPOLLOUT;
    }
    ev.events = events;
    int ret = epoll_ctl(data->epfd, op, channel->fd, &ev);
    return ret;
}

static int epollAdd(struct Channel* channel, struct EventLoop* evLoop)
{ 
    int ret = epollCtl(channel, evLoop, EPOLL_CTL_ADD);
    if (ret == -1)
    {
        perror("epoll_crl add");
        exit(0);
    }
    return ret;
}

static int epollRemove(struct Channel* channel, struct EventLoop* evLoop)
{
    int ret = epollCtl(channel, evLoop, EPOLL_CTL_DEL);
    if (ret == -1)
    {
        perror("epoll_crl delete");
        exit(0);
    }
    // 通过 channel 释放对应的 TcpConnection 资源
    channel->destroyCallback(channel->arg);

    return ret;
}

static int epollModify(struct Channel* channel, struct EventLoop* evLoop)
{
    int ret = epollCtl(channel, evLoop, EPOLL_CTL_MOD);
    if (ret == -1)
    {
        perror("epoll_crl modify");
        exit(0);
    }
    return ret;
}

static int epollDispatch(struct EventLoop* evLoop, int timeout)
{
    struct EpollData* data = (struct EpollData*)evLoop->dispatcherData;
    int count = epoll_wait(data->epfd, data->events, Max, timeout * 1000);
    for (int i = 0; i < count; ++i)
    {
        int events = data->events[i].events;
        int fd = data->events[i].data.fd;
        if (events & EPOLLERR || events & EPOLLHUP)
        {
            // 对方断开了连接, 删除 fd
            // epollRemove(Channel, evLoop);
            continue;
        }
        if (events & EPOLLIN)
        {
            eventActivate(evLoop, fd, ReadEvent);
        }
        if (events & EPOLLOUT)
        {
            eventActivate(evLoop, fd, WriteEvent);
        }
    }
    return 0;
}

static int epollClear(struct EventLoop* evLoop)
{
    struct EpollData* data = (struct EpollData*)evLoop->dispatcherData;
    free(data->events);
    close(data->epfd);
    free(data);
    return 0;
}
