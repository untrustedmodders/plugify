import os
import subprocess

# Set the base directory
base_directory = ".."

# Check if the directory exists, if not, create it
target_directory = os.path.join(base_directory, "addons", "wizard", "bin")
if not os.path.exists(target_directory):
    os.makedirs(target_directory)

# Find all .cs files in the specified directory and its subdirectories
files = []
for root, _, filenames in os.walk(os.path.join("Wizard", "Wizard", "src")):
    for filename in filenames:
        if filename.endswith(".cs"):
            files.append(os.path.join(root, filename))

# Build the command to compile the .cs files into a library
command = [
    "csc",
    "-target:library",
    f"-out:{os.path.join(target_directory, 'Wizard.dll')}",
] + files

# Execute the command
subprocess.run(command, shell=True)