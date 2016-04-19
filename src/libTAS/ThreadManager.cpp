/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.

    Author : Philippe Virouleau <philippe.virouleau@imag.fr>
 */
#include <sstream>
#include <utility>
#include <csignal>
#include "ThreadManager.h"

using namespace std;


ThreadManager ThreadManager::instance_ = ThreadManager();
atomic<int> ThreadManager::spin(0);

ThreadManager::ThreadManager()
{
    //FIXME some dirty hardcoded values for SMB
    //refTable_.insert(71616);
    //refTable_.insert(222832);
    //refTable_.insert(282080);
    //refTable_.insert(140008597117578);
    //FIXME some dirty hardcoded values for FEZ
    refTable_.insert(-557);

    // Registering a sighandler enable us to suspend the main thread from any thread !
    struct sigaction sigusr1;
    sigemptyset(&sigusr1.sa_mask);
    sigusr1.sa_flags = 0;
    sigusr1.sa_handler = ThreadManager::sigspin;
    int status = sigaction(SIGUSR1, &sigusr1, nullptr);
    if (status == -1)
        perror("Error installing signal");
}


void ThreadManager::sigspin(int sig)
{
    debuglog(LCF_THREAD, "Waiting, sig = ", sig);
    while (spin)
        ;
}

void ThreadManager::init(pthread_t tid)
{
    init_ = true;
    main_ = tid;
}

void ThreadManager::suspend(pthread_t from_tid)
{
    // We want to suspend main if:
    //  - from_tid is main (which means it asks for it)
    //  - from_tid is one of the registered threads we want to wait for
    if (from_tid == main_ || waitFor(from_tid)) {
        if (init_) {
            debuglog(LCF_THREAD, "Suspending main (", stringify(main_), ") because of ", stringify(from_tid));
            spin = 1;
            // This doesn't actually kill the thread, it send SIGUSR1 to the main
            // thread, which make it spins until resume
            pthread_kill(main_, SIGUSR1);
        }
    } else {
        debuglog(LCF_THREAD, "Not suspending because of ", stringify(from_tid));
    }
}

void ThreadManager::start(pthread_t tid, void *from, void *start_routine)
{
    if (!init_) {
        beforeSDL_.insert(start_routine);
        return;
    }
    ptrdiff_t diff = (char *)start_routine - (char *)from;
    set<pthread_t> &all = threadMap_[diff];
    all.insert(tid);
    // Register the current thread id -> routine association
    // The same tid can be reused with a different routine, but different routines
    // cannot be running at the same time with the same tid.
    currentAssociation_[tid] = diff;
    debuglog(LCF_THREAD, "Register starting ", stringify(tid)," with entrydiff ",  diff, ".");
    timespec t;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t);
    //There may be multiple call to start...
    startTime_[tid].push_back(t);
}

void ThreadManager::end(pthread_t tid)
{
    debuglog(LCF_THREAD, "Register ending ", stringify(tid), ".");
    timespec t;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t);
    //There may be multiple call to end...
    endTime_[tid].push_back(t);
}

void ThreadManager::resume()
{
    if (spin) {
        spin = 0;
        debuglog(LCF_THREAD, "Released main.");
    }
}

timespec timediff(timespec &start, timespec &end);
//FIXME pretty this
timespec timediff(timespec &start, timespec &end)
{
    timespec retVal;
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        retVal.tv_sec = end.tv_sec - start.tv_sec - 1;
        retVal.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    } else {
        retVal.tv_sec = end.tv_sec - start.tv_sec;
        retVal.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return retVal;
}

bool ThreadManager::waitFor(pthread_t tid)
{
    // Lookup the entry point corresponding to this tid, and check if it's
    // in the reference table
    ptrdiff_t diff = currentAssociation_[tid];
    return init_ && refTable_.count(diff);
}

string ThreadManager::summary()
{
    ostringstream oss;
    for (auto elem : threadMap_) {
        oss << "\nRecord for entry point : " << elem.first;
        set<pthread_t> &allThread = elem.second;
        for (pthread_t t : allThread) {
            oss << "\n  - " << stringify(t);
            //FIXME using find would permit to add the const qualifier to this member
            vector<timespec> &starts = startTime_[t];
            if (starts.empty())
                continue;
            vector<timespec> &ends = endTime_[t];
            for (unsigned int i = 0; i < starts.size(); i++) {
                oss << "\n    1: Started";
                timespec start = starts[i];
                if (i < ends.size()) {
                    timespec end = ends[i];
                    timespec diff = timediff(start, end);
                    oss << " and lasted " << diff.tv_sec << " seconds and " << diff.tv_nsec << " nsec.";
                }
            }
        }
    }
    oss << "\nThese threads started before SDL init and can't be waited for :\n";
    for (auto elem : beforeSDL_) {
        oss <<  elem << "\n";
    }
    return oss.str();
}
