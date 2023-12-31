# br Bulk Rename
br is a simple command-line utility written in C that allows you to rename multiple files and directories eficiently. It opens a list of files in your preferred text editor, enabling you to modify the names as you see fit. After editing, br renames the files according to your modifications.

## Usage
To use br, run the executable from the command line. If no arguments are provided, VimV will list files and directories in the current directory. If you provide file or directory names as arguments, VimV will use them instead.

./br [file1] [file2] ...
If you run br without any arguments, it will operate on all regular files and directories in the current directory.

## How it Works 
Listing Files: br first lists the files and directories either from the command-line arguments or from the current directory.

## Changing default Editor:
br relies on $EDITOR 

## Example
./br file1.txt file2.txt

## Notes
br skips files with names '.' and '..', as these represent the current and parent directories.
If a destination file already exists, br will skip renaming that particular file and proceed with the next one.
Error messages will be displayed if any issues occur during the process.

## Dependencies
br Standard C libraries and a text editor

## Build
./c 
./c install
./c remove

## Author
This program was created by Vulto.
Feel free to modify and distribute this code as per the license.
