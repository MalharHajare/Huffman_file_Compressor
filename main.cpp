#include <bitset>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// ─── Huffman Tree Node
// ────────────────────────────────────────────────────────

struct Node {
  unsigned char ch;
  int freq;
  Node *left;
  Node *right;

  Node(unsigned char c, int f)
      : ch(c), freq(f), left(nullptr), right(nullptr) {}
  Node(int f, Node *l, Node *r) : ch(0), freq(f), left(l), right(r) {}
};

struct Compare {
  bool operator()(Node *a, Node *b) { return a->freq > b->freq; }
};

// ─── Build frequency table
// ────────────────────────────────────────────────────

std::unordered_map<unsigned char, int>
buildFreqTable(const std::vector<unsigned char> &data) {
  std::unordered_map<unsigned char, int> freq;
  for (unsigned char c : data)
    freq[c]++;
  return freq;
}

// ─── Build Huffman tree
// ───────────────────────────────────────────────────────

Node *buildTree(const std::unordered_map<unsigned char, int> &freq) {
  std::priority_queue<Node *, std::vector<Node *>, Compare> pq;
  for (auto &[ch, f] : freq)
    pq.push(new Node(ch, f));

  if (pq.size() == 1) {
    Node *only = pq.top();
    pq.pop();
    Node *root = new Node(only->freq, only, nullptr);
    return root;
  }

  while (pq.size() > 1) {
    Node *l = pq.top();
    pq.pop();
    Node *r = pq.top();
    pq.pop();
    pq.push(new Node(l->freq + r->freq, l, r));
  }
  return pq.top();
}

// ─── Generate codes
// ───────────────────────────────────────────────────────────

void generateCodes(Node *node, const std::string &prefix,
                   std::unordered_map<unsigned char, std::string> &codes) {
  if (!node)
    return;
  if (!node->left && !node->right) {
    codes[node->ch] = prefix.empty() ? "0" : prefix;
    return;
  }
  generateCodes(node->left, prefix + "0", codes);
  generateCodes(node->right, prefix + "1", codes);
}

// ─── Free tree memory
// ─────────────────────────────────────────────────────────

void freeTree(Node *node) {
  if (!node)
    return;
  freeTree(node->left);
  freeTree(node->right);
  delete node;
}

// ─── Write bits to file
// ───────────────────────────────────────────────────────

class BitWriter {
  std::ofstream &out;
  unsigned char buf = 0;
  int count = 0;

public:
  BitWriter(std::ofstream &o) : out(o) {}

  void writeBit(int bit) {
    buf = (buf << 1) | (bit & 1);
    if (++count == 8) {
      out.put(buf);
      buf = 0;
      count = 0;
    }
  }

  // Returns number of padding bits added
  int flush() {
    int padding = 0;
    if (count > 0) {
      padding = 8 - count;
      buf <<= padding;
      out.put(buf);
    }
    return padding;
  }
};

// ─── Read bits from file
// ──────────────────────────────────────────────────────

class BitReader {
  std::ifstream &in;
  unsigned char buf = 0;
  int count = 0;

public:
  BitReader(std::ifstream &i) : in(i) {}

  int readBit() {
    if (count == 0) {
      int c = in.get();
      if (c == EOF)
        return -1;
      buf = (unsigned char)c;
      count = 8;
    }
    int bit = (buf >> 7) & 1;
    buf <<= 1;
    count--;
    return bit;
  }
};

// ─── Serialize / Deserialize tree ────────────────────────────────────────────
// Format: 1-bit flag (1 = leaf, 0 = internal), leaves store 8-bit char.

void serializeTree(Node *node, BitWriter &bw) {
  if (!node->left && !node->right) {
    bw.writeBit(1);
    for (int i = 7; i >= 0; i--)
      bw.writeBit((node->ch >> i) & 1);
    return;
  }
  bw.writeBit(0);
  serializeTree(node->left, bw);
  serializeTree(node->right, bw);
}

Node *deserializeTree(BitReader &br) {
  int flag = br.readBit();
  if (flag == 1) {
    unsigned char ch = 0;
    for (int i = 0; i < 8; i++)
      ch = (ch << 1) | br.readBit();
    return new Node(ch, 0);
  }
  Node *l = deserializeTree(br);
  Node *r = deserializeTree(br);
  return new Node(0, l, r);
}

// ─── Read binary file
// ─────────────────────────────────────────────────────────

std::vector<unsigned char> readFile(const std::string &path) {
  std::ifstream f(path, std::ios::binary);
  if (!f)
    throw std::runtime_error("Cannot open file: " + path);
  return {std::istreambuf_iterator<char>(f), {}};
}

// ─── COMPRESS
// ─────────────────────────────────────────────────────────────────
//
// File format (all in bits via BitWriter):
//   [8 bytes]  original file size (uint64, big-endian)
//   [tree]     serialized Huffman tree
//   [3 bits]   padding count (0–7)
//   [data]     encoded payload
//   [padding]  zero bits to fill last byte

