#!/usr/bin/env python3
"""检查 PE 文件里有没有 .rsrc 段 —— 也就是图标 / 版本信息资源。

为什么要这么查：不能用「文件属性里有没有图标」这种间接判断。
Windows 的 ExtractAssociatedIcon 对没有图标资源的文件会返回通用程序图标，
而不是失败，等于白测。直接解析 PE 段表最实在：
Go / gcc 的链接器只有在真的链接了资源对象时才会产生 .rsrc 段。

用法：
    python3 tools/check-pe.py dist/*.exe

任何文件缺 .rsrc 就以非零退出，构建会因此失败。
"""

import struct
import sys


def section_names(path):
    with open(path, 'rb') as f:
        d = f.read()

    if d[:2] != b'MZ':
        raise ValueError('不是 PE 文件（缺 MZ 头）')

    pe = struct.unpack_from('<I', d, 0x3C)[0]
    if d[pe:pe + 4] != b'PE\0\0':
        raise ValueError('偏移 0x%X 处没有 PE\\0\\0 签名' % pe)

    nsec = struct.unpack_from('<H', d, pe + 6)[0]
    opt_size = struct.unpack_from('<H', d, pe + 20)[0]
    off = pe + 24 + opt_size

    names = []
    for i in range(nsec):
        raw = d[off + i * 40: off + i * 40 + 8]
        names.append(raw.split(b'\0')[0].decode('ascii', 'replace'))
    return names


def main(argv):
    if len(argv) < 2:
        sys.exit('用法: check-pe.py <file.exe> [...]')

    failed = []
    for path in argv[1:]:
        try:
            names = section_names(path)
        except (ValueError, OSError) as e:
            print('FAIL %s: %s' % (path, e))
            failed.append(path)
            continue

        has = '.rsrc' in names
        print('%s %s' % ('OK  ' if has else 'FAIL', path))
        print('      段: %s' % ', '.join(names))
        if not has:
            failed.append(path)

    if failed:
        print()
        print('以下文件缺 .rsrc 段，说明图标/版本信息没有被链接进去:')
        for p in failed:
            print('  %s' % p)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
