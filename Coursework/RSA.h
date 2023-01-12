#ifndef RSA_H
#define RSA_H

#include "_file_.h"

/// <summary>
/// Function, checking sign of file
/// </summary>
/// <param name="input_filename">char array contains filename</param>
/// <param name="pubkey_filename">char array contains filename</param>
/// <param name="sign_filename">char array contains filename</param>
/// <returns>1 if sign is correct, else 0 </returns>
BOOL check_sign_file(char *input_filename, char *pubkey_filename, char *sign_filename);

/// <summary>
/// Function, creating a sign file to a input file
/// </summary>
/// <param name="input_filename">char array contains filename</param>
/// <param name="seckey_filename">char array contains filename</param>
/// <param name="sign_filename">char array contains filename</param>
void signify_file(char *input_filename, char *seckey_filename, char *sign_filename);

/// <summary>
/// Function, creating decrypted file of input file with key
/// </summary>
/// <param name="input_filename">char array contains filename</param>
/// <param name="seckey_filename">char array contains filename</param>
/// <param name="output_filename">char array contains filename</param>
void decrypt_file(char *input_filename, char *seckey_filename, char *output_filename);

/// <summary>
/// Function, creating encrypted file of input file with key
/// </summary>
/// <param name="input_filename">char array contains filename</param>
/// <param name="pubkey_filename">char array contains filename</param>
/// <param name="output_filename">char array contains filename</param>
void encrypt_file(char *input_filename, char *pubkey_filename, char *output_filename);

/// <summary>
/// Function, generating a pair of keys to en/decrypt
/// </summary>
/// <param name="key_size_str">char array contains bit size of keys</param>
/// <param name="pubkey_filename">char array contains filename</param>
/// <param name="seckey_filename">char array contains filename</param>
void generate_key(char *key_size_str, char *pubkey_filename, char *seckey_filename);
#endif // !RSA_H