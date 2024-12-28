#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ENV_VAR_LENGTH 256

// Function to fetch an environment variable from a .env file
char* get_env_variable(const char* env_file, const char* key) {
    static char value[MAX_ENV_VAR_LENGTH];
    FILE *file = fopen(env_file, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open %s\n", env_file);
        return NULL;
    }

    char line[MAX_ENV_VAR_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        char *equals = strchr(line, '=');
        if (equals) {
            *equals = '\0';  // Split key and value
            if (strcmp(line, key) == 0) {
                strncpy(value, equals + 1, MAX_ENV_VAR_LENGTH);
                value[strcspn(value, "\r\n")] = '\0'; // Remove newline
                fclose(file);
                return value;
            }
        }
    }
    fclose(file);
    fprintf(stderr, "Error: Key '%s' not found in %s\n", key, env_file);
    return NULL;
}
