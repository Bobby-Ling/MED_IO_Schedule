import tkinter as tk
from tkinterdnd2 import TkinterDnD, DND_FILES
import matplotlib.pyplot as plt
import pickle
from tkinter import messagebox


# Function to load and plot the figure from the pickle file
def load_and_plot(pickle_file_path):
    try:
        with open(pickle_file_path, "rb") as f:
            fig = pickle.load(f)
        fig.show()
    except Exception as e:
        messagebox.showerror("Error", f"Failed to load the plot: {e}")


# Function to handle drag-and-drop event
def on_drop(event):
    file_path = event.data
    if file_path.endswith(".pickle"):
        load_and_plot(
            file_path.strip("{").strip("}")
        )  # Strip curly braces added by tkinter
    else:
        messagebox.showerror("Invalid File", "Please drop a .pickle file.")


# Create the main window using TkinterDnD for drag-and-drop
root = TkinterDnD.Tk()  # Inherits from TkinterDnD
root.title("Plot Loader")
root.geometry("400x200")

# Label to display instructions
label = tk.Label(
    root, text="Drag and drop a .pickle file to load the plot", font=("Arial", 14)
)
label.pack(pady=50)

# Enable the root window to accept file drops
root.drop_target_register(DND_FILES)
root.dnd_bind("<<Drop>>", on_drop)

# Start the main loop
root.mainloop()
