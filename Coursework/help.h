#pragma once
#define HELP_TEXT "\
Course work created by Ulanovsky George and Lyashkova Snejana\n\n\
usage: Coursework [option] ... [arg] ...\n\
note: all files must be in .txt format\n\
Options and arguments (and corresponding environment variables):\n\
genkey	           : Generates keys with const size.                      Needs --size,   --pubkey, --secret   options.\n\
sign               : Finds file signature and saves it.                   Needs --infile, --secret, --sigfile  options.\n\
check              : Checks file signature and types result into console. Needs --infile, --pubkey, --sigfile  options.\n\
encrypt            : Encrypt file using key.                              Needs --infile, --pubkey, --outfile  options.\n\
decrypt            : Decrypt file using key.                              Needs --infile, --secret, --outfile  options.\n\
--size size        : Setup size for the key in bits. Must be a degree of 2 (between 256 and 2048).\n\
--pubkey filename  : Setup a filename where will be saved public key.\n\
--secret filename  : Setup a filename where will be saved secret key.\n\
--sigfile filename : Setup a filename where will be saved or checked signature.\n\
--infile filename  : Setup a filename where is saved input information.\n\
--outfile filename : Setup a filename where will be saved output information.\n\
-h                 : Print this help message and exit (also --help) \n"

#define ERROR_FIRST_PARAMETER "\
Unknown option : %s\n\
usage: Coursework [option] ... [arg] ...\n\
Try `Coursework - h' for more information.\n"

#define ERROR_NO_PARAMETERS "\
You had typed no options\n\
usage: Coursework [option] ... [arg] ...\n\
Try `Coursework - h' for more information.\n"

#define WRONG_PARAMETER "\
You had typed unknown option: %s\n\
Expected option: %s\n\
usage: Coursework [option] ... [arg] ...\n\
Try `Coursework - h' for more information.\n"

#define ERROR_NOT_ENOUGH_PARAMETERS "\
You had typed not enough options\n\
usage: Coursework [option] ... [arg] ...\n\
Try `Coursework - h' for more information.\n"

#define ERROR_DEFAULT "\
You had typed wrong options\n\
usage: Coursework [option] ... [arg] ...\n\
Try `Coursework - h' for more information.\n"

#define ERROR_FILE_IDENT "\
You entered same filenames\n\
usage: Coursework [option] ... [arg] ...\n\
Try `Coursework - h' for more information.\n"

#define ERROR_FILE_EXTENSION "\
You entered wrong filename (symbols count or extension)\n\
usage: Coursework [option] ... [arg] ...\n\
Try `Coursework - h' for more information.\n"