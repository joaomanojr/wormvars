/* add some microcontroller timer functions to get rid of initial build problems */

#define CLOCK_SECOND 1000

static void timer_nopsleep(int number_of_nops) { }

static void timer_set(struct timer *timer, unsigned int seconds) { }
static void timer_restart(struct timer *timer) { }

static int timer_expired(struct timer *timer) {
    return 0;
}