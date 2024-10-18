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
        print("    <meta name='viewport' content='width=device-width, initial-scale=1.0'>")
        print("    <title>Instaclone CGI</title>")
        print("    <link rel='stylesheet' href='css/styles.css'>")
        print("    <style>")
        print("        body {")
        print("            margin: 0;")
        print("            font-family: Arial, sans-serif;")
        print("            background-color: black;")  # Black background for the body
        print("            height: 100vh;")  # Full height for the body
        print("            display: flex;")
        print("            justify-content: center;")
        print("            align-items: center;")
        print("            color: white;")  # White text color
        print("            position: relative;")  # Relative positioning for the body
        print("        }")
        print("        .wrapper {")
        print("            background-color: black;")
        print("            width: 100%;")
        print("            height: 100%;")  # Full height for the wrapper
        print("            box-sizing: border-box;")
        print("            display: flex;")
        print("            flex-direction: column;")
        print("        }")
        print("        .top-bar {")
        print("            display: flex;")
        print("            align-items: center;")
        print("            justify-content: space-between;")
        print("            background-color: black;")  # Black background for the top bar
        print("            padding: 10px 20px;")
        print("            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);")
        print("            width: 100%;")
        print("            box-sizing: border-box;")
        print("            border-bottom: 1px solid #ccc;")  # Lower border for the top bar
        print("            color: white;")  # White text color for the top bar
        print("        }")
        print("        .top-bar .logo {")
        print("            display: flex;")
        print("            align-items: center;")
        print("        }")
        print("        .top-bar .logo h1 {")
        print("            margin: 0;")
        print("            color: white;")  # White text color for the logo
        print("        }")
        print("        .top-bar .menu {")
        print("            display: flex;")
        print("            gap: 20px;")
        print("        }")
        print("        .top-bar .menu a {")
        print("            color: white;")  # White text color for the menu links
        print("            text-decoration: none;")
        print("        }")
        print("        .container {")
        print("            width: 95%;")
        print("            max-width: 1200px;")  # Maximum width for the container
        print("            margin: 0 auto;")  # Center the container
        print("            box-sizing: border-box;")
        print("            padding: 20px;")
        print("            background-color: black;")
        print("            text-align: center;")  # Center the content
        print("            flex: 1;")  # Allow the container to grow and fill the remaining space
        print("        }")
        print("        .photo-grid {")
        print("            display: grid;")
        print("            grid-template-columns: repeat(3, 1fr);")  # Three-column layout
        print("            gap: 10px;")  # Gap between grid items
        print("        }")
        print("        .photo-grid img {")
        print("            max-width: 300%;")  # Maximum width for the image
        print("        }")
        print("        .photo-container {")
        print("            position: relative;")
        print("            width: 100%;")
        print("            padding-top: 100%;")  # Maintain aspect ratio
        print("            overflow: hidden;")
        print("        }")
        print("        .photo-container img {")
        print("            position: absolute;")
        print("            top: 0;")
        print("            left: 0;")
        print("            width: 100%;")
        print("            height: 100%;")
        print("            object-fit: cover;")  # Ensure the image covers the entire container
        print("        }")
        print("        .delete-form {")
        print("            position: absolute;")
        print("            top: 10px;")
        print("            right: 10px;")
        print("            display: none;")  # Hide delete button by default
        print("        }")
        print("        .photo-container:hover .delete-form {")
        print("            display: block;")  # Show delete button on hover
        print("        }")
        print("        .upload-container {")
        print("            display: flex;")
        print("            justify-content: center;")
        print("            align-items: center;")
        print("            cursor: pointer;")
        print("            height: 100%;")  # Make the upload container fill the grid cell
        print("            aspect-ratio: 1 / 1;")  # Ensure the container is square
        print("            box-sizing: border-box;")  # Include border in the element's total width and height
        print("        }")
        print("        .upload-container:hover {")
        print("            cursor: pointer;")  # Change cursor to pointer on hover
        print("        }")
        print("        .upload-icon {")
        print("            font-size: 48px;")
        print("            color: white;")  # White color for the upload icon
        print("            display: flex;")
        print("            justify-content: center;")
        print("            align-items: center;")
        print("            width: 100%;")
        print("            height: 100%;")
        print("        }")
        print("        .upload-form-container {")
        print("            display: flex;")
        print("            flex-direction: column;")
        print("            justify-content: center;")
        print("            align-items: center;")
        print("            margin-bottom: 20px;")  # Add some margin below the upload form
        print("        }")
        print("        .upload-form-container input, .upload-form-container button {")
        print("            margin: 5px 0;")  # Add some margin between the input and button
        print("        }")
        print("        .welcome-message {")
        print("            position: absolute;")
        print("            top: 50%;")
        print("            left: 50%;")
        print("            transform: translate(-50%, -50%);")  # Center the welcome message
        print("            background-color: rgba(0, 0, 0, 0.8);")  # Semi-transparent background
        print("            padding: 40px;")  # Increase padding for a bigger message box
        print("            border-radius: 10px;")
        print("            font-size: 24px;")  # Increase font size
        print("            z-index: 1000;")  # Ensure it appears above other elements
        print("        }")
        print("    </style>")
        print("</head>")
        print("<body>")
        print("    <div class='wrapper'>")
        print("        <div class='top-bar'>")
        print("            <div class='logo'>")
        print("                <h1>Instaclone</h1>")
        print("            </div>")
        print("            <div class='menu'>")
        print("                <a href='http://localhost:8086'>Home</a>")
        print("                <a href='http://localhost:8085'>Profile</a>")
        print("                <a href='http://localhost:8084'>Settings</a>")
        print("            </div>")
        print("        </div>")
        print("        <div class='welcome-message' id='welcome-message'>")
        print(f"            <h2>Welcome back, {username}</h2>")
        print("        </div>")
        print("        <div class='container'>")
        print("            <div class='upload-form-container'>")
        print("                <form id='upload-form' enctype='multipart/form-data' method='post' action='cgi/post.py'>")
        print("                    <input type='file' name='file' id='file-input' accept='image/*' required>")
        print("                    <button type='submit' id='upload-btn'>Post Photo</button>")
        print("                </form>")
        print("            </div>")
        print("            <div class='photo-grid' id='photo-grid'>")

        for filename in file_list:
            print("                <div class='photo-container'>")
            print(f"                    <img src='upload/{filename}' alt='Photo'>")
            print(f"                    <form class='delete-form' data-filename='{filename}'>")
            print("                        <button type='button' class='delete-btn'>Delete</button>")
            print("                    </form>")
            print("                </div>")

        print("            </div>")
        print("        </div>")
        print("    </div>")
        print("    <script>")
        print("        document.querySelectorAll('.delete-btn').forEach(button => {")
        print("            button.addEventListener('click', function() {")
        print("                const form = this.closest('.delete-form');")
        print("                const filename = form.getAttribute('data-filename');")
        print("                fetch(`cgi/delete_cgi?/upload/${filename}`, { method: 'DELETE' })")
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
        print("        // Hide the welcome message after 3 seconds")
        print("        setTimeout(() => {")
        print("            const welcomeMessage = document.getElementById('welcome-message');")
        print("            if (welcomeMessage) {")
        print("                welcomeMessage.style.visibility = 'hidden';")  # Hide the message but keep its space
        print("            }")
        print("        }, 3000);")
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
