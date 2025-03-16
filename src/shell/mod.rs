use anyhow::{bail, Context, Result};
use reqwest::{
    self,
    header::{self, HeaderMap},
};
use serde::{Deserialize, Serialize};

use rustyline::error::ReadlineError;
use rustyline::DefaultEditor;

pub fn shell() -> Result<()> {
    // `()` can be used when no completer is required
    let mut rl = DefaultEditor::new()?;
    let history_file = "history.txt";
    if rl.load_history(history_file).is_err() {
        println!("No previous history.");
    }
    loop {
        let readline = rl.readline(">> ");

        // Extract a valid "command"
        let cmd = match readline {
            Ok(line) => {
                rl.add_history_entry(line.as_str())?;
                line
            }
            Err(ReadlineError::Interrupted) => {
                println!("CTRL-C");
                break;
            }
            Err(ReadlineError::Eof) => {
                println!("CTRL-D");
                break;
            }
            Err(err) => {
                println!("Error: {:?}", err);
                break;
            }
        };

        match cmd {
            _ => todo!(),
        }
    }
    rl.save_history(history_file)?;

    Ok(())
}
