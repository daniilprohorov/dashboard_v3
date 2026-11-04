#pragma once

#include <stddef.h>
#include <stdint.h>

#include "logger.h"

#define LOGGER_LINE_MAX 1024

typedef struct {
    uint32_t low;
    uint32_t high;
} logger_range_t;

typedef struct {
    uint32_t bitrate;
    logger_ids_t ids;
    uint64_t duration_ms;
    size_t range_count; // zero means all IDs of the selected format(s)
    logger_range_t ranges[LOGGER_MAX_RANGES];
    char filter[LOGGER_FILTER_TEXT_SIZE];
} logger_config_t;

typedef enum {
    LOGGER_CMD_HELLO,
    LOGGER_CMD_STATUS,
    LOGGER_CMD_CAPS,
    LOGGER_CMD_START,
    LOGGER_CMD_STOP,
    LOGGER_CMD_ACK,
    LOGGER_CMD_PING,
} logger_command_t;

typedef struct {
    uint32_t rid; // zero only when the malformed request has no usable rid
    logger_command_t command;
    logger_config_t config;
    char sid[LOGGER_SESSION_TEXT_SIZE];
    uint64_t ack;
    logger_reason_t reason;
} logger_request_t;

// Task-owned, in-place parser. Returns a frozen wire error token or NULL.
// Normalizes the allowlist before any hardware side effect; scratch is bounded.
const char *logger_request_parse(char *line, logger_request_t *out);
