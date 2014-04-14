//  TCP connector class

#define _POSIX_C_SOURCE 1

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "mailbox.h"
#include "tcp_connector.h"

struct tcp_connector {
    int fd;
    int err;
    mailbox_t *owner;
};

tcp_connector_t *
tcp_connector_new (mailbox_t *owner)
{
    tcp_connector_t *self = malloc (sizeof *self);
    if (self)
        *self = (tcp_connector_t) { .fd = -1, .owner = owner };
    return self;
}

void
tcp_connector_destroy (tcp_connector_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        tcp_connector_t *self = *self_p;
        if (self->fd != -1 && self->err != 0) {
            const int rc = close (self->fd);
            assert (rc == 0);
        }
        free (self);
        *self_p = NULL;
    }
}

int
tcp_connector_connect (tcp_connector_t *self, unsigned short port)
{
    assert (self);

    if (self->fd != -1)
        return -1;

    //  Create socket
    const int s = socket (AF_INET, SOCK_STREAM, 0);
    if (s == -1)
        return -1;
    //  Resolve address
    const struct addrinfo hints = {
        .ai_family   = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_flags    = AI_NUMERICHOST | AI_NUMERICSERV
    };
    char service [8 + 1];
    snprintf (service, sizeof service, "%u", port);
    struct addrinfo *result = NULL;
    if (getaddrinfo ("127.0.0.1", service, &hints, &result)) {
        close (s);
        return -1;
    }
    assert (result);
    //  Set non-blocking mode
    const int flags = fcntl (s, F_GETFL, 0);
    assert (flags != -1);
    int rc = fcntl (s, F_SETFL, flags | O_NONBLOCK);
    assert (rc == 0);
    //  Initiate TCP connection
    rc = connect (s, result->ai_addr, result->ai_addrlen);
    if (rc == -1)
        self->err = errno;
    freeaddrinfo (result);
    self->fd = s;
    return rc;
}

int
tcp_connector_errno (tcp_connector_t *self)
{
    assert (self);
    return self->err;
}

int
tcp_connector_fd (tcp_connector_t *self)
{
    assert (self);
    return self->fd;
}

static int
io_event (void *self_, uint32_t flags, uint32_t *timer_interval)
{
    tcp_connector_t *self = (tcp_connector_t *) self_;
    assert (self);
    socklen_t len = sizeof self->err;
    const int rc = getsockopt (
       self->fd, SOL_SOCKET, SO_ERROR, &self->err, &len);
    assert (rc == 0);
    if (self->err == 0)
        return 0;
    else
    if (self->err != EINPROGRESS)
        return 0;
    else
        return 3;
}

static void
io_error (void *self_)
{
    printf ("tcp_connector: I/O error\n");
}

struct io_handler
tcp_connector_io_handler (tcp_connector_t *self)
{
    static struct io_handler_ops ops = {
        .event = io_event,
        .error = io_error
    };
    assert (self);
    return (struct io_handler) { .object = self, .ops = &ops };
}