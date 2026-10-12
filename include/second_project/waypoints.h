#pragma once
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <string>

struct Waypoint {
    double x, y, theta;
};

// Parses one "x,y,theta" CSV line. Returns false for blank, comment, header or malformed lines.
inline bool parseWaypoint(const std::string& line, Waypoint& out) {
    std::istringstream ss(line);
    std::string token;
    double v[3];
    for (int i = 0; i < 3; i++) {
        if (!std::getline(ss, token, ',')) return false;
        const char* begin = token.c_str();
        char* end;
        errno = 0;
        v[i] = std::strtod(begin, &end);
        while (*end == ' ' || *end == '\t' || *end == '\r') end++;
        if (end == begin || *end != '\0' || errno == ERANGE || !std::isfinite(v[i])) return false;
    }
    if (std::getline(ss, token, ',')) return false;  // more than three columns
    out = {v[0], v[1], v[2]};
    return true;
}
