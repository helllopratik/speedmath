import os
import google.generativeai as genai
from dotenv import load_dotenv
import tkinter as tk
from tkinter import scrolledtext, messagebox
from threading import Thread
import re
import time  # For timer functionality
import sys  # For restarting the program

# Load environment variables for Gemini API key
load_dotenv()
API_KEY = os.getenv('GEMINI_API_KEY')

# Configure the generative AI model
genai.configure(api_key=API_KEY)
model = genai.GenerativeModel('gemini-pro')

# Default query for the first question
DEFAULT_QUERY = (
    "Provide one speed math PYQ for Indian Banking with options (may be incorrect) per request. No answer until I provide mine."
)

# Global variables for timer and average time calculation
start_time = None
time_taken = []
question_count = 0

# Function to send queries to Gemini API
def send_query_to_gemini(query):
    try:
        chat = model.start_chat(history=[])
        response = chat.send_message(query)
        all_responses = []
        for part in response.parts:
            if part.text:
                all_responses.append(part.text)
        return " ".join(all_responses)
    except Exception as e:
        print(f"Error during interaction with Server: {e}")
        return f"Failed to get response: {e}"

# Function to handle user input
def on_send():
    global start_time, time_taken, question_count

    user_input = entry.get()
    if user_input and re.match(r'^\d$', user_input):  # Validate single-digit numeric input
        chat_area.config(state='normal')
        chat_area.insert(tk.END, f"You: {user_input}\n")
        chat_area.config(state='disabled')
        entry.delete(0, tk.END)

        # Calculate time taken for the current question
        if start_time is not None:
            elapsed_time = time.time() - start_time
            time_taken.append(elapsed_time)
            question_count += 1

            # Display average time
            avg_time = sum(time_taken) / question_count
            chat_area.config(state='normal')
            chat_area.insert(tk.END, f"Bot: Time taken for this question: {elapsed_time:.2f} seconds.\n")
            chat_area.insert(tk.END, f"Bot: Average time per question: {avg_time:.2f} seconds.\n")
            chat_area.config(state='disabled')

        def handle_response():
            if user_input == "1":
                # Send a request for the next question
                response_text = send_query_to_gemini("Send the next question only. No solution.")
                chat_area.config(state='normal')
                chat_area.insert(tk.END, f"Bot: {response_text}\n")
                chat_area.config(state='disabled')
                global start_time
                start_time = time.time()  # Restart the timer for the next question
            else:
                # Terminate the connection and notify the user
                chat_area.config(state='normal')
                chat_area.insert(tk.END, "Bot: Connection terminated. Thank you for using SpeedMath!\n")
                chat_area.config(state='disabled')
                send_button.config(state=tk.DISABLED)
                entry.config(state=tk.DISABLED)

        # Start a thread to handle the response
        thread = Thread(target=handle_response)
        thread.start()
    else:
        messagebox.showwarning("Invalid Input", "Please enter a single numeric character.")

# Function to handle the default query
def load_default_query():
    global start_time
    chat_area.config(state='normal')
    chat_area.insert(tk.END, "Bot: Sending default query to Gemini...\n")
    chat_area.config(state='disabled')

    def handle_default_query():
        response_text = send_query_to_gemini(DEFAULT_QUERY)
        chat_area.config(state='normal')
        chat_area.insert(tk.END, f"Bot: {response_text}\n")
        chat_area.config(state='disabled')
        global start_time
        start_time = time.time()  # Start the timer for the first question

    thread = Thread(target=handle_default_query)
    thread.start()

# Restart the app
def on_ask_again():
    root.destroy()
    os.execl(sys.executable, sys.executable, *sys.argv)

# Gracefully close the app
def on_close():
    if messagebox.askokcancel("Quit", "Do you want to quit?"):
        root.destroy()

# Main GUI Window
root = tk.Tk()
root.title("SpeedMath - Conversational Bot")
root.geometry("900x650")

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

# Start the App
root.protocol("WM_DELETE_WINDOW", on_close)
Thread(target=load_default_query).start()

root.mainloop()
