# TODO: Deniz delete logging

import os
import cgi
import logging
from io import BytesIO
import uuid  # Add this import at the top of your script

# Configure logging
logging.basicConfig(filename='/tmp/post_debug.log', level=logging.DEBUG)

logging.debug("post.py script started")

# Check environment variables
method = os.environ.get("REQUEST_METHOD", "")
content_length = os.environ.get("CONTENT_LENGTH", "")
content_type = os.environ.get("CONTENT_TYPE", "")
upload_path = os.environ.get("UPLOAD_PATH", "../upload/")

logging.debug(f"REQUEST_METHOD: {method}")
logging.debug(f"CONTENT_LENGTH: {content_length}")
logging.debug(f"CONTENT_TYPE: {content_type}")
logging.debug(f"UPLOAD_PATH: {upload_path}")


# Ensure CONTENT_LENGTH is set
if not content_length:
	logging.error("CONTENT_LENGTH is not set")
	print("<html>")
	print("<head>")
	print("<title>Error</title>")
	print("<style>")
	print("body { text-align: center; }")
	print("</style>")
	print("</head>")
	print("<body>")
	print("<p>Error: CONTENT_LENGTH is not set.</p>")
	print("</body>")
	print("</html>")
	exit(1)

# Read form data
try:
	# Log the raw input data
	input_data = os.read(0, int(content_length))
	logging.debug(f"Raw input data: {input_data}")

	# Use BytesIO to simulate a file-like object for FieldStorage
	input_stream = BytesIO(input_data)
	form = cgi.FieldStorage(fp=input_stream, environ=os.environ, keep_blank_values=True)
	logging.debug(f"Form keys: {form.keys()}")
except Exception as e:
	logging.error(f"Error reading form data: {e}")

logging.debug("post.py part 1 passed")

print("<html>")
print("<head>")
print("<title>File Upload</title>")
print("<style>")
print("body { text-align: center; }")
print("</style>")
print("</head>")
print("<body>")

logging.debug("post.py part 2 passed")

if method == "POST" and "file" in form:
	file_item = form["file"]
	if file_item.file:
		filename = f"{uuid.uuid4().hex}.jpg" # for now save as photo2.jpg for testing
		current_directory = os.getcwd()
		logging.debug(f"Current directory: {current_directory}")
		os.makedirs(upload_path, exist_ok=True)  # Ensure the upload directory exists
		with open(os.path.join(upload_path, filename), 'wb') as f:
			f.write(file_item.file.read())
		logging.debug(f"File {filename} uploaded successfully")
		print("<p>File uploaded successfully.</p>")
	else:
		logging.debug("No file was uploaded.")
		print("<p>No file was uploaded.</p>")
else:
	logging.debug("No file was uploaded.")
	print("<p>No file was uploaded.</p>")

logging.debug("post.py part 3 passed")

print("<p><a href=\"http://localhost:8087/home.py\">Go back</a></p>")
print("</body>")
print("</html>")

logging.debug("post.py finished")
