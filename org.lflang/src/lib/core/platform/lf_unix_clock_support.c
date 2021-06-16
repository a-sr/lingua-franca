
#include <time.h>

/**
 * Offset to _LF_CLOCK that would convert it
 * to epoch time.
 * For CLOCK_REALTIME, this offset is always zero.
 * For CLOCK_MONOTONIC, it is the difference between those
 * clocks at the start of the execution.
 */
interval_t _lf_epoch_offset = 0LL;

/**
 * Convert a _lf_time_spec_t ('tp') to an instant_t representation in
 * nanoseconds.
 *
 * @return nanoseconds (long long).
 */
instant_t convert_timespec_to_ns(_lf_time_spec_t tp) {
    return tp.tv_sec * 1000000000 + tp.tv_nsec;
}

/**
 * Convert an instant_t ('t') representation in nanoseconds to a
 * _lf_time_spec_t.
 *
 * @return _lf_time_spec_t representation of 't'.
 */
_lf_time_spec_t convert_ns_to_timespec(instant_t t) {
    _lf_time_spec_t tp;
    tp.tv_sec = t / 1000000000;
    tp.tv_nsec = (t % 1000000000);
    return tp;
}

/**
 * Calculate the necessary offset to bring _LF_CLOCK in parity with the epoch
 * time reported by CLOCK_REALTIME.
 */
void calculate_epoch_offset() {
    if (_LF_CLOCK == CLOCK_REALTIME) {
        // Set the epoch offset to zero (see tag.h)
        _lf_epoch_offset = 0LL;
    } else {
        // Initialize _lf_epoch_offset to the difference between what is
        // reported by whatever clock LF is using (e.g. CLOCK_MONOTONIC) and
        // what is reported by CLOCK_REALTIME.
        struct timespec physical_clock_snapshot, real_time_start;

        clock_gettime(_LF_CLOCK, &physical_clock_snapshot);
        long long physical_clock_snapshot_ns = convert_timespec_to_ns(physical_clock_snapshot);


        clock_gettime(CLOCK_REALTIME, &real_time_start);
        long long real_time_start_ns = convert_timespec_to_ns(real_time_start);

        _lf_epoch_offset = real_time_start_ns - physical_clock_snapshot_ns;
    }
}

/**
 * Initialize the LF clock.
 */
void lf_initialize_clock() {
    calculate_epoch_offset();
}

/**
 * Fetch the value of _LF_CLOCK (see lf_linux_support.h) and store it in tp. The
 * timestamp value in 'tp' will always be epoch time, which is the number of
 * nanoseconds since January 1st, 1970.
 *
 * @return 0 for success, or -1 for failure. In case of failure, errno will be
 *  set appropriately (see `man 2 clock_gettime`).
 */
int lf_clock_gettime(_lf_time_spec_t* tp) {
    // Adjust the clock by the epoch offset, so epoch time is always reported.
    int return_value = clock_gettime(_LF_CLOCK, (struct timespec*) tp);
    if (return_value < 0) {
        return return_value;
    }

    // We need to apply the epoch offset if it is not zero
    if (_lf_epoch_offset != 0) {
        // Here, we do a costly conversion from _lf_time_spec_t to nanoseconds,
        // add the epoch offset, and then convert back to _lf_time_spec_t. The
        // reason is simply to account for overflows from tv_nsec to tv_sec when
        // applying the offset.
        instant_t tp_in_ns = convert_timespec_to_ns(*tp);
        tp_in_ns += _lf_epoch_offset;
        *tp = convert_ns_to_timespec(tp_in_ns);
    }
    return return_value;
}