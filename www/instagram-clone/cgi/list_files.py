#!/usr/bin/env python3

import os
import cgi

def list_files():
    try:
        # Get the directory of the current script
        script_dir = os.path.dirname(os.path.abspath(__file__))
        upload_dir = os.path.join(script_dir, '../upload')

        # List files in the upload directory
        files = os.listdir(upload_dir)
        file_list = [f for f in files if os.path.isfile(os.path.join(upload_dir, f))]

        # Generate HTML content
        print("Content-Type: text/html")
        print()
        print("<!DOCTYPE html>")
        print("<html lang='en'>")
        print("<head>")
        print("    <meta charset='UTF-8'>")
        print("    <meta name='viewport' content='width=device-width, initial-scale=1.0'>")
        print("    <title>Instagram Clone</title>")
        print("    <link rel='stylesheet' href='../styles.css'>")
        print("</head>")
        print("<body>")
        print("    <h1>Instagram Clone</h1>")
        print("    <div class='container'>")
        print("        <div class='photo-grid' id='photo-grid'>")

        for filename in file_list:
            print("            <div class='photo-container'>")
            print(f"                <img src='../upload/{filename}' alt='Photo'>")
            print(f"                <form method='post' action='../upload/{filename}'>")
            print("                    <button type='submit'>Delete</button>")
            print("                </form>")
            print("            </div>")

        print("        </div>")
        print("    </div>")
        print("    <div class='container'>")
        print("        <form id='upload-form' enctype='multipart/form-data' method='post' action='post.py'>")
        print("            <input type='file' name='file' id='file-input' accept='image/*' required>")
        print("            <button type='submit' id='upload-btn'>Post Photo</button>")
        print("        </form>")
        print("    </div>")
        print("    <div class='container'>")
        print("        <form id='get-time-form' method='get' action='get.py'>")
        print("            <button type='submit' id='get-time-btn'>Get Time</button>")
        print("        </form>")
        print("    </div>")
        print("</body>")
        print("</html>")

    except Exception as e:
        print("Content-Type: text/html")
        print()
        print("<html><body>")
        print(f"<h1>Error: {str(e)}</h1>")
        print("</body></html>")

if __name__ == "__main__":
    list_files()

