import os
import google.generativeai as genai
from dotenv import load_dotenv
import tkinter as tk
from tkinter import scrolledtext, messagebox
from threading import Thread
import re
import time
import sys

# Load environment variables for Gemini API key
load_dotenv()
API_KEY = os.getenv('GEMINI_API_KEY')

# Configure the generative AI model
genai.configure(api_key=API_KEY)
model = genai.GenerativeModel('gemini-pro')

# Default query for the first question
DEFAULT_QUERY = (
    "Provide one speed math question at a time for practice for Indian banking exams. "
    "Include approximation and simplification questions. Each question should have "
    "options but mention that the options could be incorrect. After displaying the question, "
    "wait for the user's answer before revealing the correct solution and stepwise explanation. "
    "Do not evaluate the user's response or provide any solution unless explicitly prompted after the user's input."
)

# Global variables for timer and average time calculation
start_time = None
time_taken = []
question_count = 0
running_timer = True
current_question = ""  # Store the current question
current_options = ""   # Store the options for the current question

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
        return "".join(part.text for part in response.parts if part.text)
    except Exception as e:
        print(f"Error during interaction with Gemini AI: {e}")
        return f"Failed to get response: {e}"

# Function to handle user input
def on_send():
    global start_time, time_taken, question_count, current_question, current_options

    user_input = entry.get().strip()
    if user_input:
        chat_area.config(state='normal')
        chat_area.insert(tk.END, f"You: {user_input}\n")
        chat_area.config(state='disabled')
        entry.delete(0, tk.END)

        if start_time is not None:
            elapsed_time = time.time() - start_time
            time_taken.append(elapsed_time)
            question_count += 1
            avg_time = sum(time_taken) / question_count
            chat_area.config(state='normal')
            chat_area.insert(tk.END, f"Bot: Time taken for this question: {elapsed_time:.2f} seconds.\n")
            chat_area.insert(tk.END, f"Bot: Average time per question: {avg_time:.2f} seconds.\n")
            chat_area.config(state='disabled')

        def handle_response():
            response_text = send_query_to_gemini(
                f"Question: {current_question}\nOptions: {current_options}\nUser's Answer: {user_input}. "
                f"Verify if the user's answer is correct and provide a stepwise solution."
            )
            chat_area.config(state='normal')
            chat_area.insert(tk.END, f"Bot: {response_text}\n")
            chat_area.config(state='disabled')

        Thread(target=handle_response).start()
    else:
        messagebox.showwarning("Invalid Input", "Please enter your answer.")

# Function to handle the default query
def load_default_query():
    global start_time, current_question, current_options
    chat_area.config(state='normal')
    chat_area.insert(tk.END, "Bot: Sending default query to Gemini...\n")
    chat_area.config(state='disabled')

    def handle_default_query():
        response_text = send_query_to_gemini(DEFAULT_QUERY)
        question_match = re.search(r"(?<=\*\*Question.*:\*\*).+?(?=\*\*Options)", response_text, re.DOTALL)
        options_match = re.search(r"(?<=\*\*Options:\*\*).+?(?=\*\*Note)", response_text, re.DOTALL)
        global current_question, current_options
        current_question = question_match.group(0).strip() if question_match else "N/A"
        current_options = options_match.group(0).strip() if options_match else "N/A"

        chat_area.config(state='normal')
        chat_area.insert(tk.END, f"Bot: {response_text}\n")
        chat_area.config(state='disabled')
        global start_time
        start_time = time.time()

    Thread(target=handle_default_query).start()

# Restart the app
def on_ask_again():
    root.destroy()
    os.execl(sys.executable, sys.executable, *sys.argv)

# Gracefully close the app
def on_close():
    global running_timer
    running_timer = False
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
Thread(target=update_timer, daemon=True).start()

# Start the App
root.protocol("WM_DELETE_WINDOW", on_close)
Thread(target=load_default_query).start()

root.mainloop()
