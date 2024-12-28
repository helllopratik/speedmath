#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <curl/curl.h>
#include <gtk/gtk.h>

// Gemini API key
#define API_KEY " "

// Function to send data to Gemini API
char* get_gemini_response(const char* question) {
    CURL *curl;
    CURLcode res;
    char *readBuffer;
    size_t size = 0;

    curl = curl_easy_init();
    if(curl) {
        char url[256];
        snprintf(url, sizeof(url), "https://api.gemini.com/v1/chat?api_key=%s&prompt=%s", API_KEY, question);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL); // Ignore output to stdout
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return NULL;
        }

        res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_getinfo() failed\n");
            return NULL;
        }

        readBuffer = malloc(size + 1);
        if(!readBuffer) {
            fprintf(stderr, "malloc() failed\n");
            return NULL;
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL); // Ignore output to stdout
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n");
            return NULL;
        }

        curl_easy_cleanup(curl);
        return readBuffer;
    }

    return NULL;
}

// Function to create the main window
GtkWidget* create_main_window() {
    GtkWidget *window, *grid, *question_label, *question_text, *answer_label, *answer_text, *result_label, *result_text, *timer_label, *average_time_label, *average_time_text, *dark_mode_button, *light_mode_button;

    // Create window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW_TOPLEVEL(window), "Speed Math with Indian Bank PYQs");
    gtk_window_set_default_size(GTK_WINDOW_TOPLEVEL(window), 400, 300);

    // Create grid layout
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Create labels and text boxes
    question_label = gtk_label_new("Question:");
    question_text = gtk_label_new("");
    answer_label = gtk_label_new("Your Answer:");
    answer_text = gtk_entry_new();
    result_label = gtk_label_new("Result:");
    result_text = gtk_label_new("");
    timer_label = gtk_label_new("Time:");
    average_time_label = gtk_label_new("Average Time:");
    average_time_text = gtk_label_new("");

    // Create dark mode and light mode buttons
    dark_mode_button = gtk_button_new_with_label("Dark Mode");
    light_mode_button = gtk_button_new_with_label("Light Mode");

    // Add widgets to grid
    gtk_grid_attach(GTK_GRID(grid), question_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), question_text, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), answer_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), answer_text, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), result_label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), result_text, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), timer_label, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), average_time_label, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), average_time_text, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), dark_mode_button, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), light_mode_button, 1, 5, 1, 1);

    // Connect signals
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(answer_text, "activate", G_CALLBACK(on_answer_submitted), question_text, answer_text, result_text, timer_label, average_time_text);
    g_signal_connect(dark_mode_button, "clicked", G_CALLBACK(on_dark_mode_clicked), window);
    g_signal_connect(light_mode_button, "clicked", G_CALLBACK(on_light_mode_clicked), window);

    return window;
}

// Function to handle answer submission
void on_answer_submitted(GtkWidget *widget, gpointer data) {
    GtkWidget *question_text = GTK_WIDGET(data);
    GtkWidget *answer_text = GTK_WIDGET(g_object_get_data(data, "answer_text"));
    GtkWidget *result_text = GTK_WIDGET(g_object_get_data(data, "result_text"));
    GtkWidget *timer_label = GTK_WIDGET(g_object_get_data(data, "timer_label"));
    GtkWidget *average_time_text = GTK_WIDGET(g_object_get_data(data, "average_time_text"));

    const char *question = gtk_label_get_text(GTK_LABEL(question_text));
    const char *user_answer = gtk_entry_get_text(GTK_ENTRY(answer_text));

    char *gemini_response = get_gemini_response(question);
    if (gemini_response) {
        // Process Gemini response (extract correct answer, steps, etc.)
        // ...

        // Set result in UI
        gtk_label_set_text(GTK_LABEL(result_text), gemini_response);

        // Start timer
        // ...

        // Calculate and update average time
        // ...

        free(gemini_response);
    } else {
        gtk_label_set_text(GTK_LABEL(result_text), "Failed to get response from Gemini.");
    }
}

// Function to handle dark mode click
void on_dark_mode_clicked(GtkWidget *button, gpointer data) {
    GtkWidget *window = GTK_WIDGET(data);
    gtk_style_context_add_class(gtk_widget_get_style_context(window), "dark");
}

// Function to handle light mode click
void on_light_mode_clicked(GtkWidget *button, gpointer data) {
    GtkWidget *window = GTK_WIDGET(data);
    gtk_style_context_remove_class(gtk_widget_get_style_context(window), "dark");
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = create_main_window();
    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
