use std::env;
use std::io::{self, Read};
use log::{debug, error, LevelFilter};
use log4rs::config::{Config, Appender, Root};
use log4rs::append::file::FileAppender;
use log4rs::encode::pattern::PatternEncoder;

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
        print_error_page("Error: CONTENT_LENGTH is not set.");
        return;
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

    // Print HTML response
    println!("Content-Type: text/html\n");
    println!("<html>");
    println!("<head>");
    println!("<title>Login</title>");
    println!("<style>");
    println!("body {{ text-align: center; }}");
    println!("</style>");
    println!("</head>");
    println!("<body>");
    if let (Some(username), Some(password)) = (username, password) {
        debug!("Username: {}, Password: {}", username, password);
        println!("<p>Login received for username: {}</p>", username);
    } else {
        println!("<p>Invalid login data.</p>");
    }
    println!("<p><a href=\"http://localhost:8087/\">Go back</a></p>");
    println!("</body>");
    println!("</html>");
}

fn print_error_page(message: &str) {
    println!("Content-Type: text/html\n");
    println!("<html>");
    println!("<head>");
    println!("<title>Error</title>");
    println!("<style>");
    println!("body {{ text-align: center; }}");
    println!("</style>");
    println!("</head>");
    println!("<body>");
    println!("<p>{}</p>", message);
    println!("</body>");
    println!("</html>");
}
