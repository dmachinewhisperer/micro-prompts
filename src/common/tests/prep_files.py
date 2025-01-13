import re
import sys

def replace_functions_in_file(file_name, reverse=False):
    try:
        with open(file_name, 'r') as f:
            content = f.read()

        content_new = content
        replacements = 0

        if reverse:
            # reverse: Replace __cJSON_CreateObject() with cJSON_CreateObject() and __cJSON_CreateArray() with cJSON_CreateArray()
            content_new, num_replacements_object = re.subn(r'\b___cJSON_CreateObject\(\)', 'cJSON_CreateObject()', content_new)
            replacements += num_replacements_object

            content_new, num_replacements_array = re.subn(r'\b___cJSON_CreateArray\(\)', 'cJSON_CreateArray()', content_new)
            replacements += num_replacements_array
        else:
            # forwward: Rrplace cJSON_CreateObject() with __cJSON_CreateObject() and cJSON_CreateArray() with __cJSON_CreateArray()
            content_new, num_replacements_object = re.subn(r'\bcJSON_CreateObject\(\)', '___cJSON_CreateObject()', content_new)
            replacements += num_replacements_object

            content_new, num_replacements_array = re.subn(r'\bcJSON_CreateArray\(\)', '___cJSON_CreateArray()', content_new)
            replacements += num_replacements_array

        #write the changes back to the file
        if replacements > 0:
            with open(file_name, 'w') as f:
                f.write(content_new)

        return replacements

    except FileNotFoundError:
        print(f"Error: The file {file_name} was not found.")
        return 0
    except Exception as e:
        print(f"An error occurred while processing {file_name}: {e}")
        return 0

def main():
    # Check if we have exactly one file passed
    if len(sys.argv) < 2:
        print("Usage: py prep_do.py file1.c [--reverse]")
        sys.exit(1)

    file1 = sys.argv[1]
    reverse = '--rev' in sys.argv  # Check for reverse flag

    # Perform the replacement operation
    replacements = replace_functions_in_file(file1, reverse)
    if reverse:
        print(f"{file1}: {replacements} replacements undone.")
    else:
        print(f"{file1}: {replacements} replacements made.")

if __name__ == '__main__':
    main()