void compress(const std::string &inPath, const std::string &outPath) {
  auto data = readFile(inPath);
  if (data.empty())
    throw std::runtime_error("Input file is empty.");

  auto freq = buildFreqTable(data);
  Node *root = buildTree(freq);

  std::unordered_map<unsigned char, std::string> codes;
  generateCodes(root, "", codes);

  std::ofstream out(outPath, std::ios::binary);
  if (!out)
    throw std::runtime_error("Cannot create output file: " + outPath);

  // Write original size (8 bytes, big-endian)
  uint64_t origSize = data.size();
  for (int i = 7; i >= 0; i--)
    out.put((origSize >> (i * 8)) & 0xFF);

  BitWriter bw(out);

  // Write tree
  serializeTree(root, bw);

  // Build encoded bit string to know padding
  std::string encoded;
  for (unsigned char c : data)
    encoded += codes[c];

  // Write padding count (3 bits)
  int padding = (8 - (encoded.size() % 8)) % 8;
  for (int i = 2; i >= 0; i--)
    bw.writeBit((padding >> i) & 1);

  // Flush tree bits + padding-count bits to align, then write raw encoded bytes
  bw.flush();

  // Write encoded data byte by byte
  for (size_t i = 0; i < encoded.size(); i += 8) {
    unsigned char byte = 0;
    for (int j = 0; j < 8; j++) {
      byte <<= 1;
      if (i + j < encoded.size())
        byte |= (encoded[i + j] - '0');
    }
    out.put(byte);
  }

  freeTree(root);

  out.close();
  std::ifstream sizeCheck(outPath, std::ios::ate | std::ios::binary);
  uint64_t compSize = (uint64_t)sizeCheck.tellg();
  double ratio = 100.0 * (1.0 - (double)compSize / origSize);
  std::cout << "Compressed:  " << inPath << " -> " << outPath << "\n"
            << "Original:    " << origSize << " bytes\n"
            << "Compressed:  " << compSize << " bytes\n"
            << "Reduction:   " << ratio << "%\n";
}

// ─── DECOMPRESS
// ───────────────────────────────────────────────────────────────

void decompress(const std::string &inPath, const std::string &outPath) {
  std::ifstream in(inPath, std::ios::binary);
  if (!in)
    throw std::runtime_error("Cannot open file: " + inPath);

  // Read original size
  uint64_t origSize = 0;
  for (int i = 0; i < 8; i++)
    origSize = (origSize << 8) | (unsigned char)in.get();

  BitReader br(in);

  // Read tree
  Node *root = deserializeTree(br);

  // Read padding count (3 bits)
  int padding = 0;
  for (int i = 0; i < 3; i++)
    padding = (padding << 1) | br.readBit();

  // Flush remaining bits in current byte (bit reader internal buffer)
  // by reading rest of encoded data as raw bytes
  // We need to consume the rest of the file after the bit-aligned boundary.
  // Since BitWriter::flush() was called after tree+padding bits, the encoded
  // data starts on a fresh byte boundary — read remaining bytes directly.
  std::vector<unsigned char> encodedBytes(std::istreambuf_iterator<char>(in),
                                          {});

  // Decode
  std::ofstream out(outPath, std::ios::binary);
  if (!out)
    throw std::runtime_error("Cannot create output file: " + outPath);

  Node *cur = root;
  uint64_t written = 0;

  for (size_t bi = 0; bi < encodedBytes.size() && written < origSize; bi++) {
    int bitsInByte = (bi == encodedBytes.size() - 1) ? (8 - padding) : 8;
    for (int i = 7; i >= 8 - bitsInByte && written < origSize; i--) {
      int bit = (encodedBytes[bi] >> i) & 1;
      cur = bit ? cur->right : cur->left;
      if (!cur)
        throw std::runtime_error("Corrupt compressed file.");
      if (!cur->left && !cur->right) {
        out.put(cur->ch);
        written++;
        cur = root;
      }
    }
  }

  freeTree(root);
  std::cout << "Decompressed: " << inPath << " -> " << outPath << "\n"
            << "Bytes written: " << written << "\n";
}

// ─── Main
// ─────────────────────────────────────────────────────────────────────

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << "Usage:\n"
              << "  Compress:   " << argv[0] << " c <input> <output.huff>\n"
              << "  Decompress: " << argv[0] << " d <input.huff> <output>\n";
    return 1;
  }

  std::string mode = argv[1];
  std::string inPath = argv[2];
  std::string outPath = argv[3];

  try {
    if (mode == "c")
      compress(inPath, outPath);
    else if (mode == "d")
      decompress(inPath, outPath);
    else {
      std::cerr << "Unknown mode: " << mode << "\n";
      return 1;
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
  return 0;
}