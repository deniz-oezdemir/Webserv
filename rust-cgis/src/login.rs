use std::env;
use std::process;
use std::fs::{OpenOptions, File};
use std::io::{self, Read, Write, BufReader, BufWriter};
use std::io::BufRead;
use std::time::{SystemTime, UNIX_EPOCH};
use log::{debug, error, LevelFilter};
use log4rs::config::{Config, Appender, Root};
use log4rs::append::file::FileAppender;
use log4rs::encode::pattern::PatternEncoder;
use sha2::{Sha256, Digest};
use serde_json::{json, Value};

fn main() {
    // Initialize logging to a file
    let logfile = FileAppender::builder()
        .encoder(Box::new(PatternEncoder::new("{d} - {l} - {m}\n")))
        .build("/tmp/rust.log")
        .unwrap();

    let config = Config::builder()
        .appender(Appender::builder().build("logfile", Box::new(logfile)))
        .build(Root::builder().appender("logfile").build(LevelFilter::Debug))
        .unwrap();

    log4rs::init_config(config).unwrap();

    debug!("login.rs script started");

    // Check environment variables
    let method = env::var("REQUEST_METHOD").unwrap_or_default();
    let content_length = env::var("CONTENT_LENGTH").unwrap_or_default();
    let content_type = env::var("CONTENT_TYPE").unwrap_or_default();

    debug!("REQUEST_METHOD: {}", method);
    debug!("CONTENT_LENGTH: {}", content_length);
    debug!("CONTENT_TYPE: {}", content_type);

    // Ensure CONTENT_LENGTH is set
    if content_length.is_empty() {
        error!("CONTENT_LENGTH is not set");
        process::exit(1); // Exit with error status
    }

    // Read form data
    let content_length: usize = content_length.parse().unwrap_or(0);
    let mut input_data = vec![0; content_length];
    if let Err(e) = io::stdin().read_exact(&mut input_data) {
        error!("Error reading form data: {}", e);
        return;
    }
    debug!("Raw input data: {:?}", input_data);

    // Convert input data to string and log it
    let input_string = String::from_utf8_lossy(&input_data);
    debug!("Input data as string: '{}'", input_string);

    // Parse form data (simplified for demonstration)
    let form_data = input_string.trim(); // Remove leading/trailing whitespace
    let mut username = None;
    let mut password = None;
    for line in form_data.split('&') {
        let mut parts = line.split('=');
        match parts.next() {
            Some("username") => username = parts.next().map(|s| s.to_string()),
            Some("password") => password = parts.next().map(|s| s.to_string()),
            _ => {}
        }
    }

    debug!("Form data - username: {:?}, password: {:?}", username, password);

    // Generate token and save to db.txt
    if let (Some(username), Some(password)) = (username, password) {
        let hashed_password = hash_password(&password);
        debug!("Hashed password: {}", hashed_password);
        if let Some(_existing_token) = check_user_exists(&username, &hashed_password) {
            // User exists, generate a new token
            debug!("User exists, generating new token");
            let new_token = generate_token(&username, &password);
            debug!("New token generated: {}", new_token);
            update_token(&username, &new_token);
            send_token_to_client(&new_token);
            return;
        } else {
            // User does not exist, create a new entry
            debug!("User does not exist, creating new entry");
            let token = generate_token(&username, &password);
            debug!("New token generated: {}", token);
            save_to_db(&username, &hashed_password, &token);
            send_token_to_client(&token);
            return;
        }
    } else {
        println!("<html>");
        println!("<head>");
        println!("<title>Login</title>");
        println!("<style>");
        println!("body {{ text-align: center; }}");
        println!("</style>");
        println!("</head>");
        println!("<body>");
        println!("<p>Invalid login data.</p>");
        println!("<p><a href=\"http://localhost:8087/\">Go back</a></p>");
        println!("</body>");
        println!("</html>");
    }
}

fn generate_token(username: &str, password: &str) -> String {
    let timestamp = SystemTime::now().duration_since(UNIX_EPOCH).unwrap().as_secs();
    let mut hasher = Sha256::new();
    hasher.update(username);
    hasher.update(password);
    hasher.update(timestamp.to_string());
    format!("{:x}", hasher.finalize())
}

fn hash_password(password: &str) -> String {
    let mut hasher = Sha256::new();
    hasher.update(password);
    format!("{:x}", hasher.finalize())
}

fn save_to_db(username: &str, hashed_password: &str, token: &str) {
    let db_entry = json!({
        "username": username,
        "hashed_password": hashed_password,
        "token": token
    });

    let mut file = OpenOptions::new()
        .append(true)
        .create(true)
        .open("www/instagram-clone/db/db.txt")
        .unwrap();

    writeln!(file, "{}", db_entry.to_string()).unwrap();
    debug!("Saved to db: {}", db_entry.to_string());
}

fn check_user_exists(username: &str, hashed_password: &str) -> Option<String> {
    let file = File::open("www/instagram-clone/db/db.txt").unwrap();
    let reader = BufReader::new(file);

    for line in reader.lines() {
        let line = line.unwrap();
        let entry: Value = serde_json::from_str(&line).unwrap();
        if entry["username"] == username && entry["hashed_password"] == hashed_password {
            debug!("User found in db: {}", entry);
            return Some(entry["token"].as_str().unwrap().to_string());
        }
    }
    debug!("User not found in db");
    None
}

fn update_token(username: &str, new_token: &str) {
    let file = File::open("www/instagram-clone/db/db.txt").unwrap();
    let reader = BufReader::new(file);
    let mut entries: Vec<Value> = Vec::new();

    for line in reader.lines() {
        let line = line.unwrap();
        let mut entry: Value = serde_json::from_str(&line).unwrap();
        if entry["username"] == username {
            entry["token"] = json!(new_token);
            debug!("Updated token for user: {}", username);
        }
        entries.push(entry);
    }

    let file = File::create("www/instagram-clone/db/db.txt").unwrap();
    let mut writer = BufWriter::new(file);

    for entry in entries {
        writeln!(writer, "{}", entry.to_string()).unwrap();
        debug!("Written entry to db: {}", entry);
    }
}

fn send_token_to_client(token: &str) {
    println!("CGI_HEADERS");
    println!("Set-Cookie: token={}; Max-Age=86400; Path=/; HttpOnly", token);
    println!("Status: 302 Found");
    println!("Location: /home.py");
    println!("CGI_HEADERS_END");
    println!();
    debug!("Token sent to client: {}", token);
}
