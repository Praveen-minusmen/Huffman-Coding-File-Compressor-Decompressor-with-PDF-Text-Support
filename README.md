# Huffman-Coding-File-Compressor-Decompressor-with-PDF-Text-Support
A C program that compresses and decompresses files using Huffman coding. Supports direct text files and PDF/Word document conversion to text for compression. Includes both command-line interaction and file handling for efficient storage and retrieval.

## Features

- **Huffman Coding:** Efficient lossless compression using Huffman trees.
- **PDF/Word Support:** Converts PDF and Word files to text before compression (requires `pdftotext` and `libreoffice`).
- **Custom File Header:** Stores Huffman codes and padding info for accurate decompression.
- **Interactive CLI:** Choose between compress, decompress, or both.
- **Cross-Platform:** Written in standard C, with external tool dependencies for document conversion.

## Requirements

- GCC or any C compiler
- For PDF support: [`pdftotext`](https://www.xpdfreader.com/pdftotext-man.html)
- For Word support: [`libreoffice`](https://www.libreoffice.org/) (for `.doc`/`.docx` to text conversion)
- Unix-like environment (for `popen` and shell commands)

## Usage

1. **Compile the program:**
   ```sh
   gcc -o huffman_compress hf\ pdf.c
   ```

2. **Run the program:**
   ```sh
   ./huffman_compress
   ```

3. **Follow the prompts:**
   - Choose to compress, decompress, or both.
   - Enter the file name as prompted.

4. **Output:**
   - Compressed files will be named `<input>_compressed`
   - Decompressed files will be named `<input>_decompressed`

## Example

```sh
$ ./huffman_compress
Choose an option:
1. Compress and Decompress
2. Compress Only
3. Decompress Only
Enter your choice (1/2/3): 1
Enter the input file name: sample.pdf
Compression and decompression completed successfully!
```

## Notes

- For PDF/Word files, the program converts them to text before compression.
- Ensure `pdftotext` and `libreoffice` are installed and available in your system's PATH.
- The program is designed for educational purposes and may require adaptation for large files or production use.

## License

This project is open-source and available under the MIT License.

---

**Author:**  
[Praveen K]
