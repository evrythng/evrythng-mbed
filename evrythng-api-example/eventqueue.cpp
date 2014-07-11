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
#include "mbed.h"
#include "eventqueue.h"
#include <time.h>

#include "util.h"

void EventQueue::put(int64_t time, EventType type)
{
    Event event;
    event.time = time;
    event.type = type;
    queue.push(event);
}

EventType EventQueue::waitNext(int64_t currentTime)
{
    Event event = queue.top();
    queue.pop();
    int64_t w = event.time - currentTime;
    if (w > 0) wait_ms(w);
    return event.type;
}

bool EventQueue::empty()
{
    return queue.empty();
}

int EventQueue::size()
{
    return queue.size();
}