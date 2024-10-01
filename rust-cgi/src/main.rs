use std::{
    env,
    io::{self, Read},
};

fn main() -> io::Result<()> {
    let vars = std::env::vars();
    for (key, value) in vars {
        if key == "CONTENT_LENGTH" {
            println!("{key}: {value}");
        }
    }

    // Read the CONTENT_LENGTH environment variable and convert it to an integer
    let length: usize = match env::var("CONTENT_LENGTH") {
        Ok(val) => match val.parse::<usize>() {
            Ok(num) => num,
            Err(_) => {
                eprintln!("Error: CONTENT_LENGTH is not a valid integer");
                return Err(io::Error::new(
                    io::ErrorKind::InvalidInput,
                    "Invalid CONTENT_LENGTH",
                ));
            }
        },
        Err(_) => {
            eprintln!("Error: CONTENT_LENGTH is not set");
            return Err(io::Error::new(
                io::ErrorKind::NotFound,
                "CONTENT_LENGTH not set",
            ));
        }
    };

    // Create a buffer to hold the input data
    let mut buffer = vec![0; length];

    // Read the exact number of bytes from stdin
    // Read the exact number of bytes from stdin
    if let Err(e) = io::stdin().read_exact(&mut buffer) {
        eprintln!("Error reading stdin: {}", e);
        return Err(e);
    }

    // Convert the buffer to a string and print it
    let input = String::from_utf8_lossy(&buffer);
    println!("Input was: {input}");

    // Print HTTP resonse
    print_response();

    Ok(())
}

fn print_response() {
    // Print the HTTP header
    println!("Content-Type: text/html\n");

    println!("<html>");
    println!("<head><title>CGI Response</title></head>");
    println!("<body>");
    println!("<h1>Hello from Rust CGI!</h1>");
    println!("<p>This is a simple CGI response.</p>");
    println!("</body>");
    println!("</html>");
}
