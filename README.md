# 🗜️ Huffman Compressor

> Lossless file compression using Huffman Encoding — runs entirely in your browser.

A complete implementation of the Huffman coding algorithm with both a **browser-based web app** and a **C++ command-line tool**. Built as a Data Structures & Algorithms project.

---

## ✨ Features

- **Compress** any file type — text, images, binaries
- **Decompress** `.huff` files back to original (lossless)
- **Bit Comparison** — visual side-by-side of original vs compressed binary data
- **Huffman Code Table** — shows character → variable-length code mappings with bits saved
- **Byte Frequency Chart** — bar chart of the top 64 most frequent byte values
- **Compression Stats** — original size, output size, and reduction ratio
- **Download** compressed/decompressed output directly from browser
- **No server, no upload** — everything runs client-side in JavaScript

---

## 📁 Project Structure

```
huffman/
├── index.html    # Web app (single-file, open in browser)
├── main.cpp      # C++ CLI implementation
└── README.md     # This file
```

---

## 🚀 How to Run

### Option 1: Web App (No Setup Required)

Simply open `index.html` in any modern browser:

```bash
# macOS
open index.html

# Linux
xdg-open index.html

# Windows
start index.html
```

Or just **double-click** the file in your file manager.

**Usage:**
1. Select **Compress** or **Decompress** mode
2. Drag & drop a file (or click to browse)
3. Click the run button
4. View the **bit comparison**, **Huffman code table**, and **stats**
5. Download the output file

---

### Option 2: C++ Command Line

**Compile:**

```bash
g++ -std=c++17 -O2 -o huffman main.cpp
```

**Compress a file:**

```bash
./huffman c input.txt output.huff
```

**Decompress a file:**

```bash
./huffman d output.huff restored.txt
```

**Output example:**

```
Compressed:  input.txt -> output.huff
Original:    12048 bytes
Compressed:  7231 bytes
Reduction:   39.98%
```

---

## 🧠 How It Works

### Algorithm

1. **Frequency Analysis** — Count occurrences of each byte in the input
2. **Build Huffman Tree** — Use a min-heap (priority queue) to build a binary tree where frequently-used bytes get shorter codes
3. **Generate Codes** — Traverse the tree to assign variable-length binary codes to each byte
4. **Encode** — Replace each byte with its Huffman code
5. **Serialize** — Write the tree structure + encoded data to output

### File Format (`.huff`)

```
[8 bytes]   Original file size (uint64, big-endian)
[tree]      Serialized Huffman tree (1 = leaf + 8-bit char, 0 = internal node)
[3 bits]    Padding count (0–7)
[data]      Encoded payload
[padding]   Zero bits to fill last byte
```

### Compression Ratio

Compression effectiveness depends on byte distribution:

| File Type | Typical Reduction |
|-----------|------------------|
| English text | 40–50% |
| Source code | 35–45% |
| Repeated data | 60–80% |
| Already compressed (zip, jpg) | 0–5% (may increase) |

---

## 🖥️ Web App Sections

After compressing a file, the web app displays:

| Section | Description |
|---------|-------------|
| **Bit Comparison** | Original vs compressed bits shown as colored binary — `0`s in gray, `1`s in green |
| **Huffman Code Table** | Top 20 characters with their ASCII code, frequency, original 8-bit, Huffman code, and bits saved |
| **Stats** | Original size, output size, compression ratio |
| **Frequency Chart** | Bar chart of top 64 most frequent byte values |
| **Download** | One-click download of the compressed/decompressed file |

---

## 🛠️ Tech Stack

| Component | Technology |
|-----------|-----------|
| Web App | HTML, CSS, JavaScript (vanilla) |
| CLI Tool | C++17 |
| Data Structures | Min-Heap (priority queue), Binary Tree |
| Fonts | DM Mono, Syne (Google Fonts) |

---

## 📚 Data Structures Used

- **Min-Heap / Priority Queue** — efficient node selection during tree construction
- **Binary Tree** — Huffman tree for prefix-free code generation
- **Hash Map** — frequency table and code lookup
- **Bit Manipulation** — packing/unpacking variable-length codes into bytes

---

## 👤 Author

Malhar Hajare — DSA Project

---

## 📄 License

This project is for academic/educational purposes.
