/*
 * Copyright (c) 2002-2019 "Neo4j,"
 * Neo4j Sweden AB [http://neo4j.com]
 *
 * This file is part of Neo4j.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "bolt-private.h"
#include "time.h"

int64_t BoltTime_get_time_ms_from(struct timespec* tp)
{
    return (tp->tv_sec)*1000+(tp->tv_nsec)/1000000;
}

int64_t BoltTime_get_time_ms()
{
    struct timespec now;
    BoltTime_get_time(&now);
    return BoltTime_get_time_ms_from(&now);
}

void BoltTime_diff_time(struct timespec* t, struct timespec* t0, struct timespec* t1)
{
    t->tv_sec = t0->tv_sec-t1->tv_sec;
    t->tv_nsec = t0->tv_nsec-t1->tv_nsec;
    while (t->tv_nsec>=1000000000) {
        t->tv_sec += 1;
        t->tv_nsec -= 1000000000;
    }
    while (t->tv_nsec<0) {
        t->tv_sec -= 1;
        t->tv_nsec += 1000000000;
    }
}
