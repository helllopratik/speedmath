import os
import google.generativeai as genai
from dotenv import load_dotenv
import tkinter as tk
from tkinter import scrolledtext, messagebox
from threading import Thread
import re
import time
import json
import sys

# Load environment variables for Gemini API key
load_dotenv()
API_KEY = os.getenv('GEMINI_API_KEY')

# Configure the generative AI model
genai.configure(api_key=API_KEY)
model = genai.GenerativeModel('gemini-pro')

# Default query for the first question
DEFAULT_QUERY = (
    "Give one speed math question for Indian banking exams, including approximation and simplification. Each question should have options, which may be incorrect. Wait for the user's answer before showing the solution and steps. Do not evaluate or provide solutions unless asked."
)

# Session state to retain context
session = {
    "current_question": None,
    "current_options": None,
    "correct_answer": None
}

# Global variables for timer and average time calculation
start_time = None
time_taken = []
question_count = 0
running_timer = True

# Function to update the timer display
def update_timer():
    while running_timer:
        if start_time:
            elapsed_time = time.time() - start_time
            timer_label.config(text=f"Timer: {elapsed_time:.2f} seconds")
        else:
            timer_label.config(text="Timer: --")
        time.sleep(1)

# Function to send queries to Gemini API
def send_query_to_gemini(query):
    try:
        chat = model.start_chat(history=[])
        response = chat.send_message(query)
        return " ".join(part.text for part in response.parts if part.text)
    except Exception as e:
        print(f"Error during interaction with Gemini AI: {e}")
        return f"Failed to get response: {e}"

# Function to validate user input (i.e., options 'A', 'B', 'C', 'D')
def validate_user_input(user_input):
    valid_options = ['A', 'B', 'C', 'D']
    if user_input.upper() in valid_options:
        return user_input.upper()
    else:
        chat_area.config(state='normal')
        chat_area.insert(tk.END, "Bot: Invalid input! Please choose a valid option (A, B, C, D).\n", "bot")
        chat_area.config(state='disabled')
        return None

# Function to handle user input
def on_send():
    global session, time_taken, question_count

    user_input = entry.get().strip()
    if user_input:
        chat_area.config(state='normal')
        chat_area.insert(tk.END, f"You: {user_input}\n", "user")
        chat_area.config(state='disabled')
        entry.delete(0, tk.END)

        # Validate user input (A, B, C, D)
        validated_input = validate_user_input(user_input)
        if validated_input:
            # Timer logic
            if start_time is not None:
                elapsed_time = time.time() - start_time
                time_taken.append(elapsed_time)
                question_count += 1

                avg_time = sum(time_taken) / question_count
                chat_area.config(state='normal')
                chat_area.insert(tk.END, f"Bot: Time taken for this question: {elapsed_time:.2f} seconds.\n", "bot")
                chat_area.insert(tk.END, f"Bot: Average time per question: {avg_time:.2f} seconds.\n", "bot")
                chat_area.config(state='disabled')

            # Check if current question and options exist
            if session["current_question"] and session["current_options"]:
                response_text = send_query_to_gemini(
                    f"Check the user's answer '{validated_input}' for the question: '{session['current_question']}'. Provide the solution and correct answer."
                )
                chat_area.config(state='normal')
                chat_area.insert(tk.END, f"Bot: {response_text}\n", "bot")
                chat_area.config(state='disabled')

                # Fetch the next question
                Thread(target=load_default_query).start()
            else:
                chat_area.config(state='normal')
                chat_area.insert(tk.END, "Bot: I cannot validate your answer as the question is missing. Please restart.\n", "bot")
                chat_area.config(state='disabled')
        else:
            messagebox.showwarning("Invalid Input", "Please enter a valid option (A, B, C, D).")
    else:
        messagebox.showwarning("Invalid Input", "Please enter your answer.")

# Function to handle the default query
def load_default_query():
    global session, start_time

    chat_area.config(state='normal')
    chat_area.insert(tk.END, "Bot: Sending default query to Gemini...\n", "bot")
    chat_area.config(state='disabled')

    def handle_default_query():
        response_text = send_query_to_gemini(DEFAULT_QUERY)

        # Extract question and options
        question_match = re.search(r"\*\*Question.*:\*\*([\s\S]+?)\*\*Options", response_text, re.DOTALL)
        options_match = re.search(r"\*\*Options:\*\*([\s\S]+?)\*\*Note", response_text, re.DOTALL)

        if question_match:
            session["current_question"] = question_match.group(1).strip()
        else:
            session["current_question"] = "N/A"
        
        if options_match:
            session["current_options"] = options_match.group(1).strip()
        else:
            session["current_options"] = "N/A"

        # Display the question and options
        chat_area.config(state='normal')
        chat_area.insert(tk.END, f"Bot: {response_text}\n", "bot")
        chat_area.config(state='disabled')

        start_time = time.time()  # Start the timer

    thread = Thread(target=handle_default_query)
    thread.start()

# Restart the app
def on_ask_again():
    save_session()
    root.destroy()
    os.execl(sys.executable, sys.executable, *sys.argv)

# Save session state to a file
def save_session():
    with open("session.json", "w") as file:
        json.dump(session, file)

# Load session state from a file
def load_session():
    global session
    try:
        with open("session.json", "r") as file:
            session = json.load(file)
    except FileNotFoundError:
        session = {"current_question": None, "current_options": None, "correct_answer": None}

# Gracefully close the app
def on_close():
    global running_timer
    running_timer = False
    save_session()
    if messagebox.askokcancel("Quit", "Do you want to quit?"):
        root.destroy()

# Main GUI Window
root = tk.Tk()
root.title("SpeedMath - Conversational Bot")
root.geometry("900x700")

# Timer Display
timer_label = tk.Label(root, text="Timer: --", font=("Helvetica", 14))
timer_label.pack(pady=5)

# Chat Area
frame = tk.Frame(root)
frame.pack(padx=10, pady=10, fill=tk.BOTH, expand=True)

chat_area = scrolledtext.ScrolledText(frame, wrap=tk.WORD, state='disabled', height=25, width=110)
chat_area.tag_configure("user", foreground="blue")
chat_area.tag_configure("bot", foreground="green")
chat_area.pack(fill=tk.BOTH, expand=True)

# Input Field
entry = tk.Entry(root, width=80)
entry.pack(padx=10, pady=10, fill=tk.X, expand=True)

# Send Button
send_button = tk.Button(root, text="Send", command=on_send)
send_button.pack(side=tk.LEFT, padx=10, pady=10)

# Ask Again Button
ask_again_button = tk.Button(root, text="Restart", command=on_ask_again)
ask_again_button.pack(side=tk.LEFT, padx=10, pady=10)

# Start the Timer Thread
timer_thread = Thread(target=update_timer, daemon=True)
timer_thread.start()

# Load session and default query
load_session()
Thread(target=load_default_query).start()

# Start the App
root.protocol("WM_DELETE_WINDOW", on_close)
root.mainloop()
