#include "logger_protocol.h"

#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Only the protocol task parses. Even a maximally fragmented 1024-byte input
// fits here; limiting raw elements to 16 would reject valid merged allowlists.
static logger_range_t s_parse_ranges[LOGGER_LINE_MAX / 2 + 1];

static bool decimal(const char *text, uint64_t maximum, uint64_t *out)
{
    if (!text || !*text) {
        return false;
    }
    uint64_t value = 0;
    for (const char *p = text; *p; ++p) {
        if (*p < '0' || *p > '9') {
            return false;
        }
        unsigned digit = (unsigned)(*p - '0');
        if (value > (maximum - digit) / 10) {
            return false;
        }
        value = value * 10 + digit;
    }
    *out = value;
    return true;
}

static bool hex_bound(const char **cursor, uint32_t *out)
{
    const char *p = *cursor;
    uint32_t value = 0;
    unsigned digits = 0;
    while ((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'F') || (*p >= 'a' && *p <= 'f')) {
        unsigned digit = *p <= '9' ? (unsigned)(*p - '0') :
                         *p <= 'F' ? (unsigned)(*p - 'A' + 10) : (unsigned)(*p - 'a' + 10);
        if (++digits > 8) {
            return false;
        }
        value = value * 16 + digit;
        ++p;
    }
    if (!digits || value > 0x1FFFFFFF) {
        return false;
    }
    *cursor = p;
    *out = value;
    return true;
}

static int range_compare(const void *left, const void *right)
{
    const logger_range_t *a = left;
    const logger_range_t *b = right;
    return (a->low > b->low) - (a->low < b->low);
}

static bool parse_filter(const char *text, logger_config_t *config)
{
    if (strcmp(text, "all") == 0) {
        strcpy(config->filter, "all");
        return true;
    }
    size_t count = 0;
    const char *p = text;
    do {
        if (count == sizeof s_parse_ranges / sizeof s_parse_ranges[0]) {
            return false;
        }
        logger_range_t *range = &s_parse_ranges[count++];
        if (!hex_bound(&p, &range->low)) {
            return false;
        }
        range->high = range->low;
        if (*p == '-') {
            ++p;
            if (!hex_bound(&p, &range->high) || range->high < range->low) {
                return false;
            }
        }
        if (config->ids == LOGGER_IDS_11 && range->high > 0x7FF) {
            return false;
        }
        if (!*p) {
            break;
        }
        if (*p++ != ',' || !*p) {
            return false;
        }
    } while (true);

    qsort(s_parse_ranges, count, sizeof s_parse_ranges[0], range_compare);
    for (size_t i = 0; i < count; ++i) {
        logger_range_t range = s_parse_ranges[i];
        if (config->range_count && range.low <= config->ranges[config->range_count - 1].high + 1) {
            logger_range_t *last = &config->ranges[config->range_count - 1];
            if (range.high > last->high) {
                last->high = range.high;
            }
        } else {
            if (config->range_count == LOGGER_MAX_RANGES) {
                return false;
            }
            config->ranges[config->range_count++] = range;
        }
    }
    size_t used = 0;
    int width = config->ids == LOGGER_IDS_11 ? 3 : 8;
    for (size_t i = 0; i < config->range_count; ++i) {
        const logger_range_t *range = &config->ranges[i];
        int written;
        if (range->low == range->high) {
            written = snprintf(config->filter + used, sizeof config->filter - used,
                               "%s%0*" PRIX32, i ? "," : "", width, range->low);
        } else {
            written = snprintf(config->filter + used, sizeof config->filter - used,
                               "%s%0*" PRIX32 "-%0*" PRIX32,
                               i ? "," : "", width, range->low, width, range->high);
        }
        if (written < 0 || (size_t)written >= sizeof config->filter - used) {
            return false;
        }
        used += (size_t)written;
    }
    return true;
}

static bool session_id(const char *text)
{
    if (!text || strlen(text) != 16) {
        return false;
    }
    for (size_t i = 0; i < 16; ++i) {
        if (!((text[i] >= '0' && text[i] <= '9') || (text[i] >= 'A' && text[i] <= 'F'))) {
            return false;
        }
    }
    return true;
}

