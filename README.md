Treasure Hunt Manager
=====================

A simple terminal-based C application that lets you manage digital treasure hunts.
You can add treasures, view them, list all entries, and reset hunts.

Perfect for learning file handling, structs, and system-level operations in C!

---------------------
FEATURES
---------------------
- Add treasures with metadata (ID, user, location, clue, value)
- View treasure by ID
- List all treasures in a hunt
- Remove individual treasures or entire hunts
- Log every action per hunt
- Reset all hunt data securely (password-protected)


---------------------
COMPILATION
---------------------
gcc -Wall -o treasure_manager treasure_manager.c


---------------------
USAGE
---------------------
./treasure_manager --add <hunt_id>
./treasure_manager --list <hunt_id>
./treasure_manager --view <hunt_id> <treasure_id>
./treasure_manager --remove_treasure <hunt_id> <treasure_id>
./treasure_manager --remove_hunt <hunt_id>
./treasure_manager --reset <password>

NOTE: Default reset password is: 123456


---------------------
RESET FUNCTIONALITY
---------------------
The --reset command deletes:
- All treasure logs (hunt.log)
- All treasure files (treasure.dat)
- All temporary files (temp.dat)
- All symbolic links (logged_hunt-*)
- All hunt directories


---------------------
FILE STRUCTURE
---------------------
Each hunt has its own directory containing:
- hunt.log        => Action log
- treasure.dat    => Binary treasure data
- temp.dat        => Temporary file used during treasure removal

The app also creates a symlink in the root:
- logged_hunt-<hunt_id> -> points to the hunt.log file


---------------------
BUILT WITH
---------------------
- C (Standard Library)
- Unix system calls:
    open(), read(), write(), unlink(), mkdir(), symlink(), stat()
- File I/O and directory manipulation
