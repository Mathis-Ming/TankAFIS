import tkinter as tk
import serial
import json
import time
from tkinter import ttk
from tkinter import simpledialog
from tkinter import messagebox
from tkinter import filedialog
import csv
import re
import os
import shutil


class InstructionTableApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Instruction Table HMI")
        self.docname=''
        self.instructions = []  # List to store instructions
        self.table_saved = False  # Flag to track if the table is saved
        self.num_instructions = 0  # Variable to store the number of instructions
         # Label
        label = ttk.Label(root, text="ESILV - TuringAFIS", font=("Arial", 14, "bold"))
        label.grid(row=0, column=0, columnspan=5, pady=10)
        # Buttons
        new_button = ttk.Button(root, text="New", command=self.get_num_instructions)
        new_button.grid(row=self.num_instructions + 1, column=0, pady=10)

        save_button = ttk.Button(root, text="Save Table", command=self.save_table)
        save_button.grid(row=self.num_instructions + 1, column=1, pady=10)

        open_button = ttk.Button(root, text="Open Table", command=self.open_table)
        open_button.grid(row=self.num_instructions + 1, column=2, pady=10)

        send_button = ttk.Button(root, text="Send Table", command=self.send_table)
        send_button.grid(row=self.num_instructions + 1, column=3, pady=10)

        delete_button = ttk.Button(root, text="Delete Table", command=self.delete_table)
        delete_button.grid(row=self.num_instructions + 1, column=4, pady=10)
    def get_num_instructions(self):
        num_instructions = simpledialog.askinteger("Number of Instructions", "Enter the number of instructions (rows):", minvalue=1, maxvalue=100)
        if num_instructions:
            self.num_instructions = num_instructions
            self.create_table()
    def update_button_grid(self):
        # Place buttons at the bottom
        self.root.grid_rowconfigure(0, weight=1)  # Make row 2 (where buttons are) expandable
        for i in range(5):  # Assuming you have 5 buttons
            self.root.grid_columnconfigure(i, weight=1)  # Make each column expandable

    def create_table(self):
        # Create table headers
        headers = ["etatCourant", "valeurLue", "valeurAEcrire", "deplacement", "etatSuivant"]
        for col, header in enumerate(headers):
            label = ttk.Label(self.root, text=header)
            label.grid(row=2, column=col, padx=5, pady=5)
        
    
        # Entry widgets for instructions
        self.entry_widgets = []
        for row in range(3, self.num_instructions + 3):
            row_widgets = []
            for col in range(len(headers)):
                entry = ttk.Entry(self.root, width=15)
                entry.grid(row=row, column=col, padx=5, pady=5)
                row_widgets.append(entry)
            self.entry_widgets.append(row_widgets)

    def save_table(self):

        # Prompt user for the document name
        base_filename = filedialog.asksaveasfilename( filetypes=[("txt", "*.csv")])
     
        if base_filename:
            for entry_widgets in self.entry_widgets:
                etatCourant = entry_widgets[0].get()
                valeurLue = entry_widgets[1].get()
                valeurAEcrire = entry_widgets[2].get()
                deplacement = entry_widgets[3].get()
                etatSuivant = entry_widgets[4].get()

                if etatCourant and valeurLue and valeurAEcrire and deplacement and etatSuivant:
                    instruction = {
                        "etatCourant": etatCourant,
                        "valeurLue": int(valeurLue),
                        "valeurAEcrire": int(valeurAEcrire),
                        "deplacement": int(deplacement),
                        "etatSuivant": etatSuivant
                    }
                    self.instructions.append(instruction)

            self.table_saved = True
            print("Table saved:", self.instructions)
            csv_filename = f"{base_filename}.csv"
            txt_filename = f"{base_filename}.txt"
            self.docname=txt_filename
            self.save_to_csv(csv_filename, self.instructions)
            self.generate_c_code(txt_filename)
            
    def save_to_csv(self, filename, data):
        try:
            with open(filename, 'w', newline='') as csvfile:
                fieldnames = ["row", "etatCourant", "valeurLue", "valeurAEcrire", "deplacement", "etatSuivant"]
                writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
                writer.writeheader()
                for row, instruction in enumerate(data):
                    writer.writerow({"row": row, **instruction})
            messagebox.showinfo("Info", f"Table saved to {filename}")
        except Exception as e:
            messagebox.showerror("Error", f"Error saving table: {e}")

    def generate_c_code(self,filename):
        if filename:
            try:
                with open(filename, 'w') as txtfile:
                    txtfile.write(f"const int tailleTableau = {self.num_instructions};\n\n")
                    txtfile.write("Instruction tableauInstructions[tailleTableau] = {\n")
                    for row, entry_widgets in enumerate(self.entry_widgets):
                        values = [entry.get() for entry in entry_widgets]
                        txtfile.write(f'  {{ "{values[0]}", {values[1]}, {values[2]}, {values[3]}, "{values[4]}" }}')
                        if row < self.num_instructions - 1:
                            txtfile.write(',')
                        txtfile.write('\n')
                    txtfile.write("};")
                messagebox.showinfo("Info", f"C code saved to {filename}")
            except Exception as e:
                messagebox.showerror("Error", f"Error saving C code: {e}")

    def open_table(self):
        file_path = filedialog.askopenfilename(filetypes=[("CSV Files", "*.csv")])
        if file_path:
            self.docname=f"{file_path[:-3]}txt"
            print(self.docname)
            self.read_from_csv(file_path)

    
    def read_from_csv(self, filename):
        try:
            with open(filename, 'r') as csvfile:
                reader = csv.DictReader(csvfile)
                for row_index, row in enumerate(reader):
                    self.clear_row(row_index)
                    for col, header in enumerate(["etatCourant", "valeurLue", "valeurAEcrire", "deplacement", "etatSuivant"]):
                        value = row.get(header, '')
                        self.entry_widgets[row_index][col].insert(0, value)
        except Exception as e:
            messagebox.showerror("Error", f"Error reading table from CSV: {e}")

    def clear_row(self, row_index):
        for col in range(len(["etatCourant", "valeurLue", "valeurAEcrire", "deplacement", "etatSuivant"])):
            self.entry_widgets[row_index][col].delete(0, 'end')

    def chercher_fichier_ino(self, dossier_courant='.'):
        chemin_complet = None

        for dossier_racine, sous_dossiers, fichiers in os.walk(dossier_courant):
            for fichier in fichiers:
                print(fichier)
                if fichier.endswith('.ino'):
                    chemin_complet = os.path.join(dossier_racine, fichier)
                    break  # Arrête la recherche dès qu'un fichier .ino est trouvé
            if chemin_complet:
                break  # Arrête la recherche dès qu'un fichier .ino est trouvé

        return chemin_complet

    def send_table(self):
        # Read the contents of the .ino file
        #file_path = "C:/Users/owenn/OneDrive/A5/ESILV/Pi2_Concours_Robotique/BluetoothCode/IHM_Instructions/Code_AutotestV7.ino"
        file_path = self.chercher_fichier_ino()

        with open(file_path, 'r') as file:
            ino_content = file.read()

        # Define a regular expression pattern for matching the old structure and tailleTableau
        pattern = re.compile(r"const int tailleTableau =[\s\S]*?};", re.DOTALL)
        print(self.docname)
        # Read the new structure from autotest1.txt
        with open(self.docname, 'r') as new_structure_file:
            new_structure = new_structure_file.read()

        # Print old and new structures for debugging
            match = re.search(pattern, ino_content)
        if match is not None:
            print("Old Structure:")
            print(match.group(0))
        else:
            print("No match found.")
        # print("Old Structure:")
        # print(re.search(pattern, ino_content).group(0))
        print("\nNew Structure:")
        print(new_structure)

        # Use the regular expression to find and replace the old structure with the new one
        modified_content = re.sub(pattern, new_structure, ino_content)

        # Print the modified content for debugging
        print("\nModified Content:")
        print(modified_content)

        # Write the modified content back to the .ino file
        with open(file_path, 'w') as file:
            file.write(modified_content)
            file.flush()

       
    def delete_table(self):
        self.instructions.clear()
        for row, entry_widgets in enumerate(self.entry_widgets):
            for entry in entry_widgets:
                entry.delete(0, 'end')
                entry.destroy()
        self.table_saved = False
        print("Table deleted")

    def is_valid_input(self, etatCourant, etatSuivant, valeurLue, valeurAEcrire, deplacement):
        try:
            int_valeurLue = int(valeurLue)
            int_valeurAEcrire = int(valeurAEcrire)
            int_deplacement = int(deplacement)
            if not (isinstance(etatCourant, str) and isinstance(etatSuivant, str)):
                messagebox.showwarning("Warning", "etatCourant and etatSuivant must be strings.")
                return False
            if not (int_valeurLue in [1, 0, -1] and int_valeurAEcrire in [1, 0, -1] and int_deplacement in [1, 0, -1]):
                messagebox.showwarning("Warning", "Invalid values for int fields. Accepted values: 1, 0, -1.")
                return False
            return True
        except ValueError:
            messagebox.showwarning("Warning", "Invalid input. Check your entries.")
            return False

    def change_and_add_extension(file_path, new_extension=".txt"):
        # Extract the base name of the file (without extension)
        file_name = os.path.splitext(os.path.basename(file_path))[0]

        # Add the new extension
        new_file_name = file_name + new_extension
        return new_file_name

if __name__ == "__main__":
    root = tk.Tk()
    app = InstructionTableApp(root)
    root.mainloop()
