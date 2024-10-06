use std::io::{self, Error};
use std::path::{PathBuf};

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
    let target_file = match std::env::var("TARGET_FILE") {
        Ok(val) => val,
        Err(_) => {
            return Err(Error::new(
                io::ErrorKind::InvalidInput,
                "Environment variable TARGET_FILE is not set.",
            ));
        }
    };

    // Construct the full path
    let path = PathBuf::from(root.clone() + &target_file);

    // Check if the file exists
    if !path.exists() {
        eprintln!("File does not exist: {} {} {}", root, target_file, path.display());
        return Err(Error::new(
            io::ErrorKind::NotFound,
            "File does not exist.",
        ));
    }

    // Attempt to delete the file
    match std::fs::remove_file(&path) {
        Ok(_) => {
            return Ok(());
        }
        Err(e) => {
            eprintln!("File {} could not be deleted: {}", path.display(), e);
            return Err(e);
        }
    };
}
