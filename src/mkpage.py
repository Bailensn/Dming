#!/usr/bin/env python3
"""把 src/page.html 转成 src/page.h（一个 C 字符串字面量）。

为什么要有这一步：C 的字符串字面量不能跨行，得逐行加引号和 \\n。
手工维护 364 行 HTML 不现实，所以用脚本生成。

之所以能生成得很干净，是因为 page.html 里**没有任何双引号**：
HTML 属性统一用单引号，JS 字符串也用单引号。这样每行只需要加一对引号
和结尾的 \\n，不需要任何转义。

改页面时：编辑 src/page.html，然后重新跑

    python3 src/mkpage.py

不要直接改 src/page.h。
"""

import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, 'page.html')
DST = os.path.join(HERE, 'page.h')

HEADER = '''/*
 * 本文件由 src/mkpage.py 自动生成，请不要直接编辑。
 *
 *   源文件:    src/page.html
 *   重新生成:  python3 src/mkpage.py
 *
 * 占位符（渲染时被替换）: {{CLASS}} {{VERSION}} {{STUDENTS}}
 */
#ifndef DMING_PAGE_H
#define DMING_PAGE_H

static const char PAGE_TEMPLATE[] =
'''

FOOTER = ''';

#endif /* DMING_PAGE_H */
'''


def c_escape(line):
    """转义成 C 字符串字面量的内容（故意保持 UTF-8 原样，不转成 \\xNN）。"""
    out = []
    for ch in line:
        if ch == '\\':
            out.append('\\\\')
        elif ch == '"':
            out.append('\\"')
        elif ch == '?':
            # 防止 ??= ??/ 之类的 trigraph 被误解释（虽然 C23 已移除，老编译器仍在）
            out.append('?')
        else:
            out.append(ch)
    return ''.join(out)


def main():
    text = open(SRC, encoding='utf-8').read()

    if '"' in text:
        sys.exit('page.html 含双引号，会破坏生成的 C 字面量；'
                 '请把属性引号改成单引号')
    if '\\' in text:
        sys.exit('page.html 含反斜杠，请先处理')

    lines = text.split('\n')
    # 文件末尾的换行会产生一个空尾巴，去掉它
    if lines and lines[-1] == '':
        lines.pop()

    body = []
    for line in lines:
        body.append('\t"%s\\n"\n' % c_escape(line))

    with open(DST, 'w', encoding='utf-8') as f:
        f.write(HEADER)
        f.writelines(body)
        f.write(FOOTER)

    total = len(text.encode('utf-8'))
    print('已生成 %s' % DST)
    print('  源: %s (%d 行, %d 字节)' % (SRC, len(lines), total))
    print('  出: %d 字节' % os.path.getsize(DST))
    print('  C 字符串长度: %d 字节' % total)


if __name__ == '__main__':
    main()
