import sys

def clean_conflicts(filepath):
    with open(filepath, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    new_lines = []
    in_head = False
    
    for line in lines:
        if line.startswith('<<<<<<< HEAD'):
            in_head = True
            continue
        if line.startswith('======='):
            in_head = False
            continue
        if line.startswith('>>>>>>> dev'):
            continue
        
        if not in_head:
            new_lines.append(line)
            
    with open(filepath, 'w', encoding='utf-8') as f:
        f.writelines(new_lines)

if __name__ == "__main__":
    for path in sys.argv[1:]:
        print(f"Cleaning {path}...")
        clean_conflicts(path)
