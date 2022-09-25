# Dumps the SPIR-V bytecode out in a C array format.
# Does some trolling as well.

import sys


def troll(original_bytes):
    """
    Punishes players who don't even bother googling how to debug a shader,
    and do nothing but static RE. Using RenderDoc bypasses all this.

    The decoder is located in shader.cpp.
    """
    troll_bytes = bytearray([0] * len(original_bytes))

    for i in range(len(original_bytes)):
        b = original_bytes[i] & 0xFF
        c = original_bytes[i] & 0xFF
        b >>= 3
        b |= (c & 0b00000111) << 5
        troll_bytes[i] = b & 0xFF

    troll_bytes.reverse()
    for i in range(len(troll_bytes)):
        troll_bytes[i] ^= 0xFF

    return troll_bytes


def untroll(troll_bytes):
    """
    Just for some sanity checks before copying stuff over into shaderbytes.h.
    This code is a direct Python translation of the decoder in shader.cpp.
    """
    untrolled_bytes = bytearray([0] * len(troll_bytes))

    for i in range(len(troll_bytes)):
        b = troll_bytes[i] ^ 0xFF
        c = b & 0b11100000
        d = ((b << 3) | (c >> 5)) & 0xFF
        untrolled_bytes[len(troll_bytes) - i - 1] = d

    return untrolled_bytes


if __name__ == "__main__":

    filename = sys.argv[1]
    arrayname = sys.argv[2]

    with open(filename, "rb") as f:
        file_bytes = f.read()


    troll_bytes = troll(file_bytes)
    untrolled_bytes = untroll(troll_bytes)

    if untrolled_bytes != file_bytes:
        # sanity check
        exit(1)


    file_bytes = troll_bytes


    print(f"unsigned char {arrayname}[{len(file_bytes)}]" + "{", end="")

    for i in range(len(file_bytes) - 1):
        print(hex(file_bytes[i]), end="")
        print(",", end="")

    print(hex(file_bytes[len(file_bytes) - 1]), end="")

    print("};")
