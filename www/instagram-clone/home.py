#!/usr/bin/env python3

import os
import cgi

def list_files():
    try:
        # Get the directory of the current script
        script_dir = os.path.dirname(os.path.abspath(__file__))
        upload_dir = os.path.join(script_dir, 'upload')

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
        print("    <title>Instagram Clone CGI</title>")
        print("    <link rel='stylesheet' href='css/styles.css'>")
        print("</head>")
        print("<body>")
        print("    <h1>Instagram Clone</h1>")
        print("    <div class='container'>")
        print("        <div class='photo-grid' id='photo-grid'>")

        for filename in file_list:
            print("            <div class='photo-container'>")
            print(f"                <img src='upload/{filename}' alt='Photo'>")
            print(f"                <form class='delete-form' data-filename='{filename}'>")
            print("                    <button type='button' class='delete-btn'>Delete</button>")
            print("                </form>")
            print("            </div>")

        print("        </div>")
        print("    </div>")
        print("    <div class='container'>")
        print("        <form id='upload-form' enctype='multipart/form-data' method='post' action='cgi/post.py'>")
        print("            <input type='file' name='file' id='file-input' accept='image/*' required>")
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
        print("    </script>")
        print("</body>")
        print("</html>")

    except Exception as e:
        print("<html><body>")
        print(f"<h1>Error: {str(e)}</h1>")
        print("</body></html>")

if __name__ == "__main__":
    list_files()
