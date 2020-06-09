/* Timer stubs
 *
 * This must be replaced by target timer routines.
 */

void timer_nopsleep(int number_of_nops) { }

void timer_set(struct timer *timer, unsigned int seconds) { }
void timer_restart(struct timer *timer) { }
int timer_expired(struct timer *timer) {
    /* Always expired, not waiting here */
    return 1;
}