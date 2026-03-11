#ifndef HELPERS_H
#define HELPERS_H

#define ALLOC_FAILURE(msg)                                                     \
  do {                                                                         \
    fprintf(stderr, "%s:%d:[ERROR]: Allocation error '%s'\n", __FILE__,        \
            __LINE__, (msg));                                                  \
    exit(EXIT_FAILURE);                                                        \
  } while (0)
#define BAD_ARG(msg)                                                           \
  do {                                                                         \
    fprintf(stderr, "%s:%d:[ERROR]: Bad argument '%s'\n", __FILE__, __LINE__,  \
            (msg));                                                            \
    exit(EXIT_FAILURE);                                                        \
  } while (0)
#define TODO(msg)                                                              \
  do {                                                                         \
    fprintf(stderr, "%s:%d:[INFO]: TODO '%s'\n", __FILE__, __LINE__, (msg));   \
    exit(EXIT_SUCCESS);                                                        \
  } while (0)
#define UNREACHABLE(msg)                                                       \
  do {                                                                         \
    fprintf(stderr, "%s:%d:[ERROR]: Unreachable '%s'\n", __FILE__, __LINE__,   \
            (msg));                                                            \
    exit(EXIT_FAILURE);                                                        \
  } while (0)
#define UNUSED(x) (void)(x)

extern int helpers_ishexdigit(char ch);
extern int helpers_isdigit(char ch);
extern int helpers_isspace(char ch);

#endif /* HELPERS_H */
