#include <grstaps/Timer.h>

std::vector< std::pair<double, const char*> > Timer::_timer_splits;

/**
 * Timer class constructor
 */
Timer::Timer() {
    this->running = false;
    this->elapsed_time = 0;

}


/**
 * Default Timer destructor
 */
Timer::~Timer() { }


/**
 * Starts the Timer - similar to starting a stopwatch
 */
void Timer::start() {

    if (!this->running) {
#ifdef __MACH__ 	/* For OSX */
        gettimeofday(&this->start_time, NULL);
#else				/* For linux */
        clock_gettime(CLOCK_MONOTONIC, &this->start_time);
#endif
        this->running = true;
    }
    return;
}


/**
 * Stops the timer - similar to stopping a stopwatch
 */
void Timer::stop() {
    if (this->running) {
#ifdef __MACH__ /* For OSX */
        gettimeofday(&this->end_time, NULL);
#else			/* For linux */
        clock_gettime(CLOCK_MONOTONIC, &this->end_time);
#endif
        this->running = false;
        this->elapsed_time += this->diff(this->start_time, this->end_time);
    }
    return;
}


/**
 * Resets the timer - similar to resetting a stopwatch.
 */
void Timer::reset() {
    this->elapsed_time = 0;
    this->running = false;
}


/**
 * Restarts the timer. The elapsed time will accumulate along doublewith the
 * previous time(s) the timer was running. If the timer was already running
 * this function does nothing
 */
void Timer::restart() {
    if (!this->running) {
        this->elapsed_time += this->diff(this->start_time, this->end_time);
        this->start();
    }
}


/**
 * Records a message corresponding to a given time recorded by the timer.
 * When this method is called it assumes that the timer has been stopped
 * and has the current time for the process corresponding to the message
 * @param msg a msg corresponding to this time split
 */
void Timer::recordSplit(const char* msg) {
    _timer_splits.push_back(std::pair<double, const char*>(getTime(), msg));
}



/**
 * Returns the amount of time elapsed from start to stop of the timer. If the
 * timer is currently runnning, returns the time from the timer start to the present
 * time.
 * @return the elapsed time
 */
double Timer::getTime() {
    /* If the timer is not running */
    if (!this->running) {
#ifdef __MACH__		/* For OSX */
        return this->elapsed_time * 1.0E-6;
#else				/* For Linux */
        return this->elapsed_time * 1.0E-9;
#endif
    }

        /* If the timer is currently running */
    else {
        timespec temp;
#ifdef __MACH__ 	/* For OSX */
        gettimeofday(&temp, NULL);
#else				/* For Linux */
        clock_gettime(CLOCK_MONOTONIC, &temp);
#endif

        this->elapsed_time += this->diff(this->start_time, temp);

#ifdef __MACH__		/* For OSX */
        return this->elapsed_time * 1.0E-6;
#else				/* For Linux */
        return this->elapsed_time * 1.0E-9;
#endif
    }
}


/**
 * Helper function which computes the time between the values of
 * two timespec structs.
 * @param start timespec representing the start time
 * @param end timespec representing the end time
 */
double Timer::diff(timespec start, timespec end) {
    double sec, delta;
#ifdef __MACH__
    double usec;
		delta = end.tv_usec - start.tv_usec;
#else
    double nsec;
    delta = end.tv_nsec - start.tv_nsec;
#endif

    if (delta < 0) {
        sec = end.tv_sec - start.tv_sec;
#ifdef __MACH__
        usec = 1.0E6 + delta;
#else
        nsec = 1.0E9 + delta;
#endif

    } else {
        sec = end.tv_sec - start.tv_sec;
#ifdef __MACH__
        usec = delta;
#else
        nsec = delta;
#endif
    }

#ifdef __MACH__
    return (sec * 1.0E6 + usec);
#else
    return(sec*1.0E9 + nsec);
#endif
}


/**
 * This method will loop through all of the Timer's splits and print a
 * formatted message string (80 characters in length) to the console
 * with the message and the time corresponding to that message
 */
void Timer::printSplits() {

    const char* curr_msg;
    double curr_split;
    int msg_length, num_whitespaces;

    for (int i=0; i < (int)_timer_splits.size(); i++) {
        std::stringstream formatted_msg;

        curr_split = _timer_splits.at(i).first;
        curr_msg = _timer_splits.at(i).second;
        msg_length = strlen(curr_msg);

        /* Num whitespaces for formatting is:
         * 80 max char - 13 char for logger - 13 for time - msg length */
        num_whitespaces = 80 - 13 - 11 - msg_length -3;

        formatted_msg << curr_msg;

        /* Add periods to format message to 80 characters length */
        for (int i=0; i < num_whitespaces; i++)
            formatted_msg << ".";

        printf("%s%.7f sec \n", formatted_msg.str().c_str(), curr_split);
    }
}


/**
 * This method will loop through all of the Timer's splits and print a
 * formatted message string (80 characters in length) to the console
 * with the message and the time corresponding to that message
 */
void Timer::calcSplits() {

    const char* curr_msg;
    double curr_split;
    int msg_length, num_whitespaces;

    schedTime = 0;
    taTime = 0;
    planTime = 0;
    mpTime = 0;

    for (int i=0; i < (int)_timer_splits.size(); i++) {
        std::stringstream formatted_msg;

        curr_split = _timer_splits.at(i).first;
        curr_msg = _timer_splits.at(i).second;
        if(strcmp(curr_msg,"TA") == 0){
            taTime += curr_split;
        }
        else if(strcmp(curr_msg,"SCHED") == 0){
            schedTime += curr_split;
        }
        else if(strcmp(curr_msg,"PLAN") == 0){
            planTime += curr_split;
        }
        else if(strcmp(curr_msg,"MP") == 0){
            mpTime += curr_split;
        }
    }

    planTime = planTime - taTime;
    taTime = taTime - schedTime;
    schedTime = schedTime - mpTime;

}
