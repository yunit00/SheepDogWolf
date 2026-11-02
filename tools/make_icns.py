"""Package the generated PNG icon sizes as ICNS, without iconutil services."""
import pathlib
import struct
import sys

folder, output = map(pathlib.Path, sys.argv[1:])
chunks = []
for kind, filename in [(b'icp4', 'icon_16x16.png'), (b'icp5', 'icon_32x32.png'),
                       (b'icp6', 'icon_32x32@2x.png'), (b'ic07', 'icon_128x128.png'),
                       (b'ic08', 'icon_256x256.png'), (b'ic09', 'icon_512x512.png'),
                       (b'ic10', 'icon_512x512@2x.png')]:
    data = (folder / filename).read_bytes()
    if not data.startswith(b'\x89PNG\r\n\x1a\n'):
        raise ValueError('Expected PNG icon')
    chunks.append(kind + struct.pack('>I', len(data) + 8) + data)
body = b''.join(chunks)
output.write_bytes(b'icns' + struct.pack('>I', len(body) + 8) + body)
