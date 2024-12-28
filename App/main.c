#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "../env_loader.h"

// Global Variables
char api_key[256];
int current_question = 0;
double total_time = 0;
int answered_questions = 0;
double question_times[100]; // Array to store time for each question

// Questions (Mock Data for Now)
typedef struct {
    char question[256];
    char year[10];
} Question;

Question questions[] = {
    {"What is 25 + 35?", "2022"},
    {"What is 50 x 2?", "2021"}
};
int total_questions = 2;

// Function prototypes
static void submit_answer(GtkWidget *widget, gpointer entry);
static void next_question(GtkWidget *widget, gpointer data);
static gboolean update_timer(gpointer label);
void load_api_key();

// Timer variables
gboolean running = FALSE;
guint timer_id = 0;
gdouble start_time;

// Initialize GTK Application
int main(int argc, char *argv[]) {
    load_api_key();

    gtk_init(&argc, &argv);

    // Main Window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Speed Math App");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Main Layout
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Question Label
    GtkWidget *question_label = gtk_label_new(questions[current_question].question);
    gtk_grid_attach(GTK_GRID(grid), question_label, 0, 0, 1, 1);

    // Year Label
    GtkWidget *year_label = gtk_label_new(questions[current_question].year);
    gtk_grid_attach(GTK_GRID(grid), year_label, 0, 1, 1, 1);

    // Answer Entry
    GtkWidget *entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 2, 1, 1);

    // Timer Label (to display the timer)
    GtkWidget *timer_label = gtk_label_new("00:00:00");
    gtk_grid_attach(GTK_GRID(grid), timer_label, 0, 3, 1, 1);

    // Average Time Label
    GtkWidget *avg_time_label = gtk_label_new("Average Time: 00:00:00");
    gtk_grid_attach(GTK_GRID(grid), avg_time_label, 0, 4, 3, 1);

    // Submit Button
    GtkWidget *submit_button = gtk_button_new_with_label("Submit");
    gtk_grid_attach(GTK_GRID(grid), submit_button, 0, 5, 1, 1);
    g_signal_connect(submit_button, "clicked", G_CALLBACK(submit_answer), entry);

    // Next Question Button
    GtkWidget *next_button = gtk_button_new_with_label("Next Question");
    gtk_grid_attach(GTK_GRID(grid), next_button, 1, 5, 1, 1);
    g_signal_connect(next_button, "clicked", G_CALLBACK(next_question), NULL);

    // Show all widgets
    gtk_widget_show_all(window);

    // Start Timer
    running = TRUE;
    start_time = g_get_monotonic_time() / 1000.0;

    gtk_main();
    return 0;
}

// Load API Key from .env
void load_api_key() {
    char env_file[256];
    snprintf(env_file, sizeof(env_file), "%s/Desktop/.env", getenv("HOME"));
    const char *key = get_env_variable(env_file, "GEMINI_API_KEY");
    if (!key) {
        fprintf(stderr, "Failed to load API Key.\n");
        exit(1);
    }
    strncpy(api_key, key, sizeof(api_key));
}

// Submit Answer Handler
static void submit_answer(GtkWidget *widget, gpointer entry) {
    const char *user_answer = gtk_entry_get_text(GTK_ENTRY(entry));
    printf("User Answer: %s\n", user_answer); // Simulate API interaction

    // Stop Timer and calculate time taken for the current question
    if (running) {
        double elapsed = g_get_monotonic_time() / 1000.0 - start_time;
        question_times[current_question] = elapsed;
        total_time += elapsed;
        answered_questions++;
        running = FALSE;
        g_source_remove(timer_id);
    }

    // Update Average Time
    double avg_time = total_time / answered_questions;
    char avg_time_text[50];
    snprintf(avg_time_text, sizeof(avg_time_text), "Average Time: %.2f seconds", avg_time);
    gtk_label_set_text(GTK_LABEL(gtk_widget_get_parent(widget)), avg_time_text); // Update average time label
}

// Next Question Handler
static void next_question(GtkWidget *widget, gpointer data) {
    current_question++;
    if (current_question < total_questions) {
        // Load next question
        gtk_label_set_text(GTK_LABEL(gtk_widget_get_parent(widget)), questions[current_question].question);
        
        // Reset Timer
        gtk_label_set_text(GTK_LABEL(gtk_widget_get_parent(widget)), "00:00:00");

        running = TRUE;
        start_time = g_get_monotonic_time() / 1000.0;
        timer_id = g_timeout_add(1000, update_timer, gtk_widget_get_parent(widget));
    }
}

// Timer Update Handler
static gboolean update_timer(gpointer label) {
    if (!running) return FALSE;

    gdouble elapsed = g_get_monotonic_time() / 1000.0 - start_time;
    int hours = (int)(elapsed / 3600);
    int minutes = (int)((elapsed / 60)) % 60;
    int seconds = (int)(elapsed) % 60;

    // Update timer label with the new time
    gtk_label_set_text(GTK_LABEL(label), g_strdup_printf("%02d:%02d:%02d", hours, minutes, seconds));

    return TRUE;
}

