#include "moving_average.h"

float update_moving_average(SensorState *s, float new_value)
{
    if (s->count < WINDOW_SIZE) {
        s->window[s->index] = new_value;
        s->sum += new_value;
        s->count++;
    } else {
        s->sum -= s->window[s->index];
        s->window[s->index] = new_value;
        s->sum += new_value;
    }

    s->index = (s->index + 1) % WINDOW_SIZE;

    return s->sum / s->count;
}