#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <ctype.h> // For isdigit and ispunct
#include <unistd.h> // For sleep
#include "../env_loader.h"

// Global Variables
char api_key[256];
char response_buffer[8192];
GtkWidget *response_label;
GtkWidget *entry;
GtkWidget *status_label;
GtkWidget *window;

// Function Prototypes
void load_api_key();
void send_query(const char *query);
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp);
void handle_user_query(GtkWidget *widget, gpointer data);
void update_status(const char *status);

// Default Query
const char *default_query = "give me simplification, approximation and speed math question for preparation practice for Indian banking exam. You will give pyq question one question at a time and we will give the answer (give option also and do mention that the option could be given wrong). After user sends an answer to you, you will give stepwise complete answer. And then mention a note: for next question press 1 or any numeric or special character. If user did, then show them the next question. One question at a time";

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Main Window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Gemini Speed Math Practice");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Main Layout
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Status Label
    status_label = gtk_label_new("App Loading...");
    gtk_grid_attach(GTK_GRID(grid), status_label, 0, 0, 1, 1);

    // Response Label
    response_label = gtk_label_new("Waiting for response...");
    gtk_label_set_line_wrap(GTK_LABEL(response_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(response_label), 0);
    gtk_grid_attach(GTK_GRID(grid), response_label, 0, 1, 1, 1);

    // User Input Entry
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter your answer (numeric/special characters only)");
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 2, 1, 1);

    // Submit Button
    GtkWidget *submit_button = gtk_button_new_with_label("Submit");
    gtk_grid_attach(GTK_GRID(grid), submit_button, 0, 3, 1, 1);
    g_signal_connect(submit_button, "clicked", G_CALLBACK(handle_user_query), NULL);

    gtk_widget_show_all(window);

    // Update Status Step-by-Step
    update_status("App Working...");
    sleep(4);

    load_api_key();
    update_status("File Checked...");
    sleep(4);

    update_status("Connecting to Network...");
    sleep(4);

    send_query(default_query);

    gtk_main();
    return 0;
}

// Update Status
void update_status(const char *status) {
    gtk_label_set_text(GTK_LABEL(status_label), status);
    while (gtk_events_pending()) gtk_main_iteration();
}

// Load API Key from .env
void load_api_key() {
    char env_file[256];
    snprintf(env_file, sizeof(env_file), "%s/Desktop/speedmath/.env", getenv("HOME"));
    const char *key = get_env_variable(env_file, "key");
    if (!key) {
        update_status("Failed: API Key Missing");
        fprintf(stderr, "Failed to load API Key.\n");
        exit(1);
    }
    strncpy(api_key, key, sizeof(api_key));
    printf("API Key Loaded: %s\n", api_key);  // Optional: Debug print to ensure it's loaded
}

// Send Query to Gemini API
void send_query(const char *query) {
    update_status("Sending Request...");

    CURL *curl = curl_easy_init();
    if (!curl) {
        update_status("Failed: CURL Initialization");
        fprintf(stderr, "Failed to initialize CURL.\n");
        return;
    }

    // Correct URL for Gemini API
    char url[512];
    snprintf(url, sizeof(url), "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=%s", api_key);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Increased size for authorization header buffer to avoid truncation
    char authorization_header[512];  // Make this large enough for the full "Bearer <api_key>" string
    snprintf(authorization_header, sizeof(authorization_header), "Authorization: Bearer %s", api_key);
    headers = curl_slist_append(headers, authorization_header);

    char post_data[1024];
    snprintf(post_data, sizeof(post_data), "{\"contents\": [{\"parts\": [{\"text\": \"%s\"}]}]}", query);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_buffer);

    // Enable verbose mode for more details
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        update_status("Failed: Request Error");
        fprintf(stderr, "CURL error: %s\n", curl_easy_strerror(res));
    } else {
        update_status("Response Achieved...");
        gtk_label_set_text(GTK_LABEL(response_label), response_buffer);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

// Write Callback for CURL
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    strncat(userp, contents, total_size);
    return total_size;
}

// Handle User Query
void handle_user_query(GtkWidget *widget, gpointer data) {
    const char *user_input = gtk_entry_get_text(GTK_ENTRY(entry));

    // Validate user input
    for (const char *c = user_input; *c; c++) {
        if (!(isdigit(*c) || ispunct(*c))) {
            gtk_label_set_text(GTK_LABEL(response_label), "Invalid input. Only numeric and special characters are allowed.");
            return;
        }
    }

    // Send query to Gemini
    send_query(user_input);

    // Clear the entry box for new input
    gtk_entry_set_text(GTK_ENTRY(entry), "");
}
