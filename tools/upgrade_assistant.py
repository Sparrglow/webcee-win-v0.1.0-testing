import re
import sys
import os

def upgrade_file(file_path):
    """
    Upgrades a file from legacy WebCee syntax to the new function-scope syntax.
    """
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Define patterns
    # We use simple token replacement which is robust for nested structures
    # WCE_ROW_BEGIN -> wce_row({
    # WCE_ROW_END -> });
    
    replacements = [
        (r'WCE_ROW_BEGIN', 'wce_row({'),
        (r'WCE_ROW_END', '});'),
        (r'WCE_COL_BEGIN', 'wce_col({'),
        (r'WCE_COL_END', '});'),
        (r'WCE_COLUMN_BEGIN', 'wce_col({'),
        (r'WCE_COLUMN_END', '});'),
        (r'WCE_CARD_BEGIN', 'wce_card("", {'),
        (r'WCE_CARD_END', '});'),
        (r'WCE_CONTAINER_BEGIN', 'wce_container({'),
        (r'WCE_CONTAINER_END', '});'),
        (r'WCE_PANEL_BEGIN', 'wce_panel({'),
        (r'WCE_PANEL_END', '});'),
        
        # Leaf nodes - try to convert uppercase macros to lowercase functions
        # This is heuristic and might need manual review
        (r'WCE_TEXT\(', 'wce_text('),
        (r'WCE_BUTTON\(', 'wce_button('),
        (r'WCE_SLIDER\(', 'wce_slider('),
        (r'WCE_PROGRESS\(', 'wce_progress('),
        (r'WCE_INPUT\(', 'wce_input('),
    ]

    new_content = content
    for pattern, replacement in replacements:
        new_content = re.sub(pattern, replacement, new_content)

    if new_content != content:
        print(f"Upgrading {file_path}...")
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(new_content)
    else:
        print(f"No changes needed for {file_path}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python upgrade_assistant.py <file_or_directory>")
        sys.exit(1)

    target = sys.argv[1]
    if os.path.exists(target):
        if os.path.isfile(target):
            upgrade_file(target)
        elif os.path.isdir(target):
            for root, dirs, files in os.walk(target):
                for file in files:
                    if file.endswith('.c') or file.endswith('.h') or file.endswith('.wce'):
                        upgrade_file(os.path.join(root, file))
    else:
        print(f"Error: {target} not found")
