import os
import subprocess

# Set the base directory
base_directory = ".."

# Set the core library
core_library = os.path.join(base_directory, "addons", "wizard", "bin", "wizard.dll")

# Check if the "../addons/wizard/plugins" directory exists, and create it if not
target_directory = os.path.join(base_directory, "addons", "wizard", "plugins")
if not os.path.exists(target_directory):
    os.makedirs(target_directory)

# Initialize a count variable
count = 0

# Use os.walk to find all .cs files in the specified directory and its subdirectories
for root, _, filenames in os.walk(os.path.join("Plugins", "Plugins", "src")):
    for filename in filenames:
        if filename.endswith(".cs"):
            # Build the command to compile the .cs file into a library
            source_file = os.path.join(root, filename)
            output_file = os.path.join(target_directory, os.path.splitext(filename)[0] + ".dll")
            
            command = [
                "csc",
                "-target:library",
                f"-reference:{core_library}",
                f"-out:{output_file}",
                source_file
            ]
            
            # Execute the command
            subprocess.run(command, shell=True)
            
            # Increment the count
            count += 1

# Optionally, you can print the count of processed files
print(f"Processed {count} .cs files and created corresponding DLLs in the 'plugins' directory.")