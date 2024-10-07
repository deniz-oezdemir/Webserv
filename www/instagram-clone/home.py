#!/usr/bin/env python3

import warnings
warnings.filterwarnings("ignore", category=DeprecationWarning)

import os
import cgi
import json


def get_env_variable(var_name):
    return os.environ.get(var_name)


def get_username_from_token(token, db_path):
    try:
        with open(db_path, "r") as db_file:
            for line in db_file:
                entry = json.loads(line.strip())
                if entry.get("token") == token:
                    return entry.get("username")
    except FileNotFoundError:
        return None
    except json.JSONDecodeError:
        return None
    return None


def list_files(username, upload_dir):
    try:
        # List files in the upload directory
        files = os.listdir(upload_dir)
        file_list = [f for f in files if os.path.isfile(os.path.join(upload_dir, f))]

        # Generate HTML content
        print()
        print("<!DOCTYPE html>")
        print("<html lang='en'>")
        print("<head>")
        print("    <meta charset='UTF-8'>")
        print(
            "    <meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        )
        print("    <title>Instagram Clone CGI</title>")
        print("    <link rel='stylesheet' href='css/styles.css'>")
        print("</head>")
        print("<body>")
        print("    <h1>Instagram Clone</h1>")
        print(f"    <h2>Welcome back, {username}</h2>")
        print("    <div class='container'>")
        print("        <div class='photo-grid' id='photo-grid'>")

        for filename in file_list:
            print("            <div class='photo-container'>")
            print(f"                <img src='upload/{filename}' alt='Photo'>")
            print(
                f"                <form class='delete-form' data-filename='{filename}'>"
            )
            print(
                "                    <button type='button' class='delete-btn'>Delete</button>"
            )
            print("                </form>")
            print("            </div>")

        print("        </div>")
        print("    </div>")
        print("    <div class='container'>")
        print(
            "        <form id='upload-form' enctype='multipart/form-data' method='post' action='cgi/post.py'>"
        )
        print(
            "            <input type='file' name='file' id='file-input' accept='image/*' required>"
        )
        print("            <button type='submit' id='upload-btn'>Post Photo</button>")
        print("        </form>")
        print("    </div>")
        print("    <div class='container'>")
        print("        <form id='get-time-form' method='get' action='cgi/get.py'>")
        print("            <button type='submit' id='get-time-btn'>Get Time</button>")
        print("        </form>")
        print("    </div>")
        print("    <script>")
        print("        document.querySelectorAll('.delete-btn').forEach(button => {")
        print("            button.addEventListener('click', function() {")
        print("                const form = this.closest('.delete-form');")
        print("                const filename = form.getAttribute('data-filename');")
        print(
            "                fetch(`cgi/delete_cgi?/upload/${filename}`, { method: 'DELETE' })"
        )
        print("                    .then(response => {")
        print("                        if (response.ok) {")
        print("                            form.closest('.photo-container').remove();")
        print("                        } else {")
        print("                            console.error('Failed to delete the file');")
        print("                        }")
        print("                    })")
        print("                    .catch(error => {")
        print("                        console.error('Error:', error);")
        print("                    });")
        print("            });")
        print("        });")
        print("    </script>")
        print("</body>")
        print("</html>")

    except Exception as e:
        print("<html><body>")
        print(f"<h1>Error: {str(e)}</h1>")
        print("</body></html>")


def main():
    token = get_env_variable("COOKIE")
    root_dir = get_env_variable("ROOT_DIR")
    upload_dir = get_env_variable("UPLOAD_DIR")

    if not root_dir:
        exit(1)
    if not upload_dir:
        upload_dir = os.path.join(root_dir, "upload")
    if not token:
        print("CGI_HEADERS")
        print("Status: 302")
        print("Location: /cgi/cookie_check_cgi")
        print("CGI_HEADERS_END")
        print()
        return

    db_path = os.path.join(root_dir, "db", "db.json")
    username = get_username_from_token(token, db_path)

    if not username:
        print("CGI_HEADERS")
        print("Status: 302")
        print("Location: /cgi/cookie_check_cgi")
        print("CGI_HEADERS_END")
        print()
        return

    list_files(username, upload_dir)


if __name__ == "__main__":
    main()


# if __name__ == "__main__":
#     list_files()
