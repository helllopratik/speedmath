#include <stdio.h>
#include <stdlib.h> // Include for getenv
#include "env_loader.h"

int main() {
    // Dynamically construct the path to the .env file
    char env_file[256];
    snprintf(env_file, sizeof(env_file), "%s.env", getenv("HOME")); // Adjust path for non-Linux systems

    // Fetch the API key
    const char* api_key = get_env_variable(env_file, "GEMINI_API_KEY");
    if (api_key) {
        printf("Loaded API Key: %s\n", api_key);
    } else {
        fprintf(stderr, "Failed to load API Key.\n");
        return 1;  // Exit with error
    }

    // Use the API key in your application
    printf("Connecting to Gemini API with the key...\n");

    // Additional application logic would go here

    return 0;
}

