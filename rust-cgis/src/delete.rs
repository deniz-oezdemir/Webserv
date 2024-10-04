use std::io::{self, Error};

fn main() -> io::Result<()> {

    // Get the DELETE_FILE environment variable
    let root = match std::env::var("ROOT_DIR") {
        Ok(val) => val,
        Err(_) => {
            return Err(Error::new(
                io::ErrorKind::InvalidInput,
                "Environment variable ROOT_DIR is not set.",
            ));
        }
    };

    // Get the TARGET_FILE environment variable
    let mut path = match std::env::var("TARGET_FILE") {
        Ok(val) => val,
        Err(_) => {
            return Err(Error::new(
                io::ErrorKind::InvalidInput,
                "Environment variable TARGET_FILE is not set.",
            ));
        }
    };

    path = root + &path;

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

