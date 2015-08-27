//
//  mem_functions.h
//  camera-sdk
//
//  Created by 0day on 15/6/12.
//  Copyright (c) 2015年 Pili Engineering. All rights reserved.
//

#ifndef camera_sdk_mem_functions_h
#define camera_sdk_mem_functions_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *put_buff(uint8_t *data, const uint8_t *src, size_t src_size) {
    memcpy(data, src, src_size);
    return data + src_size;
}

static uint8_t *put_byte(uint8_t *data, uint8_t val) {
    data[0] = val;
    return data + 1;
}

static uint8_t *put_be16(uint8_t *data, short val)
{
    char buf[2];
    buf[1] = val & 0xff;
    buf[0] = (val >> 8) & 0xff;
    
    return put_buff(data, (const uint8_t *)buf, sizeof(uint16_t));
}

static int get_be16(uint8_t* val) {
    return ((val[0] & 0xff) << 8) | ((val[1] & 0xff)) ;
}

static uint8_t *put_be24(uint8_t *data, int32_t val)
{
    char buf[3];
    
    buf[2] = val & 0xff;
    buf[1] = (val >> 8) & 0xff;
    buf[0] = (val >> 16) & 0xff;
    
    return put_buff(data, (const uint8_t *)buf, 3);
}

static int get_be24(uint8_t* val) {
    int ret = ((val[2]&0xff)) | ((val[1]&0xff) << 8) | ((val[0]&0xff)<<16) ;
    return ret;
}

static uint8_t *put_be32(uint8_t *data, int32_t val)
{
    char buf[4];
    
    buf[3] = val & 0xff;
    buf[2] = (val >> 8) & 0xff;
    buf[1] = (val >> 16) & 0xff;
    buf[0] = (val >> 24) & 0xff;
    
    return put_buff(data, (const uint8_t *)buf, sizeof(int32_t));
}
static int get_be32(uint8_t* val) {
    return ((val[0]&0xff)<<24) | ((val[1]&0xff)<<16) | ((val[2]&0xff) << 8) | ((val[3]&0xff)) ;
}

static uint8_t *put_tag(uint8_t *data, uint8_t *tag)
{
    uint8_t *p = data;
    while (*tag) {
        p = put_byte(p, *tag++);
    }
    
    return p;
}

static uint8_t *put_string(uint8_t *data, const char *str, size_t str_length) {
    uint8_t *p = data;
    if(str_length < 0xFFFF) {
        p = put_byte(p, kAMFString);
        p = put_be16(p, str_length);
    } else {
        p = put_byte(p, kAMFLongString);
        p = put_be32(p, (int32_t)str_length);
    }
    p = put_buff(p, (const uint8_t*)str, str_length);
    
    return p;
}

static uint8_t *put_double(uint8_t *data, double val) {
    uint8_t *p = data;
    p = put_byte(p, kAMFNumber);
    
    p = put_buff(p, (uint8_t *)&val, sizeof(int64_t));
    return p;
}

static uint8_t *put_bool(uint8_t *data, int val) {
    uint8_t *p = data;
    p = put_byte(p, kAMFBoolean);
    p = put_byte(p, val);
    return p;
}

static uint8_t *put_name(uint8_t *data, const char *name, size_t name_length) {
    uint8_t *p = data;
    p = put_be16(p, name_length);
    p = put_buff(p, (uint8_t*)name, name_length);
    return p;
}

static uint8_t *put_named_double(uint8_t *data, const char *name, size_t name_length, double val) {
    uint8_t *p = data;
    p = put_name(p, name, name_length);
    p = put_double(p, val);
    return p;
}

static uint8_t *put_named_string(uint8_t *data, const char *name, size_t name_length, const char *val, size_t val_length) {
    uint8_t *p = data;
    p = put_name(p, name, name_length);
    p = put_string(p, val, val_length);
    return p;
}

static uint8_t *put_named_bool(uint8_t *data, const char *name, size_t name_length, int val) {
    uint8_t *p = data;
    p = put_name(p, name, name_length);
    p = put_bool(p, val);
    return p;
}

#endif
