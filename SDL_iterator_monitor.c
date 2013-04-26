
#include "SDL/SDL.h"
#include "SDL/SDL_thread.h"


/*----------------------------------------------------------------------------*/
/**
 * A monitor that we use to iterate through an iterator
 *
 *
 * */

typedef struct {
    /* next item on iterator */
    void* (*iter_get_next)(void*);
    /* has next item on iterator? */
    int (*iter_has_next)(void*);
    /* we're finished with the iterator */
    void (*iter_done)(void*);
    /* user data we use for iterating */
    void* iter_udata;

    void* cond_can_get_iter;
    void* sem_iterating;
    void* mutex;
} SDL_IterMon_t;

void* SDL_IterMon_init()
{
    SDL_IterMon_t* mon;

    mon = calloc(1,sizeof(SDL_IterMon_t));
    mon->mutex = SDL_CreateMutex();
    mon->cond_can_get_iter = SDL_CreateCond();
    mon->sem_iterating = SDL_CreateSemaphore(0);
    return mon;
}

/**
 * Only return after the iterator is finished
 * @param iter user data for iterator
 * @param iter_get_next the get next callback
 * @param iter_has_next the has next callback 
 * @param iter_done the done callback 
 * @param do_meanwhile function to run when no work needs to be done */
void SDL_IterMon_set_iter(
        void* monitor,
        void* iter,
        void* (*iter_get_next)(void*),
        int (*iter_has_next)(void*),
        void (*iter_done)(void*),
        void (*do_meanwhile)(void))
{
    SDL_IterMon_t* mon;
    
    mon = monitor;

    if (-1 == SDL_mutexP(mon->mutex)) assert(FALSE);

    // FIXME_NEEDS_WORK

    assert(!mon->iter);
    mon->iter = iter;
    mon->iter_get_next = iter_get_next;
    mon->iter_has_next = iter_has_next;
    mon->iter_done_next = iter_done_next;
    SDL_CondSignal(mon->cond_can_get_iter);

    /* do some other work while we're waiting */
    if (do_meanwhile)
        do_meanwhile();

    if (-1 == SDL_mutexV(mon->mutex)) assert(FALSE);

    while (mon->iter)
        SDL_SemWait(mon->sem_iterating);
}

/**
 * Get next item from the monitor 
 * @return the next item */
void* SDL_IterMon_get_next(void* monitor)
{
    SDL_IterMon_t* mon;
    void* next;

    mon = monitor;

    if (-1 == SDL_mutexP(mon->mutex)) assert(FALSE);

    /* wait for the iterator to be set */
    while (!mon->iter)
        if (-1 == SDL_CondWait(
                    mon->cond_can_get_iter,
                    mon->mutex)) assert(FALSE);

    assert(mon->iter);

    if (mon->iter_has_next(mon->iter))
    {
        next = mon->iter_get_next(mon->iter);
        assert(next);
    }
    else
    {
        mon->iter_done(mon->iter);
        mon->iter = NULL;
        SDL_SemPost(mon->sem_iterating);
        next = NULL;
    }

    if (-1 == SDL_mutexV(mon->mutex)) assert(FALSE);

    return next;
}

