use std::{env, io::{self, Read}};

fn main() -> io::Result<()> {
    let vars = std::env::vars();
    for (key, value) in vars {
        if key == "CONTENT_LENGTH" {
            println!("{key}: {value}");
        }
    }

    let length: usize = env::var("CONTENT_LENGTH")
        .unwrap_or_else(|_| "0".to_string())
        .parse()
        .expect("Cannot transfor to integer");

    // Create a buffer to hold the input data
    let mut buffer = vec![0; length];

    // Read the exact number of bytes from stdin
    io::stdin().read_exact(&mut buffer)?;

    // Convert the buffer to a string and print it
    let input = String::from_utf8_lossy(&buffer);
    println!("Input was: {input}");

    Ok(())
}
