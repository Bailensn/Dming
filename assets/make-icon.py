#!/usr/bin/env python3
"""从 assets/icon-source.png 生成 assets/dming.ico 与 assets/dming.png。

icon-source.png 是 Dming.webp 里的一帧整幅关键帧（620x620、带 alpha 的 RGBA），
见 README 的「图标」一节。想换图标时：把新的正方形 PNG 覆盖成 icon-source.png，
再跑一次本脚本即可。

只依赖标准库，不需要 Pillow。ICO 里全部用 BMP/DIB 条目（不用 PNG 压缩条目），
兼容性最好 —— 某些图标解析器不认 ICO 内嵌 PNG。
"""

import os
import struct
import zlib

SIZES = [16, 24, 32, 48, 64, 128, 256]
PREVIEW_SIZE = 256


# --------------------------------------------------------------------------
# 极简 PNG 读取：8bit RGBA（colortype 6），支持全部 5 种扫描线 filter
# --------------------------------------------------------------------------
def read_png(path):
    data = open(path, 'rb').read()
    if data[:8] != b'\x89PNG\r\n\x1a\n':
        raise ValueError('%s 不是 PNG 文件' % path)

    pos = 8
    idat = b''
    width = height = None
    while pos + 8 <= len(data):
        length = struct.unpack_from('>I', data, pos)[0]
        tag = data[pos + 4:pos + 8]
        body = data[pos + 8:pos + 8 + length]
        if tag == b'IHDR':
            width, height, depth, color, comp, filt, interlace = \
                struct.unpack('>IIBBBBB', body)
            if depth != 8 or color != 6 or interlace != 0:
                raise ValueError('只支持 8bit RGBA 非隔行 PNG'
                                 '（当前 depth=%d color=%d interlace=%d）'
                                 % (depth, color, interlace))
        elif tag == b'IDAT':
            idat += body
        elif tag == b'IEND':
            break
        pos += 12 + length

    raw = zlib.decompress(idat)
    stride = width * 4
    out = bytearray(height * stride)
    prev = bytearray(stride)
    p = 0
    for y in range(height):
        ftype = raw[p]
        p += 1
        line = bytearray(raw[p:p + stride])
        p += stride
        if ftype == 1:      # Sub
            for i in range(4, stride):
                line[i] = (line[i] + line[i - 4]) & 0xFF
        elif ftype == 2:    # Up
            for i in range(stride):
                line[i] = (line[i] + prev[i]) & 0xFF
        elif ftype == 3:    # Average
            for i in range(stride):
                a = line[i - 4] if i >= 4 else 0
                line[i] = (line[i] + ((a + prev[i]) >> 1)) & 0xFF
        elif ftype == 4:    # Paeth
            for i in range(stride):
                a = line[i - 4] if i >= 4 else 0
                b = prev[i]
                c = prev[i - 4] if i >= 4 else 0
                pa, pb, pc = abs(b - c), abs(a - c), abs(a + b - 2 * c)
                if pa <= pb and pa <= pc:
                    pr = a
                elif pb <= pc:
                    pr = b
                else:
                    pr = c
                line[i] = (line[i] + pr) & 0xFF
        elif ftype != 0:
            raise ValueError('未知的 PNG filter 类型 %d' % ftype)
        out[y * stride:(y + 1) * stride] = line
        prev = line
    return out, width, height


# --------------------------------------------------------------------------
# 面积平均缩放（下采样质量好，且不用第三方库）
# --------------------------------------------------------------------------
def resize_area(src, sw, sh, dw, dh):
    dst = bytearray(dw * dh * 4)
    rows = [(y * sh // dh, max(y * sh // dh + 1, (y + 1) * sh // dh))
            for y in range(dh)]
    cols = [(x * sw // dw, max(x * sw // dw + 1, (x + 1) * sw // dw))
            for x in range(dw)]
    for y in range(dh):
        y0, y1 = rows[y]
        for x in range(dw):
            x0, x1 = cols[x]
            r = g = b = a = 0
            n = (y1 - y0) * (x1 - x0)
            for yy in range(y0, y1):
                base = yy * sw * 4
                for xx in range(x0, x1):
                    o = base + xx * 4
                    r += src[o]
                    g += src[o + 1]
                    b += src[o + 2]
                    a += src[o + 3]
            o = (y * dw + x) * 4
            dst[o] = r // n
            dst[o + 1] = g // n
            dst[o + 2] = b // n
            dst[o + 3] = a // n
    return dst


# --------------------------------------------------------------------------
# 输出
# --------------------------------------------------------------------------
def bmp_entry(px, size):
    """ICO 条目内容：BITMAPINFOHEADER + BGRA(自下而上) + AND 掩码。"""
    header = struct.pack('<IiiHHIIiiII',
                         40, size, size * 2, 1, 32, 0, size * size * 4,
                         0, 0, 0, 0)
    xor = bytearray()
    for y in range(size - 1, -1, -1):
        for x in range(size):
            o = (y * size + x) * 4
            xor += bytes((px[o + 2], px[o + 1], px[o], px[o + 3]))
    mask_stride = ((size + 31) // 32) * 4
    return header + bytes(xor) + bytes(mask_stride * size)


def write_ico(path, images):
    offset = 6 + 16 * len(images)
    directory = struct.pack('<HHH', 0, 1, len(images))
    blobs = b''
    for size, blob in images:
        dim = 0 if size >= 256 else size
        directory += struct.pack('<BBBBHHII', dim, dim, 0, 0, 1, 32,
                                 len(blob), offset)
        offset += len(blob)
        blobs += blob
    with open(path, 'wb') as f:
        f.write(directory + blobs)


def write_png(path, px, size):
    raw = bytearray()
    for y in range(size):
        raw.append(0)
        raw += px[y * size * 4:(y + 1) * size * 4]

    def chunk(tag, body):
        payload = tag + body
        return (struct.pack('>I', len(body)) + payload
                + struct.pack('>I', zlib.crc32(payload) & 0xFFFFFFFF))

    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n'
                + chunk(b'IHDR', struct.pack('>IIBBBBB', size, size, 8, 6, 0, 0, 0))
                + chunk(b'IDAT', zlib.compress(bytes(raw), 9))
                + chunk(b'IEND', b''))


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    src_path = os.path.join(here, 'icon-source.png')
    src, sw, sh = read_png(src_path)
    if sw != sh:
        raise SystemExit('icon-source.png 必须是正方形（当前 %dx%d）' % (sw, sh))

    images = []
    for size in SIZES:
        px = src if size == sw else resize_area(src, sw, sh, size, size)
        images.append((size, bmp_entry(px, size)))
        if size == PREVIEW_SIZE:
            write_png(os.path.join(here, 'dming.png'), px, size)

    ico = os.path.join(here, 'dming.ico')
    write_ico(ico, images)
    print('源图: %s (%dx%d)' % (src_path, sw, sh))
    print('已生成 %s —— %d 个尺寸: %s，共 %d 字节'
          % (ico, len(SIZES), ', '.join(str(s) for s in SIZES),
             os.path.getsize(ico)))
    print('已生成 %s' % os.path.join(here, 'dming.png'))


if __name__ == '__main__':
    main()
