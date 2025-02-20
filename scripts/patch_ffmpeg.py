import sys

def apply_patch(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    for i, line in enumerate(lines):
        if "#define MAX_URL_SIZE" in line:
            lines[i] = "#define MAX_URL_SIZE 8192\n"
        
    with open(file_path, 'w') as file:
        file.writelines(lines)

if __name__ == "__main__":
    file_path = sys.argv[1]  # First argument is the file path
    print("Patching file:", file_path)
    apply_patch(file_path)
    print("Successfully patched", file_path)
