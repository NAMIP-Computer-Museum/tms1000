# This is a sample Python script.

# Press Maj+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.

mapping = [3, 4, 11, 12, 19, 20, 27, 28, 35, 36, 43, 44, 51, 52, 59, 60, 0, 7, 8, 15, 16, 23, 24, 31, 32, 39, 40, 47, 48, 55, 56, 63, 2, 5, 10, 13, 18, 21, 26, 29, 34, 37, 42, 45, 50, 53, 58, 61, 1, 6, 9, 14, 17, 22, 25, 30, 33, 38, 41, 46, 49, 54, 57, 62]

# algo adapted from web but does not seem to match
def convert(tab):
    rx = 0
    res = [0] * (int)(len(tab)/8)
    for page in range(0,16):
        for pc in range(0,64):
            for bit in range(7,-1,-1):
                if page<8 :
                   rbi = pc*128+bit*16+page
                else:
                    rbi = pc*128+bit*16+23-page
                res[rx]=res[rx]*2+tab[rbi]
            rx = rx+1
    return res

def reorder(tab):
    res = [0] * len(tab)
    for i in range(0,len(tab)):
        r = i % 64
        d = i // 64
        res[i] = tab[d*64+mapping[r]]
    return res

# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    # Open the binary file in binary mode ('rb')
    with open('littleprof78raw.bin', 'rb') as file:
        # Read the entire content of the file into a bytes object
        raw_byte = bytearray(file.read())

    # Convert each byte to its binary representation and concatenate
    raws = ''.join(format(byte, '08b') for byte in raw_byte)

    # Now, bit_array is a string containing the binary representation of the file
    print(raws)

    raw = [int(bit) for bit in raws]

    # Now, bit_array is a list containing the bits as integers
    print(raw)
    print(len(raw))

    bin = convert(raw)
    print(bin)
    bins = [hex(byte) for byte in bin]
    print(bins)

    rbin = reorder(bin)
    print(rbin)
    rbins = [hex(byte) for byte in rbin]
    print(rbins)

