#include <gtk/gtk.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define GEMINI_API_KEY " "
#define API_URL "https://api.gemini.com/v1/speed-math"

// Global Variables for Timer
static int timer_running = 0;
static time_t start_time;
static GtkLabel *timer_label;
static double total_time = 0;
static int questions_answered = 0;

// Timer Update Function
static gboolean update_timer(gpointer data) {
    if (timer_running) {
        time_t current_time = time(NULL);
        int elapsed = (int)difftime(current_time, start_time);
        char time_str[50];
        sprintf(time_str, "%02d:%02d:%02d", elapsed / 3600, (elapsed % 3600) / 60, elapsed % 60);
        gtk_label_set_text(timer_label, time_str);
    }
    return TRUE;
}

// Start Timer Function
void start_timer() {
    timer_running = 1;
    start_time = time(NULL);
}

// Stop Timer Function
void stop_timer() {
    timer_running = 0;
    time_t current_time = time(NULL);
    total_time += difftime(current_time, start_time);
    questions_answered++;
}

// Average Time Calculation
char* calculate_average_time() {
    static char avg_time[50];
    double avg = (questions_answered > 0) ? (total_time / questions_answered) : 0;
    sprintf(avg_time, "Average Time: %.2f seconds", avg);
    return avg_time;
}

// HTTP POST Request to Gemini API
char* send_to_gemini(const char* question, const char* user_answer) {
    CURL *curl;
    CURLcode res;
    char post_fields[512];
    sprintf(post_fields, "api_key=%s&question=%s&answer=%s", GEMINI_API_KEY, question, user_answer);

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize CURL\n");
        return NULL;
    }

    curl_easy_setopt(curl, CURLOPT_URL, API_URL);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields);

    static char response[1024];
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (void *)strncpy);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "CURL request failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return NULL;
    }

    curl_easy_cleanup(curl);
    return response;
}

// Submit Button Callback
void on_submit_clicked(GtkButton *button, gpointer user_data) {
    GtkEntry *question_entry = GTK_ENTRY(user_data);
    const char *question = gtk_entry_get_text(GTK_ENTRY(question_entry));

    GtkEntry *answer_entry = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "answer_entry"));
    const char *user_answer = gtk_entry_get_text(GTK_ENTRY(answer_entry));

    stop_timer();
    char *response = send_to_gemini(question, user_answer);

    GtkLabel *result_label = GTK_LABEL(g_object_get_data(G_OBJECT(button), "result_label"));
    gtk_label_set_text(result_label, response ? response : "Error connecting to API");

    GtkLabel *average_label = GTK_LABEL(g_object_get_data(G_OBJECT(button), "average_label"));
    gtk_label_set_text(average_label, calculate_average_time());
}

// Dark/Light Mode Toggle
void on_toggle_theme(GtkSwitch *switch_btn, gpointer user_data) {
    gboolean state = gtk_switch_get_active(switch_btn);
    GtkCssProvider *css_provider = gtk_css_provider_new();
    if (state) {
        gtk_css_provider_load_from_data(css_provider, "* { background: #121212; color: white; }", -1, NULL);
    } else {
        gtk_css_provider_load_from_data(css_provider, "* { background: white; color: black; }", -1, NULL);
    }
    GtkStyleContext *style_context = gtk_widget_get_style_context(GTK_WIDGET(user_data));
    gtk_style_context_add_provider(style_context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
}

// Main Function
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Main Window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Speed Math App");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Main Layout
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Question Input
    GtkWidget *question_label = gtk_label_new("Enter Question:");
    gtk_grid_attach(GTK_GRID(grid), question_label, 0, 0, 1, 1);

    GtkWidget *question_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), question_entry, 1, 0, 1, 1);

    // Answer Input
    GtkWidget *answer_label = gtk_label_new("Your Answer:");
    gtk_grid_attach(GTK_GRID(grid), answer_label, 0, 1, 1, 1);

    GtkWidget *answer_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), answer_entry, 1, 1, 1, 1);

    // Submit Button
    GtkWidget *submit_button = gtk_button_new_with_label("Submit");
    gtk_grid_attach(GTK_GRID(grid), submit_button, 0, 2, 2, 1);
    g_object_set_data(G_OBJECT(submit_button), "answer_entry", answer_entry);
    g_object_set_data(G_OBJECT(submit_button), "result_label", gtk_label_new(NULL));
    g_signal_connect(submit_button, "clicked", G_CALLBACK(on_submit_clicked), question_entry);

    // Result Label
    GtkWidget *result_label = gtk_label_new("Result:");
    gtk_grid_attach(GTK_GRID(grid), result_label, 0, 3, 2, 1);

    // Timer
    timer_label = GTK_LABEL(gtk_label_new("00:00:00"));
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(timer_label), 0, 4, 2, 1);

    g_timeout_add(1000, update_timer, NULL);
    start_timer();

    // Average Time Label
    GtkWidget *average_label = gtk_label_new("Average Time: 0.00 seconds");
    gtk_grid_attach(GTK_GRID(grid), average_label, 0, 5, 2, 1);
    g_object_set_data(G_OBJECT(submit_button), "average_label", average_label);

    // Dark Mode Toggle
    GtkWidget *theme_switch = gtk_switch_new();
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Dark Mode:"), 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), theme_switch, 1, 6, 1, 1);
    g_signal_connect(theme_switch, "state-set", G_CALLBACK(on_toggle_theme), window);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}

