#define CLOCK_SECOND 1000

struct timer {
    int dummy;
};

#ifdef __cplusplus
extern "C" {
#endif

void timer_nopsleep(int number_of_nops);
void timer_set(struct timer *timer, unsigned int seconds);
void timer_restart(struct timer *timer);
int timer_expired(struct timer *timer);

#ifdef __cplusplus
}
#endif
