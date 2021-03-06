/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *     Copyright 2010, 2011 Couchbase, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#include "internal.h"

/**
 * Spool a store request
 *
 * @author Trond Norbye
 * @todo add documentation
 * @todo fix the expiration so that it works relative/absolute etc..
 * @todo we might want to wait to write the data to the sockets if the
 *       user want to run a batch of store requests?
 */
LIBCOUCHBASE_API
libcouchbase_error_t libcouchbase_store(libcouchbase_t instance,
                                        const void *command_cookie,
                                        libcouchbase_storage_t operation,
                                        const void *key, libcouchbase_size_t nkey,
                                        const void *bytes, libcouchbase_size_t nbytes,
                                        libcouchbase_uint32_t flags, libcouchbase_time_t exp,
                                        libcouchbase_cas_t cas)
{
    return libcouchbase_store_by_key(instance, command_cookie, operation, NULL, 0,
                                     key, nkey, bytes, nbytes, flags, exp, cas);
}

libcouchbase_error_t libcouchbase_store_by_key(libcouchbase_t instance,
                                               const void *command_cookie,
                                               libcouchbase_storage_t operation,
                                               const void *hashkey,
                                               libcouchbase_size_t nhashkey,
                                               const void *key, libcouchbase_size_t nkey,
                                               const void *bytes, libcouchbase_size_t nbytes,
                                               libcouchbase_uint32_t flags, libcouchbase_time_t exp,
                                               libcouchbase_cas_t cas)
{
    libcouchbase_server_t *server;
    protocol_binary_request_set req;
    libcouchbase_size_t headersize;
    libcouchbase_size_t bodylen;
    int vb, idx;

    /* we need a vbucket config before we can start getting data.. */
    if (instance->vbucket_config == NULL) {
        return libcouchbase_synchandler_return(instance, LIBCOUCHBASE_ETMPFAIL);
    }

    if (nhashkey == 0) {
        nhashkey = nkey;
        hashkey = key;
    }
    (void)vbucket_map(instance->vbucket_config, hashkey, nhashkey, &vb, &idx);
    if (idx < 0 || idx > (int)instance->nservers) {
        /* the config says that there is no server yet at that position (-1) */
        return libcouchbase_synchandler_return(instance, LIBCOUCHBASE_NETWORK_ERROR);
    }
    server = instance->servers + idx;

    memset(&req, 0, sizeof(req));
    req.message.header.request.magic = PROTOCOL_BINARY_REQ;
    req.message.header.request.keylen = ntohs((libcouchbase_uint16_t)nkey);
    req.message.header.request.extlen = 8;
    req.message.header.request.datatype = PROTOCOL_BINARY_RAW_BYTES;
    req.message.header.request.vbucket = ntohs((libcouchbase_uint16_t)vb);
    req.message.header.request.opaque = ++instance->seqno;
    req.message.header.request.cas = cas;
    req.message.body.flags = htonl(flags);
    req.message.body.expiration = htonl((libcouchbase_uint32_t)exp);

    headersize = sizeof(req.bytes);
    switch (operation) {
    case LIBCOUCHBASE_ADD:
        req.message.header.request.opcode = PROTOCOL_BINARY_CMD_ADD;
        break;
    case LIBCOUCHBASE_REPLACE:
        req.message.header.request.opcode = PROTOCOL_BINARY_CMD_REPLACE;
        break;
    case LIBCOUCHBASE_SET:
        req.message.header.request.opcode = PROTOCOL_BINARY_CMD_SET;
        break;
    case LIBCOUCHBASE_APPEND:
        req.message.header.request.opcode = PROTOCOL_BINARY_CMD_APPEND;
        req.message.header.request.extlen = 0;
        headersize -= 8;
        break;
    case LIBCOUCHBASE_PREPEND:
        req.message.header.request.opcode = PROTOCOL_BINARY_CMD_PREPEND;
        req.message.header.request.extlen = 0;
        headersize -= 8;
        break;
    default:
        /* We were given an unknown storage operation. */
        return libcouchbase_synchandler_return(instance,
                                               libcouchbase_error_handler(instance, LIBCOUCHBASE_EINVAL,
                                                                          "Invalid value passed as storage operation"));
    }

    /* Make it known that this was a success. */
    libcouchbase_error_handler(instance, LIBCOUCHBASE_SUCCESS, NULL);

    bodylen = nkey + nbytes + req.message.header.request.extlen;
    req.message.header.request.bodylen = htonl((libcouchbase_uint32_t)bodylen);

    libcouchbase_server_start_packet(server, command_cookie, &req, headersize);
    libcouchbase_server_write_packet(server, key, nkey);
    libcouchbase_server_write_packet(server, bytes, nbytes);
    libcouchbase_server_end_packet(server);
    libcouchbase_server_send_packets(server);

    return libcouchbase_synchandler_return(instance, LIBCOUCHBASE_SUCCESS);
}
