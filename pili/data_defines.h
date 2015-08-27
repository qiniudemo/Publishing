//
//  data_defines.h
//  camera-sdk
//
//  Created on 15/1/26.
//  Copyright (c) Pili Engineering, Qiniu Inc. All rights reserved.
//

#ifndef camera_sdk_data_defines_h
#define camera_sdk_data_defines_h

#include <stdbool.h>
#include <stdlib.h>

#include "pili_type.h"

PILI_TYPE(struct pili_packet_buffer, pili_packet_buffer_t);
PILI_TYPE_POINTER(struct pili_packet_buffer, pili_packet_buffer_p);

struct pili_packet_buffer {
    RTMPPacket                *packet;
    struct pili_packet_buffer *prev;
    struct pili_packet_buffer *next;
};

/**
 * Alloc a pili_packet_buffer_p.
 * @param buffer a pointer wait for memory alloc.
 * @return The alloc result, 0 for success, -1 for memory alloc failure.
 */
pili_packet_buffer_p pili_create_packet_buffer();

/**
 * Init a pili_packet_buffer_p.
 * @param buffer    the buffer to be inited.
 * @param data      inside rtmp packet.
 * @param prev      queue's prev node.
 * @param next      queue's next node
 * @return Init result, 0 for success, -1 for failure. Failure happens when buffer is NULL.
 */
int pili_init_packet_buffer(pili_packet_buffer_p buffer,
                            RTMPPacket           *packet,
                            pili_packet_buffer_p prev,
                            pili_packet_buffer_p next);

/**
 * Release a pili_packet_buffer_p.
 * @param buffer to be released.
 * @return Release result, 0 for success.
 */
int pili_release_packet_buffer(pili_packet_buffer_p buffer);


typedef void (*pili_packet_queue_release_cb)(pili_stream_context_p ctx, RTMPPacket *packet);
typedef void (*pili_packet_queue_drop_one_packet_cb)(pili_stream_context_p ctx, RTMPPacket *packet);

struct pili_packet_queue {
    pili_packet_buffer_p                    head;
    int                                     length;
    int                                     capacity;
    uint8_t                                 drop_frame_policy;
    pili_stream_context_p                   context;
    pili_packet_queue_release_cb            release_cb;
    pili_packet_queue_drop_one_packet_cb    drop_packet_cb;
};

typedef struct pili_packet_queue pili_packet_queue_t;

/**
 * Alloc a pili_packet_queue.
 * @param queue a pointer wait for memory alloc.
 * @return The alloc result, 0 for success, -1 for memory alloc failure.
 */
pili_packet_queue_p pili_create_queue();

/**
 * Init a pili_packt_queue.
 * @param queue                 the queue to be inited, you should always call this function after pili_queue_create() called before you use the queue.
 * @param capacity              the capacity of the queue.
 * @param drop_frame_strategy   this param will influence how a packet be droped when queue is full.
 * @param release_cb            when queue will be released, this callback function will be invoked. It give you a chance to release pili_packet_buffer_p.
 * @param drop_packet_cb        when a packet will be droped, this callback function will be invoked. It give you a chance to release pili_packet_buffer_p.
 * @return              Init result, 0 for success, -1 for failure. Failure happens when queue is NULL.
 */
int pili_init_queue(pili_packet_queue_p queue,
                    int capacity,
                    int drop_frame_strategy,
                    pili_stream_context_p context,
                    pili_packet_queue_release_cb release_cb,
                    pili_packet_queue_drop_one_packet_cb drop_packet_cb);

/**
 * Release a pili_packet_queue.
 * @param queue the queue to be released.
 * @return Release result, 0 for success.
 */
int pili_release_queue(pili_packet_queue_p queue);

/**
 * Check to see if the queue is full.
 * @param queue the queue to be checked.
 * @return Check result, true for full, false for not full.
 */
bool pili_queue_is_full(pili_packet_queue_p queue);

/**
 * Check to see if the queue is empty.
 * @param queue the queue to be checked.
 * return Check result, true for empty, false for not empty.
 */
bool pili_queue_is_empty(pili_packet_queue_p queue);

/**
 * Length of a queue.
 * @param queue the queue you wanna check.
 * @return Length of the queue. -1 will be returned if queue is NULL.
 */
int pili_queue_length(pili_packet_queue_p queue);

/**
 * Get a packet buffer from queue.
 * @param queue         the queue to access.
 * @param index         which one you want to pick.
 * @param packet_buffer the result.
 * @return 0 for success, -1 for failure. Failure happens when queue is NULL or index is out of range.
 */
int pili_queue_get_packet_buffer(pili_packet_queue_p queue, int index, pili_packet_buffer_p *packet_buffer);

/**
 * Get first packet buffer from queue.
 * @param queue     the queue to access.
 * @packet_buffer   the result.
 * @return 0 for success, -1 for failure. Failure happens when queue is NULL or index is out of range.
 */
int pili_queue_get_first_packet_buffer(pili_packet_queue_p queue, pili_packet_buffer_p *packet_buffer);

/**
 * Get last packet buffer from queue.
 * @param queue     the queue to access.
 * @packet_buffer   the result.
 * @return 0 for success, -1 for failure. Failure happens when queue is NULL or index is out of range.
 */
int pili_queue_get_last_packet_buffer(pili_packet_queue_p queue, pili_packet_buffer_p *packet_buffer);

/**
 * Drop a packet from a queue.
 * @param queue the queue to access.
 * @return 0 for success, -1 for failure. Failure happens when queue is NULL.
 */
int pili_queue_drop_one_packet(pili_packet_queue_p queue);

/**
 * Write a packet to a queue.
 * @param queue the queue to access.
 * @return 0 for success, -1 & -2  for failure. -1 happens when queue is NULL or packet is NULL, -2 returned when queue is full.
 */
int pili_queue_write_one_packet(pili_packet_queue_p queue, RTMPPacket *packet);

/**
 * Drop a packt.
 * @param queue the queue to access.
 * @return 0 for success, -1 & -2 for failure. -1 returned when queue is NULL, -2 returned when queue is empty.
 */
int pili_queue_read_one_packet(pili_packet_queue_p queue, RTMPPacket **packet);


#endif
