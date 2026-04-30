# Image Steganography using LSB Encoding and Decoding
## Overview
This project implements an image steganography system in C using Least Significant Bit (LSB) encoding. It allows users to securely hide a secret text file inside a BMP image and later retrieve it without affecting the visible quality of the image.

---

## Features
-	 Encode secret data into BMP images
-	 Decode hidden data from stego images
-	 Preserves original image appearance
-	 Embeds metadata (magic string, file size, extension)
-	 Command-line interface for easy usage

---

## How it Works
During encoding, the program first copies the first 54 bytes of BMP header to preserve the original image structure. A predefined magic string ("#*") is embedded during encoding to mark the image as a valid stego file. It proceeds to encode metadata such as the secret file’s extension and size, followed by the actual file content. Each bit of this information is stored in the least significant bits (LSB) of the image’s pixel data, ensuring that visual quality remains unchanged. 
During decoding, the program skips the header and verifies the magic string to confirm the presence of embedded data. It then sequentially extracts the metadata and reconstructs the original secret file by reading the encoded bits from the image.

---

##  Concepts Demonstrated
-	Bitwise operations (LSB manipulation)
-	File handling in C
-	BMP image structure handling
-	Command-line arguments (CLA)
-	Modular programming
  
---

## Project Structure
```bash
.
├── beautiful.bmp
├── common.h
├── decode.c
├── decode.h
├── encode.c
├── encode.h
├── makefile
├── secret.txt
├── test_encode.c
└── types.h
```

---

## Build Instructions
### Using Makefile
```bash
make
./stego
```
### Clean build files
```bash
make clean
```

---

## Usage
### Encoding
```bash
./stego -e sample/beautiful.bmp sample/secret.txt stego.bmp
```
### Decoding
```bash
./stego -d stego.bmp decoded.txt
```
