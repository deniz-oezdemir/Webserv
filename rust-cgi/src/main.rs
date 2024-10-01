use std::io::{self, Error};

fn main() -> io::Result<()> {
    let args: Vec<String> = std::env::args().collect();

    if args.len() != 2 {
        return Err(Error::new(
            io::ErrorKind::InvalidInput,
            "Incorrect number of arguments to rust-cgi.",
        ));
    }

    let path = args[1].clone();

    // Check if the file exists
    if std::fs::metadata(&path).is_err() {
        eprintln!("File does not exist: {path}");
        return Err(Error::new(
            io::ErrorKind::NotFound,
            "File does not exist.",
        ));
    }

    match std::fs::remove_file(&path) {
        Ok(_) => {
            eprintln!("File deleted: {path}")
        }
        Err(_) => {
            eprintln!("File {path} could not be deleted")
        }
    };

    Ok(())
}
