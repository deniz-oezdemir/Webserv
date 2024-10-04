use cookie::Cookie;
use std::collections::HashMap;
use std::fs;
use std::io::{self, Error};

fn main() -> io::Result<()> {
    // Get the COOKIE environment variable
    let cookie_str: String = match std::env::var("COOKIE") {
        Ok(val) => val,
        Err(_) => {
            return Err(Error::new(
                io::ErrorKind::InvalidInput,
                "Environment variable COOKIE is not set.",
            ));
        }
    };

    let cookie_value: Cookie = match Cookie::parse(cookie_str) {
        Ok(val) => val,
        Err(_) => {
            return Err(Error::new(
                io::ErrorKind::InvalidData,
                "Cookie could not be parsed correctly.",
            ))
        }
    };

    // Read cookies file and turn it to map
    let cookies_file: String = fs::read_to_string("../../cookies/cookies.txt")?;

    let mut cookies_map = HashMap::new();
    for line in cookies_file.lines() {
        let parts: Vec<&str> = line.split(':').collect();
        if parts.len() == 2 {
            cookies_map.insert(parts[0].to_string(), parts[1].to_string());
        }
    }

    // Search for cookie in map
    if cookies_map.contains_key(&cookie_value.to_string()) {
        println!("Content-Type: text/plain");
        println!();
        println!("Cookie matches key!");
    } else {
        println!("Content-Type: text/html");
        println!();
        println!("<html>");
        println!("<head>");
        println!("<style>");
        println!("body {{");
        println!("    font-family: Arial, sans-serif;");
        println!("    background-color: #f2f2f2;");
        println!("    display: flex;");
        println!("    justify-content: center;");
        println!("    align-items: center;");
        println!("    height: 100vh;");
        println!("    margin: 0;");
        println!("}}");
        println!("h1 {{");
        println!("    color: #333;");
        println!("}}");
        println!(".login-container {{");
        println!("    background-color: #fff;");
        println!("    padding: 20px;");
        println!("    border-radius: 8px;");
        println!("    box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);");
        println!("}}");
        println!("label {{");
        println!("    display: block;");
        println!("    margin-bottom: 8px;");
        println!("    color: #555;");
        println!("}}");
        println!("input[type='text'], input[type='password'] {{");
        println!("    width: 100%;");
        println!("    padding: 10px;");
        println!("    margin-bottom: 20px;");
        println!("    border: 1px solid #ccc;");
        println!("    border-radius: 4px;");
        println!("}}");
        println!("input[type='submit'] {{");
        println!("    width: 100%;");
        println!("    padding: 10px;");
        println!("    background-color: #4CAF50;");
        println!("    color: white;");
        println!("    border: none;");
        println!("    border-radius: 4px;");
        println!("    cursor: pointer;");
        println!("}}");
        println!("input[type='submit']:hover {{");
        println!("    background-color: #45a049;");
        println!("}}");
        println!("</style>");
        println!("</head>");
        println!("<body>");
        println!("<div class=\"login-container\">");
        println!("<h1>Login</h1>");
        println!("<form method=\"POST\" action=\"/www/instagram-clone/cgi/login.rs\">");
        println!("<label for=\"username\">Username:</label>");
        println!("<input type=\"text\" id=\"username\" name=\"username\">");
        println!("<label for=\"password\">Password:</label>");
        println!("<input type=\"password\" id=\"password\" name=\"password\">");
        println!("<input type=\"submit\" value=\"Login\">");
        println!("</form>");
        println!("</div>");
        println!("</body>");
        println!("</html>");
    }

    Ok(())
}
