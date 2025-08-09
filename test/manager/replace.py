import os
import shutil

# Path to the new DLL you want to copy
source_dll = r"D:\plugify-module-cpp\test\example_plugin\cmake-build-debug\test.dll"

# Root directory to search in
root_dir = r"D:\plugify\build\Windows\Debug\test\plug\extensions"

def replace_dll(source, root):
    if not os.path.isfile(source):
        print(f"[ERROR] Source DLL not found: {source}")
        return

    for dirpath, _, filenames in os.walk(root):
        if "test.dll" in filenames:
            dll_path = os.path.join(dirpath, "test.dll")
            try:
                shutil.copy2(source, dll_path)  # copy2 preserves metadata
                print(f"[OK] Replaced: {dll_path}")
            except Exception as e:
                print(f"[FAIL] Could not replace {dll_path}: {e}")

if __name__ == "__main__":
    replace_dll(source_dll, root_dir)