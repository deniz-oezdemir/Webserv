import os
import cgi
import cgitb
import logging

# Enable CGI traceback
cgitb.enable()

# Configure logging
logging.basicConfig(filename='/tmp/post_debug.log', level=logging.DEBUG)

logging.debug("post.py script started")

form = cgi.FieldStorage()  # FieldStorage() reads form data from std in but in our case needs data from request?
method = os.environ.get("REQUEST_METHOD", "")
upload_path = os.environ.get("PATH_INFO", "../upload/")

print("Content-Type: text/html")
print()

print("<html>")
print("<head>")
print("<title>File Upload</title>")
print("</head>")
print("<body>")

if method == "POST" and "file" in form:
	file_item = form["file"]
	if file_item.file:
		filename = "photo2.jpg" #for now save as photo2.jpg for testing #os.path.basename(file_item.filename)
		with open(os.path.join(upload_path, filename), "wb") as f:
			f.write(file_item.file.read())
		print(f"<p>post.py: File'<b>{filename}</b>' uploaded successfully.</p>")
	else:
		print("<p>post.py: Failed to upload file.</p>")
else:
	print("<p>post.py: No file was uploaded.</p>")

print("<p><a href=\"/instagram-clone/index.html\">Go back</a></p>")
print("</body>")
print("</html>")
