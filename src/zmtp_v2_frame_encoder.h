/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.*/

#include "pdu.h"
#include "iobuf.h"

#ifndef __ZMTP_V2_FRAME_ENCODER_H_INCLUDED__
#define __ZMTP_V2_FRAME_ENCODER_H_INCLUDED__

#define ZMTP_V2_FRAME_ENCODER_READY     0x01
#define ZMTP_V2_FRAME_ENCODER_READ_OK   0x02

typedef struct zmtp_v2_frame_encoder zmtp_v2_frame_encoder_t;

struct zmtp_v2_frame_encoder_info {
    unsigned int flags;
    uint8_t *buffer;
    size_t buffer_size;
};

typedef struct zmtp_v2_frame_encoder_info zmtp_v2_frame_encoder_info_t;

zmtp_v2_frame_encoder_t *
    zmtp_v2_frame_encoder_new (zmtp_v2_frame_encoder_info_t *info);

int
    zmtp_v2_frame_encoder_putmsg (zmtp_v2_frame_encoder_t *self,
        pdu_t *pdu, zmtp_v2_frame_encoder_info_t *info);

int
    zmtp_v2_frame_encoder_read (zmtp_v2_frame_encoder_t *self,
        iobuf_t *iobuf, zmtp_v2_frame_encoder_info_t *info);

int
    zmtp_v2_frame_encoder_advance (zmtp_v2_frame_encoder_t *self,
        size_t n, zmtp_v2_frame_encoder_info_t *info);

void
    zmtp_v2_frame_encoder_destroy (zmtp_v2_frame_encoder_t **base_p);

#endif
