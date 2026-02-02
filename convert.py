"""Convert .c file exported from Piskel from ARGB8888 to RGB565. See <https://www.piskelapp.com>."""

import pathlib
import re
import os


def argb8888_to_rgb565(color):
    r = (color >> 16) & 0xFF
    g = (color >> 8) & 0xFF
    b = color & 0xFF

    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)


def convert_c_file(input_path, output_path):
    with open(input_path, "r") as f:
        content = f.read()

    # Extract all hex color values
    hex_values = re.findall(r"0x[0-9a-fA-F]{8}", content, re.ASCII)
    if len(hex_values) != 16 * 16:
        print(f"Warning: expected 256 values, found {len(hex_values)}")

    # Convert to RGB565
    rgb565_values = [argb8888_to_rgb565(int(v, 16)) for v in hex_values]

    # Header replacement
    header = """#if defined(__AVR__)
#include <avr/pgmspace.h>
#elif defined(__PIC32MX__)
#define PROGMEM
#elif defined(__arm__)
#define PROGMEM
#endif

"""

    # Format output array
    lines = []
    for i in range(0, len(rgb565_values), 16):
        chunk = rgb565_values[i : i + 16]
        line = ", ".join(f"0x{v:04X}" for v in chunk)
        lines.append("  " + line)

    array_body = ",\n".join(lines)

    output = (
        header
        + f"const unsigned short {pathlib.Path(output_path).stem}[256] PROGMEM = {{\n{array_body}\n}};\n"
    )

    with open(output_path, "w") as f:
        f.write(output)

    print(f"Converted file written to {output_path}")


if __name__ == "__main__":
    piskel_filenames = [
        filename
        for filename in os.listdir(pathlib.Path(__file__).parent / "aqi-matrix")
        if filename.endswith(".c")
    ]
    for filename in piskel_filenames:
        input_path = pathlib.Path(__file__).parent / "aqi-matrix" / filename
        output_path = pathlib.Path(__file__).parent / "aqi-matrix" / (filename[:-2] + ".h")
        convert_c_file(input_path, output_path)