const char *logger_request_parse(char *line, logger_request_t *out)
{
    memset(out, 0, sizeof *out);
    char *save = NULL;
    char *version = strtok_r(line, " ", &save);
    char *kind = strtok_r(NULL, " ", &save);
    char *rid = strtok_r(NULL, " ", &save);
    char *command = strtok_r(NULL, " ", &save);
    uint64_t number;
    if (rid && decimal(rid, INT32_MAX, &number) && number) {
        out->rid = (uint32_t)number;
    }
    if (!version || version[0] != '@' || version[1] != 'D' ||
        !decimal(version + 2, UINT32_MAX, &number) || !kind || strcmp(kind, "Q") || !out->rid) {
        return "invalid_command";
    }
    if (strcmp(version, "@D1")) {
        return "unsupported_version";
    }
    if (!command) {
        return "invalid_command";
    }
    static const char *const commands[] = {"HELLO", "STATUS", "CAPS", "START", "STOP", "ACK", "PING"};
    size_t index;
    for (index = 0; index < sizeof commands / sizeof commands[0]; ++index) {
        if (!strcmp(command, commands[index])) {
            break;
        }
    }
    if (index == sizeof commands / sizeof commands[0]) {
        return "invalid_command";
    }
    out->command = (logger_command_t)index;

    char *keys[4];
    char *values[4];
    size_t argc = 0;
    for (char *arg = strtok_r(NULL, " ", &save); arg; arg = strtok_r(NULL, " ", &save)) {
        if (argc == 4) {
            return "invalid_argument";
        }
        char *equals = strchr(arg, '=');
        if (!equals || equals == arg || !equals[1] || strchr(equals + 1, '=')) {
            return "invalid_argument";
        }
        *equals = '\0';
        for (size_t i = 0; i < argc; ++i) {
            if (!strcmp(keys[i], arg)) {
                return "invalid_argument";
            }
        }
        keys[argc] = arg;
        values[argc++] = equals + 1;
    }
    if (out->command <= LOGGER_CMD_CAPS) {
        return argc ? "invalid_argument" : NULL;
    }
    const char *bitrate = NULL, *ids = NULL, *filter = NULL, *duration = NULL;
    const char *sid = NULL, *seq = NULL, *reason = NULL;
    for (size_t i = 0; i < argc; ++i) {
        if (out->command == LOGGER_CMD_START && !strcmp(keys[i], "bitrate")) {
            bitrate = values[i];
        } else if (out->command == LOGGER_CMD_START && !strcmp(keys[i], "ids")) {
            ids = values[i];
        } else if (out->command == LOGGER_CMD_START && !strcmp(keys[i], "filter")) {
            filter = values[i];
        } else if (out->command == LOGGER_CMD_START && !strcmp(keys[i], "duration_ms")) {
            duration = values[i];
        } else if (out->command != LOGGER_CMD_START && !strcmp(keys[i], "sid")) {
            sid = values[i];
        } else if (out->command == LOGGER_CMD_ACK && !strcmp(keys[i], "seq")) {
            seq = values[i];
        } else if (out->command == LOGGER_CMD_STOP && !strcmp(keys[i], "reason")) {
            reason = values[i];
        } else {
            return "invalid_argument";
        }
    }
    if (out->command == LOGGER_CMD_START) {
        if (!bitrate || !ids || !filter) {
            return "invalid_argument";
        }
        if (!decimal(bitrate, UINT32_MAX, &number) || number < 1000 || number > 1000000) {
            return "invalid_bitrate";
        }
        out->config.bitrate = (uint32_t)number;
        if (!strcmp(ids, "11")) {
            out->config.ids = LOGGER_IDS_11;
        } else if (!strcmp(ids, "29")) {
            out->config.ids = LOGGER_IDS_29;
        } else if (!strcmp(ids, "both")) {
            out->config.ids = LOGGER_IDS_BOTH;
        } else {
            return "invalid_argument";
        }
        if (!parse_filter(filter, &out->config)) {
            return "invalid_argument";
        }
        if (duration) {
            if (!decimal(duration, INT64_MAX / 1000, &number) || !number) {
                return "invalid_argument";
            }
            out->config.duration_ms = number;
        }
        return NULL;
    }
    if (!session_id(sid)) {
        return "invalid_argument";
    }
    memcpy(out->sid, sid, sizeof out->sid);
    if (out->command == LOGGER_CMD_ACK) {
        if (!seq || !decimal(seq, UINT64_MAX, &out->ack)) {
            return "invalid_argument";
        }
    } else if (out->command == LOGGER_CMD_STOP) {
        if (reason && !strcmp(reason, "ctrl_c")) {
            out->reason = LOGGER_REASON_CTRL_C;
        } else if (reason && !strcmp(reason, "host_stop")) {
            out->reason = LOGGER_REASON_HOST_STOP;
        } else {
            return "invalid_argument";
        }
    }
    return NULL;
}
