import warnings
warnings.filterwarnings("ignore", category=DeprecationWarning)

import os
import cgi
import logging
from io import BytesIO
import uuid
from PIL import Image

# Configure logging
logging.basicConfig(filename="/tmp/post_debug.log", level=logging.DEBUG)


def log_environment_variables():
    """Log the environment variables."""
    method = os.environ.get("REQUEST_METHOD", "")
    content_length = os.environ.get("CONTENT_LENGTH", "")
    content_type = os.environ.get("CONTENT_TYPE", "")
    upload_path = os.environ.get("UPLOAD_PATH", "../upload/")

    logging.debug(f"REQUEST_METHOD: {method}")
    logging.debug(f"CONTENT_LENGTH: {content_length}")
    logging.debug(f"CONTENT_TYPE: {content_type}")
    logging.debug(f"UPLOAD_PATH: {upload_path}")

    return method, content_length, content_type, upload_path


def ensure_content_length(content_length):
    """Ensure CONTENT_LENGTH is set."""
    if not content_length:
        logging.error("CONTENT_LENGTH is not set")
        print_html_error("Error: CONTENT_LENGTH is not set.", 400)
        exit(1)


def print_html_error(message, status_code):
    """Print an HTML error message with a specific status code."""
    print(f"Status: {status_code}")
    print("Content-Type: text/html")
    print()
    print("<html>")
    print("<head>")
    print("<title>Error</title>")
    print("<link rel='stylesheet' href='/instagram-clone/css/styles.css'>")
    print("</head>")
    print("<body>")
    print("    <div class='container'>")
    print(f"        <h1>{message}</h1>")
    print("    </div>")
    print("</body>")
    print("</html>")


def read_all_input(content_length):
    input_data = b""
    while len(input_data) < content_length:
        chunk = os.read(0, content_length - len(input_data))
        if not chunk:
            break  # End of input stream
        input_data += chunk
    return input_data


def read_form_data(content_length):
    """Read and return form data."""
    try:
        content_length = int(os.environ.get("CONTENT_LENGTH", 0))
        input_data = read_all_input(content_length)

        input_stream = BytesIO(input_data)

        logging.debug(f"Raw input data: {input_data}")
        logging.debug(f"Raw input data length: {len(input_data)}")
        logging.debug(f"Input stream: {input_stream}")
        # logging.debug(f"Input stream len: {len(input_stream)}")

        form = cgi.FieldStorage(
            fp=input_stream, environ=os.environ, keep_blank_values=True
        )
        logging.debug(f"Form keys: {form.keys()}")
        return form
    except Exception as e:
        logging.error(f"Error reading form data: {e}")
        print_html_error("Error reading form data.", 400)
        exit(1)


def validate_and_save_file(file_item, upload_path):
    """Validate the file and save it if valid."""
    if not file_item.filename.lower().endswith(".jpg"):
        logging.debug("Incorrect file format. Only .jpg files are accepted.")
        print("    <div class='container'>")
        print("        <h1>415 Unsupported Media Type</h1>")
        print("        <p>Only .jpg files are accepted.</p>")
        print("    </div>")
        return

    try:
        image = Image.open(file_item.file)
        image.verify()
        file_item.file.seek(0)

        filename = f"{uuid.uuid4().hex}.jpg"
        os.makedirs(upload_path, exist_ok=True)
        with open(os.path.join(upload_path, filename), "wb") as f:
            f.write(file_item.file.read())
        logging.debug(f"File {filename} uploaded successfully")
        print("    <div class='container'>")
        print("        <h1>File uploaded successfully.</h1>")
        print("    </div>")
    except (IOError, SyntaxError) as e:
        logging.debug(f"File is not a valid image: {e}")
        print("    <div class='container'>")
        print("        <h1>400 Bad request</h1>")
        print("        <p>Only valid image files are accepted.</p>")
        print("    </div>")


def main():
    logging.debug("post.py script started")

    method, content_length, content_type, upload_path = log_environment_variables()
    ensure_content_length(content_length)
    form = read_form_data(content_length)

    print("<!DOCTYPE html>")
    print("<html lang='en'>")
    print("<head>")
    print("    <meta charset='UTF-8'>")
    print("    <meta name='viewport' content='width=device-width, initial-scale=1.0'>")
    print("    <title>Post response</title>")
    print("    <style>")
    print("        body {")
    print("            font-family: Arial, sans-serif;")
    print("            background-color: black;")  # Black background for the body
    print("            color: white;")  # White text color
    print("            margin: 0;")
    print("            padding: 0;")
    print("            display: flex;")
    print("            justify-content: center;")
    print("            align-items: center;")
    print("            flex-direction: column;")
    print("            height: 100vh;")
    print("        }")
    print("        .container {")
    print("            background-color: black;")  # Black background for the container
    print("            color: white;")  # White text color for the container
    print("            padding: 20px;")
    print("            border-radius: 8px;")
    print("            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);")
    print("            margin-bottom: 20px;")
    print("            text-align: center;")
    print("            width: 80%;")
    print("            max-width: 600px;")
    print("        }")
    print("        h1 {")
    print("            margin-bottom: 20px;")
    print("        }")
    print("        h2 {")
    print("            margin: 20px 0;")
    print("        }")
    print("        button {")
    print("            background-color: #007bff;")
    print("            color: #fff;")
    print("            border: none;")
    print("            padding: 10px 20px;")
    print("            border-radius: 4px;")
    print("            cursor: pointer;")
    print("        }")
    print("        button:hover {")
    print("            background-color: #0056b3;")
    print("        }")
    print("        a {")
    print("            color: #007bff;")
    print("            text-decoration: none;")
    print("        }")
    print("        a:hover {")
    print("            text-decoration: underline;")
    print("        }")
    print("    </style>")
    print("</head>")
    print("<body>")

    if method == "POST" and "file" in form:
        file_item = form["file"]
        if file_item.file:
            validate_and_save_file(file_item, upload_path)
        else:
            logging.debug("No file was uploaded.")
            print("    <div class='container'>")
            print("        <h1>No file was uploaded.</h1>")
            print("    </div>")
    else:
        logging.debug("No file was uploaded.")
        print("    <div class='container'>")
        print("        <h1>No file was uploaded.</h1>")
        print("    </div>")

    print("    <div class='container'>")
    print("        <a href='/index.html'>Go back</a>")
    print("    </div>")
    print("</body>")
    print("</html>")

    logging.debug("post.py finished")


if __name__ == "__main__":
    main()
