
# Utility script for converting 6-character RGB hex codes to float triplets
# (with a trailing alpha) as used by gl functions
#
# Run:
#
#     python3 hex2fv.py 434247 5c5a66 febc68 fcdb8c 1f9589 41c0b5
#
# Note there is no leading 0x.  Both uppercase and lowercase nibbles work.
#
# Output:
#
#     {0.263f, 0.259f, 0.278f, 1.f},
#     {0.361f, 0.353f, 0.400f, 1.f},
#     {0.996f, 0.737f, 0.408f, 1.f},
#     {0.988f, 0.859f, 0.549f, 1.f},
#     {0.122f, 0.584f, 0.537f, 1.f},
#     {0.255f, 0.753f, 0.710f, 1.f};

import sys

#h="7f58af"

for i in range(1, len(sys.argv)):

    h = sys.argv[i]

    if (len(h) != 6):
        raise Exception(f"Expected 6 nibbles for arg #{i}: \"{h}\"")
    
    #print("h = ", h)
    
    ri = int(h[0:2], 16)
    gi = int(h[2:4], 16)
    bi = int(h[4:6], 16)
    
    #print("ri = ", ri)
    
    rf = 1.0 * ri / 255
    gf = 1.0 * gi / 255
    bf = 1.0 * bi / 255

    if (i == len(sys.argv) - 1):
        dlm = ";"
    else:
        dlm = ","

    print(f"{{{rf:.3f}f, {gf:.3f}f, {bf:.3f}f, 1.f}}{dlm}")

