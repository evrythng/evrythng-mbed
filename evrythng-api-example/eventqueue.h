/*
 * (c) Copyright 2012 EVRYTHNG Ltd London / Zurich
 * www.evrythng.com
 *
 * --- DISCLAIMER ---
 *
 * EVRYTHNG provides this source code "as is" and without warranty of any kind,
 * and hereby disclaims all express or implied warranties, including without
 * limitation warranties of merchantability, fitness for a particular purpose,
 * performance, accuracy, reliability, and non-infringement.
 *
 * Author: Michel Yerly
 *
 */
#include <queue>
#include <stdint.h>

enum EventType {
    EVT_SET_PROP,
    EVT_GET_PROP
};

struct Event {
    int64_t time;
    EventType type;
};

class EventComparison
{
public:
    bool operator() (const Event& lhs, const Event&rhs) const {
        return (lhs.time>rhs.time);
    }
};

class EventQueue
{
public:
    void put(int64_t time, EventType type);
    EventType waitNext(int64_t currentTime);
    bool empty();
    int size();
private:
    std::priority_queue<Event, std::vector<Event>, EventComparison> queue;
};