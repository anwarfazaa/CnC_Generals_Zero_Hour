#!/usr/bin/env python3
"""
Convert SPIR-V binary files to C header files with embedded byte arrays.
Usage: spirv_to_header.py <input.spv> <output.h> <array_name>
"""

import sys
import os

def spirv_to_header(input_file, output_file, array_name):
    """Convert SPIR-V binary to C header with byte array."""

    # Read SPIR-V binary
    with open(input_file, 'rb') as f:
        spirv_data = f.read()

    # Generate header content
    header_content = f"""/* Auto-generated from {os.path.basename(input_file)} */
#ifndef {array_name.upper()}_H
#define {array_name.upper()}_H

static const unsigned char {array_name}[] = {{
"""

    # Write bytes in rows of 12
    for i in range(0, len(spirv_data), 12):
        chunk = spirv_data[i:i+12]
        hex_values = ', '.join(f'0x{b:02x}' for b in chunk)
        header_content += f"    {hex_values},\n"

    header_content += f"""}};

static const unsigned int {array_name}_len = {len(spirv_data)};

#endif // {array_name.upper()}_H
"""

    # Write header file
    with open(output_file, 'w') as f:
        f.write(header_content)

    print(f"Generated {output_file} ({len(spirv_data)} bytes)")

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <input.spv> <output.h> <array_name>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]
    array_name = sys.argv[3]

    if not os.path.exists(input_file):
        print(f"Error: Input file '{input_file}' not found")
        sys.exit(1)

    spirv_to_header(input_file, output_file, array_name)
