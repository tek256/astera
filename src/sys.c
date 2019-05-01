#include "sys.h"

#include <GLFW/glfw3.h>

void sys_log(const char* format, ...){
    va_list va;
    va_start(va, format);
    vfprintf(stdout, format, va);
    va_end(va);
}

void sys_warn(const char* format, ...){
    va_list va;
    va_start(va, format);
    vfprintf(stdout, format, va);
    va_end(va);
}

void sys_err(const char* format, ...){
    va_list va;
    va_start(va, format);
    vfprintf(stderr, format, va);
    va_end(va);
}


time_s t_get_time(){
    return glfwGetTime() * MS_PER_SEC;
}

time_s t_get_time_since(time_s t){
    return t_get_time() - t;
}

time_s t_get_time_until(time_s t){
    return t - t_get_time();
}

time_s t_get_time_diff(time_s t){
    time_s c = t_get_time();
    if(c > t){
        return c - t;
    }
    return t - c;
}

bool t_is_time_before(time_s a, time_s b){
    return a < b;
}

bool t_is_time_after(time_s a, time_s b){
    return a > b;
}

bool t_cmp_timer_names(const char a[TIMER_NAME_LENGTH], const char b[TIMER_NAME_LENGTH]){
    for(int i=0;i<TIMER_NAME_LENGTH;++i){
        if(a[i] != b[i]){
            return false;
        }
    }
    return true;
}

void t_cpy_timer_names(char dst[TIMER_NAME_LENGTH], const char src[TIMER_NAME_LENGTH]){
    for(int i=0;i<TIMER_NAME_LENGTH;++i){
        dst[i] = src[i];
    }
}

timer_s* t_get_timer(const char name[TIMER_NAME_LENGTH]){
    if(t_timer_count == 0){
        t_cpy_timer_names(t_timers[t_timer_count].name, name);
        t_timers[t_timer_count].timestamp = t_get_time();
        t_timer_count++;

        return &t_timers[t_timer_count-1];
    }

    for(int i=0;i<t_timer_count;++i){
        if(t_cmp_timer_names(t_timers[i].name, name)){
            return &t_timers[i];
        }
    }

    if(t_timer_count == TIMER_MAX_COUNT){
        return NULL;
    }

    t_cpy_timer_names(t_timers[t_timer_count].name, name);
    t_timers[t_timer_count].timestamp = t_get_time();
    t_timer_count++;

    return &t_timers[t_timer_count-1];
}

void t_end_timer(const char name[TIMER_NAME_LENGTH]){
    int index = -1;

    for(int i=0;i<t_timer_count;++i){
        if(t_cmp_timer_names(t_timers[i].name, name)){
            index = i;
            break;
        }
    }

    if(index == -1){
        return;
    }

    for(int i=index;i<t_timer_count-1;++i){
        t_timers[i] = t_timers[i+1];
    }
    t_timer_count --;
}

void t_mark_timer(const char name[TIMER_NAME_LENGTH]){
    timer_s* t = t_get_timer(name);
    if(t){
        t->timestamp = t_get_time();
    }
}

void t_mark_ttimer(const char name[TIMER_NAME_LENGTH], time_s t){
    timer_s* timer = t_get_timer(name);
    if(t){
        timer->timestamp = t;
    }
}

time_s t_get_timer_since(const char name[TIMER_NAME_LENGTH]){
    timer_s* t = t_get_timer(name);
    if(t){
        return t_get_time() - t->timestamp;
    }
    return 0;
}

time_s t_get_timer_until(const char name[TIMER_NAME_LENGTH]){
    timer_s* t = t_get_timer(name);
    if(t){
        return t->timestamp - t_get_time();
    }
    return 0;
}

time_s t_get_timer_diff(const char name[TIMER_NAME_LENGTH]){
    timer_s* t = t_get_timer(name);

    if(t){
        time_s ts = t_get_timer(name)->timestamp;
        time_s c = t_get_time();

        if(ts > c){
            return ts - c;
        }
        return c - ts;

    }
    return 0;
}

void t_end_timert(timer_s* t){
    t_end_timer(t->name);
}

void t_mark_timert(timer_s* t){
    t->timestamp = t_get_time();
}

void t_mark_ttimert(timer_s* t, time_s timestamp){
    t->timestamp = timestamp;
}

time_s t_get_timert_since(timer_s* t){
    return t_get_time() - t->timestamp;
}

time_s t_get_timert_until(timer_s* t){
    return t->timestamp - t_get_time();
}

time_s t_get_timert_diff(timer_s* t){
    time_s ts = t->timestamp;
    time_s c  = t_get_time();

    if(ts > c){
        return ts - c;
    }
    return c - ts;
}

