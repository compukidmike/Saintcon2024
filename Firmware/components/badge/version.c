#include <stdio.h>
#include <string.h>
#include "version.h"

bool version_parse(const char *version_str, version_t *version) {
    memset(version, 0, sizeof(version_t));

    // Parse with git hash if present
    if (sscanf(version_str, "%hhu.%hhu.%hhu+%8s", &version->major, &version->minor, &version->patch, version->git_hash) == 4) {
        if (strcasecmp(version->git_hash, "unknown") == 0) {
            version->git_hash[0] = '\0';
        }
        return true;
    }

    // Otherwise parse without git hash
    else {
        return sscanf(version_str, "%hhu.%hhu.%hhu", &version->major, &version->minor, &version->patch) == 3;
    }
}

int version_compare(version_t *a, version_t *b) {
    // Compare major version components
    if (a->major < b->major) {
        return -1;
    } else if (a->major > b->major) {
        return 1;
    }

    // Compare minor version components
    if (a->minor < b->minor) {
        return -1;
    } else if (a->minor > b->minor) {
        return 1;
    }

    // Compare patch version components
    if (a->patch < b->patch) {
        return -1;
    } else if (a->patch > b->patch) {
        return 1;
    }

    // Versions are equal
    return 0;
}

version_t firmware_version() {
    version_t version;
    version.major = FIRMWARE_VERSION_MAJOR;
    version.minor = FIRMWARE_VERSION_MINOR;
    version.patch = FIRMWARE_VERSION_PATCH;
    memset(version.git_hash, 0, sizeof(version.git_hash));
    snprintf(version.git_hash, sizeof(version.git_hash), "%s", FIRMWARE_VERSION_GIT_HASH);
    return version;
}

const char *firmware_version_str() {
    return FIRMWARE_VERSION_STRING;
}

int firmware_version_compare(const char *version_str) {
    version_t current_version = firmware_version();
    version_t compare_version;
    if (!version_parse(version_str, &compare_version)) {
        return -1;
    }
    return version_compare(&current_version, &compare_version);
}