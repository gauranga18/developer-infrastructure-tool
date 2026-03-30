#ifndef INIT_PROJECT
#define INIT_PROJECT

typedef enum {
    LANG_PYTHON,
    LANG_NODE,
    LANG_GO,
    LANG_RUST,
    LANG_C,
    LANG_CUSTOM
} LangType;

typedef enum {
    CI_NONE,
    CI_GITHUB,
    CI_GITLAB
} CIType;

typedef struct {
    const char *project_name;
    LangType language;
    CIType ci;
    int interactive;
} InitOptions;

int init_project(const InitOptions *opts);

#endif