#ifndef TEMPLATES_H
#define TEMPLATES_H

#include "init.h"

// Get Dockerfile content for language
const char* get_dockerfile_template(LangType lang);

// Get .gitignore content for language
const char* get_gitignore_template(LangType lang);

// Get main source file content
const char* get_main_template(LangType lang, const char *project_name);

// Get test file content
const char* get_test_template(LangType lang);

// Get .forge.yaml content
const char* get_forge_yaml_template(const char *project_name, LangType lang);

// Get README content
const char* get_readme_template(const char *project_name, LangType lang);

// Get .env.example content
const char* get_env_example_template(LangType lang);

// Get CI template
const char* get_ci_template(CIType ci, LangType lang);

#endif