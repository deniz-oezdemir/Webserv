use std::env;
use std::process;
use std::fs::{File};
use std::io::{BufReader};
use std::io::BufRead;
use serde_json::{Value};

fn main() {

    let method = env::var("REQUEST_METHOD").unwrap_or_default();
    let root_dir = env::var("ROOT_DIR").unwrap_or_default();
    let cookies = env::var("COOKIE").unwrap_or_default();

    if cookies.is_empty() { 
        print_login();
        return;
    }
    if root_dir.is_empty() {
        process::exit(1); // Exit with error status
    }
    if method.is_empty() || method != "GET" {
        process::exit(1); // Exit with error status
    }

    if check_token_exists(&cookies, &root_dir) {
        print_redirect();
    }
    else {
        print_login();
    }
}


fn check_token_exists(token: &str, root_dir: &str) -> bool {
    let file = File::open(root_dir.to_owned() + "/db/db.json").unwrap();
    let reader = BufReader::new(file);

    for line in reader.lines() {
        let line = line.unwrap();
        let entry: Value = serde_json::from_str(&line).unwrap();
        if entry["token"] == token {
            return true;
        }
    }
    false
}

fn print_redirect() {
    println!("CGI_HEADERS");
    println!("Status: 302");
    println!("Location: /home.py");
    println!("CGI_HEADERS_END");
    println!();
}

fn print_login() {
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
    println!("<form method=\"POST\" action=\"/cgi/login_cgi\">");
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

// fn main() -> io::Result<()> {
//     // Get the COOKIE environment variable
//     let cookie_str: String = match std::env::var("COOKIE") {
//         Ok(val) => val,
//         Err(_) => {
//             print_login();
//             return Ok(());
//         }
//     };

//     let cookie_value: Cookie = match Cookie::parse(cookie_str) {
//         Ok(val) => val,
//         Err(_) => {
//             print_login();
//             return Ok(());
//         }
//     };

//     // Read cookies file and turn it to map
//     let cookies_file: String = fs::read_to_string("./www/instagram-clone/cookies/cookies.txt")?;

//     let mut cookies_map = HashMap::new();
//     for line in cookies_file.lines() {
//         let parts: Vec<&str> = line.split(':').collect();
//         if parts.len() == 2 {
//             cookies_map.insert(parts[0].to_string(), parts[1].to_string());
//         }
//     }

//     // Search for cookie in map
//     if cookies_map.contains_key(&cookie_value.to_string()) {
//         println!("Content-Type: text/plain");
//         println!();
//         println!("Cookie matches key!");
//     } else {
//         print_login();
//     }

//     Ok(())
// }
