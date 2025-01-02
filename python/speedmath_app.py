import os
import google.generativeai as genai
from dotenv import load_dotenv
import tkinter as tk
from tkinter import scrolledtext, messagebox
from threading import Thread
import sys  # Import sys to handle restarting the program

# Load environment variables for Gemini API key
load_dotenv()
API_KEY = os.getenv('GEMINI_API_KEY')

# Configure the generative AI model
genai.configure(api_key=API_KEY)

# Initialize generative model for SpeedMath app chat
model = genai.GenerativeModel('gemini-pro')

def send_query_to_gemini(query):
    """Sends a query to Gemini AI and returns the response."""
    try:
        chat = model.start_chat(history=[])
        response = chat.send_message(query)
        
        # Collect responses from all parts
        all_responses = []
        for part in response.parts:
            if part.text:
                all_responses.append(part.text)
        
        return " ".join(all_responses)
    except Exception as e:
        print(f"Error during interaction with Gemini AI: {e}")
        return "Failed to get response from Gemini AI."

# GUI function to send query to Gemini AI
def on_send():
    user_input = entry.get()
    if user_input:
        chat_area.config(state='normal')
        chat_area.insert(tk.END, f"You: {user_input}\n")
        chat_area.config(state='disabled')
        entry.delete(0, tk.END)

        # Disable the send button and start a thread to handle the request
        send_button.config(state=tk.DISABLED)
        ask_again_button.config(state=tk.NORMAL)

        def detect_and_respond():
            # Send the user input to Gemini AI
            response_text = send_query_to_gemini(user_input)
            
            # Display the Gemini AI's response
            chat_area.config(state='normal')
            chat_area.insert(tk.END, f'Bot: {response_text}\n')
            chat_area.config(state='disabled')
        
        # Start a thread to handle the response without blocking the main UI
        thread = Thread(target=detect_and_respond)
        thread.start()

def on_ask_again():
    """Restart the program."""
    root.destroy()
    os.execl(sys.executable, sys.executable, *sys.argv)

def on_close():
    """Close the application gracefully."""
    if messagebox.askokcancel("Quit", "Do you want to quit?"):
        root.destroy()

# Create the main window for SpeedMath app
root = tk.Tk()
root.title("SpeedMath - Conversational Bot")
root.geometry("800x600")

# Create a frame for the chat area
frame = tk.Frame(root)
frame.pack(padx=10, pady=10, fill=tk.BOTH, expand=True)

# Create a scrollable text area for the chat
chat_area = scrolledtext.ScrolledText(frame, wrap=tk.WORD, state='disabled', height=25, width=100)
chat_area.pack(fill=tk.BOTH, expand=True)

# Create an entry box for user input
entry = tk.Entry(root, width=70)
entry.pack(padx=10, pady=10, fill=tk.X, expand=True)

# Create a send button
send_button = tk.Button(root, text="Send", command=on_send)
send_button.pack(side=tk.LEFT, padx=10, pady=10)

# Create an ask again button
ask_again_button = tk.Button(root, text="Ask Again", command=on_ask_again, state=tk.DISABLED)
ask_again_button.pack(side=tk.LEFT, padx=10, pady=10)

# Set the on-close event
root.protocol("WM_DELETE_WINDOW", on_close)

# Allow resizing for better UX
root.rowconfigure(0, weight=1)
root.columnconfigure(0, weight=1)
frame.rowconfigure(0, weight=1)
frame.columnconfigure(0, weight=1)

# Start the GUI event loop
root.mainloop()
