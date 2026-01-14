#ifdef RT_THREAD_PLATFORM
/* Some toolchains expect _fini; provide a weak stub to satisfy linker. */
void _fini(void) {}
#endif
