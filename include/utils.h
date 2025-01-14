#ifndef UTILS_H
#define UTILS_H

#ifndef UNUSED
#define UNUSED __attribute__((unused))
#endif

#ifndef when_null
#define when_null(x, l) \
    if ((x) == NULL)    \
    {                   \
        goto l;         \
    }
#endif

#ifndef when_not_null
#define when_not_null(x, l) \
    if ((x) != NULL)        \
    {                       \
        goto l;             \
    }
#endif

#ifndef when_true
#define when_true(x, l) \
    if ((x))            \
    {                   \
        goto l;         \
    }
#endif

#ifndef when_false
#define when_false(x, l) \
    if (!(x))            \
    {                    \
        goto l;          \
    }
#endif

#ifndef when_failed
#define when_failed(x, l) \
    if ((x) != 0)         \
    {                     \
        goto l;           \
    }
#endif

#ifndef when_str_empty
#define when_str_empty(x, l)      \
    if ((x) == NULL || *(x) == 0) \
    {                             \
        goto l;                   \
    }
#endif

#define ARRAY_LEN(x) ((int)(sizeof(x) / sizeof((x)[0])))

#endif // UTILS_H