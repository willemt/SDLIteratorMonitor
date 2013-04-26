void* SDL_IterMon_init();

void SDL_IterMon_set_iter(
        void* monitor,
        void* iter,
        void* (*iter_get_next)(void*),
        int (*iter_has_next)(void*),
        void (*iter_done)(void*),
        void (*do_meanwhile)(void));

void* SDL_IterMon_get_next(void* monitor);

