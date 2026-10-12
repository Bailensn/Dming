/*
 * dming —— 课堂随机点名器（纯 C 实现）
 *
 * 从「可执行文件所在目录」读取两个文本文件：
 *
 *   p.txt      每行一个学生姓名（空行忽略）
 *   class.txt  班级名称，取第一个非空行
 *
 * 生成一个临时 HTML 页面（DMING*.html）用系统默认浏览器打开，
 * 等待若干秒后删除，可用 -keep 保留。
 *
 * 支持 Windows / macOS / Linux / Android(Termux)。
 *
 * 为什么不用 Go：Go 链接出的 PE 结构特殊（没有常规 CRT、节表不寻常），
 * 国内杀软的启发式引擎很容易把未签名的 Go 程序误判成木马。用系统自带
 * C 工具链编出的常规 PE，误报率低得多。
 *
 * 编译见 build.sh / build.bat，也可以直接：
 *   cc -O2 -o dming dming.c
 * Windows 上加资源（图标 + 版本信息）：
 *   windres dming.rc -O coff -o dming_res.o && cc -O2 -o dming.exe dming.c dming_res.o -luser32
 */

/*
 * 特性宏必须在任何 #include 之前定义。
 *
 * 用 -std=c99（而不是 gnu99）时会定义 __STRICT_ANSI__，于是：
 *   glibc -> 不再自动定义 _DEFAULT_SOURCE，_POSIX_C_SOURCE 也设不上，
 *            __USE_POSIX 被关掉，readlink / fork / execvp / waitpid /
 *            nanosleep / realpath 全部失去声明。
 *   macOS -> __DARWIN_C_LEVEL 退化成 __DARWIN_C_ANSI，POSIX 声明一样被藏。
 * 在 GCC 14 / Clang 下隐式声明是硬错误，直接编译失败。
 *
 * bionic（Android）不做这套隐藏，所以只有 Android 能编过 —— 这个差异很坑。
 * 这里显式打开，配合构建脚本里的 -std=gnu99 双保险。
 */
#if !defined(_WIN32)
#  ifndef _DEFAULT_SOURCE
#    define _DEFAULT_SOURCE 1      /* glibc: 打开 POSIX + BSD 声明 */
#  endif
#  ifndef _DARWIN_C_SOURCE
#    define _DARWIN_C_SOURCE 1     /* macOS: 打开全部声明（其他平台无副作用） */
#  endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "page.h"

#define APP_NAME    "dming"
#define APP_VERSION "3.4.3"
#define APP_DESC    "课堂随机点名器"

/* 默认：开浏览器后等 3 秒再删临时文件 */
#define DEFAULT_DELAY_MS 3000
#define MAX_TEMP_TRIES   32

/* ==========================================================================
 * 平台层
 *
 * Windows 上路径全部用 wchar_t（_wfopen / ShellExecuteW）：
 * fopen 走 ANSI 代码页，遇到中文用户名（C:\Users\张三\...）会直接失败。
 * 其他平台就是普通 char。
 * ========================================================================== */

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <shellapi.h>   /* ShellExecuteW —— 不在 windows.h 里，漏了会编译失败 */
#  include <wchar.h>      /* wcslen / wcsrchr */
typedef wchar_t pchar;
#  define PLIT_(s) L##s
#  define PATH_SEP L'\\'
#else
#  include <sys/types.h>  /* pid_t / ssize_t，别指望 unistd.h 一定会带进来 */
#  include <unistd.h>
#  include <sys/stat.h>
#  include <sys/wait.h>
#  include <errno.h>
#  include <limits.h>
#  ifdef __APPLE__
#    include <mach-o/dyld.h>
#  endif
typedef char pchar;
#  define PLIT_(s) s
#  define PATH_SEP '/'
#endif
#define PLIT(s) PLIT_(s)

/* ---------- 内存 ---------- */

static void oom(void)
{
#ifdef _WIN32
    MessageBoxW(NULL, L"内存不足", L"dming", MB_ICONERROR);
#endif
    fputs("dming: 内存不足\n", stderr);
    exit(1);
}

static void *xmalloc(size_t n)
{
    void *p = malloc(n ? n : 1);
    if (!p)
        oom();
    return p;
}

static void *xrealloc(void *p, size_t n)
{
    void *q = realloc(p, n ? n : 1);
    if (!q)
        oom();
    return q;
}

/* ---------- pchar 小工具 ---------- */

static size_t plen(const pchar *s)
{
#ifdef _WIN32
    return wcslen(s);
#else
    return strlen(s);
#endif
}

static pchar *pdup(const pchar *s)
{
    size_t n = (plen(s) + 1) * sizeof(pchar);
    pchar *r = xmalloc(n);
    memcpy(r, s, n);
    return r;
}

/* dir + 分隔符 + name；目录已带分隔符就不重复加 */
static pchar *pjoin(const pchar *dir, const pchar *name)
{
    size_t a = plen(dir), b = plen(name);
    pchar *r = xmalloc((a + b + 2) * sizeof(pchar));
    memcpy(r, dir, a * sizeof(pchar));
    if (a > 0 && dir[a - 1] != PATH_SEP)
        r[a++] = PATH_SEP;
    memcpy(r + a, name, (b + 1) * sizeof(pchar));
    return r;
}

/* pchar* -> UTF-8 的 char*，用于打印。caller 负责 free。 */
static char *p_to_utf8(const pchar *s)
{
#ifdef _WIN32
    int need = WideCharToMultiByte(CP_UTF8, 0, s, -1, NULL, 0, NULL, NULL);
    char *u;
    if (need <= 0)
        return NULL;
    u = xmalloc((size_t)need);
    WideCharToMultiByte(CP_UTF8, 0, s, -1, u, need, NULL, NULL);
    return u;
#else
    return pdup(s);
#endif
}

/* 打印路径（自动转 UTF-8） */
static void fprint_path(FILE *f, const pchar *p)
{
    char *u = p_to_utf8(p);
    if (u) {
        fputs(u, f);
        free(u);
    } else {
        fputs("<路径无法显示>", f);
    }
}

/* ---------- 报错：命令行 + 图形弹窗（Windows 双击场景看不见控制台） ---------- */

static void report_error(const char *fmt, ...)
{
    char buf[2048];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);

    fputs("错误：", stderr);
    fputs(buf, stderr);
    fputc('\n', stderr);
#ifdef _WIN32
    {
        /* buf 是 UTF-8，转成宽字符再弹窗 */
        int need = MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);
        if (need > 0) {
            wchar_t *w = xmalloc((size_t)need * sizeof(wchar_t));
            MultiByteToWideChar(CP_UTF8, 0, buf, -1, w, need);
            MessageBoxW(NULL, w, L"dming 出错", MB_ICONERROR | MB_OK);
            free(w);
        }
    }
#endif
}

/* ---------- 可执行文件所在目录 ---------- */

#ifndef _WIN32
/* 把 "/a/b/c" 就地截成 "/a/b"。成功返回 0。 */
static int strip_last_segment(char *p)
{
    char *s = strrchr(p, '/');
    if (!s)
        return -1;
    if (s == p)
        s[1] = '\0';    /* 根目录，保留开头的 '/' */
    else
        *s = '\0';
    return 0;
}

/*
 * 从 argv[0] 推目录。这是 Termux/Android 上的主要途径：
 * termux-exec 为了绕开 SELinux 对私有目录的执行限制，会把所有程序交给
 * /system/bin/linker64 启动，于是 /proc/self/exe 指向的是 linker 而不是本程序。
 * 但它在转交时会把 argv[0] 规范化成程序的绝对路径（相对路径和 PATH 调用都一样），
 * 所以这里反而更可靠。
 */
static pchar *dir_from_argv0(const char *a0)
{
    if (!a0 || !*a0)
        return NULL;

    if (strchr(a0, '/')) {
        /* 带路径（可能是相对的），按当前目录展开 */
        char real[PATH_MAX];
        if (!realpath(a0, real))
            return NULL;
        if (strip_last_segment(real) != 0)
            return NULL;
        return pdup(real);
    }

    /* 裸文件名：在 PATH 里找 */
    {
        const char *path = getenv("PATH");
        const char *p = path;
        if (!p || !*p)
            return NULL;
        while (*p) {
            const char *colon = strchr(p, ':');
            size_t seg = colon ? (size_t)(colon - p) : strlen(p);
            if (seg > 0 && seg + strlen(a0) + 2 < 4096) {
                char cand[4096];
                snprintf(cand, sizeof cand, "%.*s/%s", (int)seg, p, a0);
                if (access(cand, X_OK) == 0) {
                    char real[PATH_MAX];
                    if (realpath(cand, real) && strip_last_segment(real) == 0)
                        return pdup(real);
                }
            }
            if (!colon)
                break;
            p = colon + 1;
        }
    }
    return NULL;
}
#endif

static pchar *exe_dir(const char *argv0)
{
#ifdef _WIN32
    wchar_t buf[32768];
    DWORD n;
    wchar_t *s;

    /* GetModuleFileNameW 在 Windows 上足够可靠，不用 argv[0] 兜底 */
    (void)argv0;

    n = GetModuleFileNameW(NULL, buf, (DWORD)(sizeof buf / sizeof buf[0]));
    if (n == 0 || n >= (DWORD)(sizeof buf / sizeof buf[0]))
        return NULL;
    s = wcsrchr(buf, L'\\');
    if (!s)
        return NULL;
    *s = L'\0';
    return pdup(buf);
#elif defined(__APPLE__)
    {
        char buf[PATH_MAX];
        uint32_t sz = (uint32_t)sizeof buf;
        if (_NSGetExecutablePath(buf, &sz) != 0)
            return dir_from_argv0(argv0);
        {
            char real[PATH_MAX];
            if (!realpath(buf, real))
                return dir_from_argv0(argv0);
            if (strip_last_segment(real) != 0)
                return NULL;
            return pdup(real);
        }
    }
#else
    {
        char buf[4096];
        ssize_t n = readlink("/proc/self/exe", buf, sizeof buf - 1);
        if (n > 0) {
            const char *base;
            buf[n] = '\0';
            base = strrchr(buf, '/');
            base = base ? base + 1 : buf;
            /*
             * Termux/Android 上 /proc/self/exe 指向 linker 本身，不是本程序，
             * 必须排掉，否则会在 /apex/com.android.runtime/bin 下面找 p.txt。
             */
            if (strcmp(base, "linker64") != 0 && strcmp(base, "linker") != 0 &&
                access(buf, F_OK) == 0) {
                char real[PATH_MAX];
                if (realpath(buf, real) && strip_last_segment(real) == 0)
                    return pdup(real);
            }
        }
        return dir_from_argv0(argv0);
    }
#endif
}

/* ---------- 系统临时目录（程序目录不可写时的退路） ---------- */

static pchar *temp_dir(void)
{
#ifdef _WIN32
    wchar_t buf[MAX_PATH + 2];
    DWORD n = GetTempPathW((DWORD)(sizeof buf / sizeof buf[0]), buf);
    if (n == 0 || n > (DWORD)(sizeof buf / sizeof buf[0]))
        return NULL;
    /* GetTempPathW 通常带结尾反斜杠，去掉以免 pjoin 再加一个 */
    while (n > 1 && (buf[n - 1] == L'\\' || buf[n - 1] == L'/'))
        buf[--n] = L'\0';
    return pdup(buf);
#else
    const char *t = getenv("TMPDIR");
    if (!t || !*t)
        t = "/tmp";
    return pdup(t);
#endif
}

/* ---------- 文件操作 ---------- */

static FILE *p_fopen_read(const pchar *path)
{
#ifdef _WIN32
    return _wfopen(path, L"rb");
#else
    return fopen(path, "rb");
#endif
}

static FILE *p_fopen_write(const pchar *path)
{
#ifdef _WIN32
    return _wfopen(path, L"wb");
#else
    return fopen(path, "wb");
#endif
}

static void p_remove(const pchar *path)
{
#ifdef _WIN32
    _wremove(path);
#else
    remove(path);
#endif
}

static int p_exists(const pchar *path)
{
    FILE *f = p_fopen_read(path);
    if (f) {
        fclose(f);
        return 1;
    }
    return 0;
}

/* 读整个文件，返回 NUL 结尾的缓冲区；失败返回 NULL */
static char *read_file(const pchar *path, size_t *out_len)
{
    FILE *f = p_fopen_read(path);
    size_t cap = 8192, len = 0;
    char *buf;

    if (!f)
        return NULL;

    buf = xmalloc(cap);
    for (;;) {
        size_t got;
        if (len + 4096 + 1 > cap) {
            cap *= 2;
            buf = xrealloc(buf, cap);
        }
        got = fread(buf + len, 1, 4096, f);
        len += got;
        if (got < 4096)
            break;
    }
    fclose(f);
    buf[len] = '\0';
    if (out_len)
        *out_len = len;
    return buf;
}

/* ==========================================================================
 * 动态字符串缓冲
 * ========================================================================== */

typedef struct {
    char *p;
    size_t len, cap;
} Buf;

static void buf_init(Buf *b)
{
    b->cap = 256;
    b->len = 0;
    b->p = xmalloc(b->cap);
    b->p[0] = '\0';
}

static void buf_need(Buf *b, size_t extra)
{
    if (b->len + extra + 1 > b->cap) {
        while (b->len + extra + 1 > b->cap)
            b->cap *= 2;
        b->p = xrealloc(b->p, b->cap);
    }
}

static void buf_putn(Buf *b, const char *s, size_t n)
{
    buf_need(b, n);
    memcpy(b->p + b->len, s, n);
    b->len += n;
    b->p[b->len] = '\0';
}

static void buf_puts(Buf *b, const char *s)
{
    buf_putn(b, s, strlen(s));
}

static void buf_putc(Buf *b, char c)
{
    buf_need(b, 1);
    b->p[b->len++] = c;
    b->p[b->len] = '\0';
}

static void buf_free(Buf *b)
{
    free(b->p);
    b->p = NULL;
    b->len = b->cap = 0;
}

/* ==========================================================================
 * 姓名列表
 * ========================================================================== */

typedef struct {
    char **v;
    size_t n, cap;
} Names;

static void names_add(Names *ns, const char *s, size_t len)
{
    if (ns->n + 1 >= ns->cap) {
        ns->cap = ns->cap ? ns->cap * 2 : 16;
        ns->v = xrealloc(ns->v, ns->cap * sizeof(char *));
    }
    ns->v[ns->n] = xmalloc(len + 1);
    memcpy(ns->v[ns->n], s, len);
    ns->v[ns->n][len] = '\0';
    ns->n++;
}

/* ---------- 文本按行拆 ---------- */

static int is_blank(unsigned char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' ||
           c == '\v' || c == '\f';
}

/* 去掉首尾空白（就地截断尾部），返回新的起点 */
static char *trim(char *s)
{
    char *e;
    while (*s && is_blank((unsigned char)*s))
        s++;
    e = s + strlen(s);
    while (e > s && is_blank((unsigned char)e[-1]))
        *--e = '\0';
    return s;
}

/*
 * 按行拆开、逐行 trim、跳过空行，每行回调一次。
 * 兼容 CRLF；开头的 UTF-8 BOM 会被剥掉。返回非空行数。
 */
static size_t split_lines(char *text, void (*add)(void *, const char *, size_t),
                          void *ud)
{
    char *p = text;
    size_t count = 0;

    if ((unsigned char)p[0] == 0xEF && (unsigned char)p[1] == 0xBB &&
        (unsigned char)p[2] == 0xBF)
        p += 3;

    while (*p) {
        char *nl = strchr(p, '\n');
        char *line;
        if (nl)
            *nl = '\0';
        line = trim(p);
        if (*line) {
            add(ud, line, strlen(line));
            count++;
        }
        if (!nl)
            break;
        p = nl + 1;
    }
    return count;
}

static void add_name_cb(void *ud, const char *s, size_t len)
{
    names_add((Names *)ud, s, len);
}

/* class.txt 只取第一个非空行 */
typedef struct {
    char *value;
} FirstLine;

static void add_first_cb(void *ud, const char *s, size_t len)
{
    FirstLine *fl = (FirstLine *)ud;
    if (!fl->value) {
        fl->value = xmalloc(len + 1);
        memcpy(fl->value, s, len);
        fl->value[len] = '\0';
    }
}

/* ==========================================================================
 * 转义
 * ========================================================================== */

/*
 * 注入 JS 数组用：转义 " \ 和 < > &，UTF-8 原样透传。
 *
 * < > & 并不是 JSON 语法要求的，但这段 JSON 是嵌在 HTML 的 <script> 里的：
 * 姓名要是含 "</script>"（或者 "<script>"），就会冲出脚本块变成 HTML，
 * 轻则页面崩掉，重则被当成注入。转成 \u003c 之后 JS 解析出来还是原字符，
 * 显示完全一样，但不可能再冲出标签。这是「JSON 嵌 HTML」的标准做法。
 */
static void json_escape(Buf *out, const char *s)
{
    const unsigned char *p = (const unsigned char *)s;
    for (; *p; p++) {
        switch (*p) {
        case '"':  buf_puts(out, "\\\""); break;
        case '\\': buf_puts(out, "\\\\"); break;
        case '\n': buf_puts(out, "\\n");  break;
        case '\r': buf_puts(out, "\\r");  break;
        case '\t': buf_puts(out, "\\t");  break;
        case '<':  buf_puts(out, "\\u003c"); break;
        case '>':  buf_puts(out, "\\u003e"); break;
        case '&':  buf_puts(out, "\\u0026"); break;
        default:
            if (*p < 0x20) {
                char tmp[8];
                snprintf(tmp, sizeof tmp, "\\u%04x", (unsigned)*p);
                buf_puts(out, tmp);
            } else {
                buf_putc(out, (char)*p);
            }
        }
    }
}

/* 放进 HTML 文本节点/标题里用 */
static void html_escape(Buf *out, const char *s)
{
    for (; *s; s++) {
        switch (*s) {
        case '&': buf_puts(out, "&amp;");  break;
        case '<': buf_puts(out, "&lt;");   break;
        case '>': buf_puts(out, "&gt;");   break;
        case '\'': buf_puts(out, "&#39;"); break;
        default:  buf_putc(out, *s);
        }
    }
}

/* ==========================================================================
 * 模板渲染：把 {{KEY}} 换成 value
 * ========================================================================== */

static void render(Buf *out, const char *tpl, const char *key, const char *value)
{
    size_t klen = strlen(key);
    const char *p = tpl;
    for (;;) {
        const char *hit = strstr(p, key);
        if (!hit) {
            buf_puts(out, p);
            return;
        }
        buf_putn(out, p, (size_t)(hit - p));
        buf_puts(out, value);
        p = hit + klen;
    }
}

/* ==========================================================================
 * 打开浏览器
 * ========================================================================== */

#ifdef _WIN32
static int open_browser(const pchar *path)
{
    HINSTANCE r = ShellExecuteW(NULL, L"open", path, NULL, NULL, SW_SHOWNORMAL);
    return ((INT_PTR)r > 32) ? 0 : -1;   /* >32 才是成功 */
}
#else
static int try_exec(char *const argv[])
{
    pid_t pid = fork();
    int status = 0;
    pid_t w;

    if (pid < 0)
        return -1;
    if (pid == 0) {
        execvp(argv[0], argv);
        _exit(127);          /* 127 = 找不到命令 */
    }

    /* waitpid 可能被信号打断（EINTR），那不是失败，要接着等 */
    do {
        w = waitpid(pid, &status, 0);
    } while (w < 0 && errno == EINTR);

    if (w < 0)
        return -1;
    if (WIFEXITED(status) && WEXITSTATUS(status) == 127)
        return -1;
    return 0;
}

static int open_browser(const pchar *path)
{
    char *p = (char *)path;   /* 非 Windows 下 pchar 就是 char */
    static const char *cands[] = {
        "xdg-open", "termux-open", "gio", "sensible-browser",
        "x-www-browser", "wslview", NULL
    };
    size_t i;
    for (i = 0; cands[i]; i++) {
        char *argv[4];
        argv[0] = (char *)cands[i];
        if (strcmp(cands[i], "gio") == 0) {
            argv[1] = (char *)"open";
            argv[2] = p;
            argv[3] = NULL;
        } else {
            argv[1] = p;
            argv[2] = NULL;
            argv[3] = NULL;
        }
        if (try_exec(argv) == 0)
            return 0;
    }
    return -1;
}
#endif

/* ==========================================================================
 * 延时（被信号打断就提前返回）
 * ========================================================================== */

static void sleep_ms(int ms)
{
    if (ms <= 0)
        return;
#ifdef _WIN32
    Sleep((DWORD)ms);
#else
    {
        struct timespec ts;
        ts.tv_sec = ms / 1000;
        ts.tv_nsec = (long)(ms % 1000) * 1000000L;
        if (nanosleep(&ts, &ts) == -1 && errno == EINTR)
            return;          /* Ctrl+C：不继续等了 */
    }
#endif
}

/* ==========================================================================
 * 命令行
 * ========================================================================== */

typedef struct {
    int keep;
    int no_open;
    int quiet;
    int delay_ms;
} Options;

static void usage(FILE *out, const char *argv0)
{
    fprintf(out,
        "%s %s —— %s\n"
        "\n"
        "用法：\n"
        "  %s [选项]\n"
        "\n"
        "数据文件（放在程序所在目录，不是当前目录）：\n"
        "  p.txt       每行一个学生姓名\n"
        "  class.txt   班级名称，取第一个非空行\n"
        "\n"
        "选项：\n"
        "  -keep            保留生成的临时 HTML 文件，不自动删除\n"
        "  -no-open         只生成 HTML，不自动打开浏览器\n"
        "  -delay <时间>    浏览器打开后等多久再删（默认 3s；可写 800ms / 8s / 8）\n"
        "  -quiet           安静模式，只输出临时文件路径\n"
        "  -h, --help       显示本帮助\n"
        "  -v, --version    显示版本\n",
        APP_NAME, APP_VERSION, APP_DESC, argv0);
}

/*
 * "8s" / "800ms" / "8"（裸数字按秒）。
 *
 * 上限 24 小时：strtol 溢出时会返回 LONG_MAX，后面再乘 1000 就是有符号溢出
 * （未定义行为）。所以必须在乘法【之前】卡住，不能先算再判断。
 */
static int parse_delay(const char *s, int *out_ms)
{
    char *end = NULL;
    long v = strtol(s, &end, 10);
    long ms;

    if (end == s || v < 0)
        return -1;

    if (*end == '\0' || strcmp(end, "s") == 0) {
        if (v > 86400L)
            return -1;
        ms = v * 1000L;
    } else if (strcmp(end, "ms") == 0) {
        if (v > 86400000L)
            return -1;
        ms = v;
    } else {
        return -1;
    }

    *out_ms = (int)ms;
    return 0;
}

/* ==========================================================================
 * main
 * ========================================================================== */

/* 生成 "DMING<random>.html" 的完整路径 */
static pchar *make_temp_path(const pchar *dir, int attempt)
{
    long r = (long)((time(NULL) * 1000 + attempt * 7919) % 100000000L);
    char name[64];
    snprintf(name, sizeof name, "DMING%ld.html", r);
#ifdef _WIN32
    {
        wchar_t wname[64];
        size_t k;
        for (k = 0; k <= strlen(name); k++)
            wname[k] = (wchar_t)(unsigned char)name[k];
        return pjoin(dir, wname);
    }
#else
    return pjoin(dir, name);
#endif
}

/* 在 dir 下写页面，成功返回路径，失败返回 NULL */
static pchar *write_page(const pchar *dir, const char *data, size_t len)
{
    int attempt;
    for (attempt = 0; attempt < MAX_TEMP_TRIES; attempt++) {
        pchar *path = make_temp_path(dir, attempt);
        FILE *f;
        if (p_exists(path)) {
            free(path);
            continue;
        }
        f = p_fopen_write(path);
        if (!f) {
            free(path);
            return NULL;      /* 目录不可写，别重试了 */
        }
        if (fwrite(data, 1, len, f) != len) {
            fclose(f);
            p_remove(path);
            free(path);
            return NULL;
        }
        fclose(f);
        return path;
    }
    return NULL;
}

int main(int argc, char **argv)
{
    Options opt;
    const char *argv0 = (argc > 0 && argv[0]) ? argv[0] : APP_NAME;
    pchar *dir = NULL, *list_path = NULL, *class_path = NULL, *page_path = NULL;
    char *list_text = NULL, *class_text = NULL;
    Names names;
    FirstLine fl;
    Buf page, students, cls;
    int i;

#ifdef _WIN32
    /* 让控制台按 UTF-8 显示中文（配合源码里的 UTF-8 字面量） */
    SetConsoleOutputCP(CP_UTF8);
#endif

    opt.keep = 0;
    opt.no_open = 0;
    opt.quiet = 0;
    opt.delay_ms = DEFAULT_DELAY_MS;

    for (i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (strcmp(a, "-keep") == 0 || strcmp(a, "--keep") == 0) {
            opt.keep = 1;
        } else if (strcmp(a, "-no-open") == 0 || strcmp(a, "--no-open") == 0) {
            opt.no_open = 1;
        } else if (strcmp(a, "-quiet") == 0 || strcmp(a, "--quiet") == 0) {
            opt.quiet = 1;
        } else if (strcmp(a, "-delay") == 0 || strcmp(a, "--delay") == 0) {
            if (i + 1 >= argc) {
                fputs("错误：-delay 后面要跟一个时间，例如 8s 或 800ms\n", stderr);
                return 2;
            }
            if (parse_delay(argv[++i], &opt.delay_ms) != 0) {
                fprintf(stderr, "错误：看不懂的时间 '%s'，用 8s / 800ms / 8\n",
                        argv[i]);
                return 2;
            }
        } else if (strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0) {
            usage(stdout, argv0);
            return 0;
        } else if (strcmp(a, "-v") == 0 || strcmp(a, "--version") == 0) {
            printf("%s %s\n", APP_NAME, APP_VERSION);
            return 0;
        } else {
            fprintf(stderr, "错误：未知选项 '%s'（用 -h 看用法）\n", a);
            return 2;
        }
    }

    /* ---- 数据目录 = 可执行文件所在目录 ---- */
    dir = exe_dir(argv0);
    if (!dir) {
        report_error("无法确定程序所在目录");
        return 1;
    }

    list_path = pjoin(dir, PLIT("p.txt"));
    class_path = pjoin(dir, PLIT("class.txt"));

    /* ---- 名单 ---- */
    list_text = read_file(list_path, NULL);
    if (!list_text) {
        char *u = p_to_utf8(list_path);
        report_error("未找到名单文件 %s", u ? u : "(路径无法显示)");
        free(u);
        return 1;
    }
    memset(&names, 0, sizeof names);
    if (split_lines(list_text, add_name_cb, &names) == 0) {
        char *u = p_to_utf8(list_path);
        report_error("名单文件 %s 是空的", u ? u : "(路径无法显示)");
        free(u);
        return 1;
    }

    /* ---- 班级 ---- */
    class_text = read_file(class_path, NULL);
    if (!class_text) {
        char *u = p_to_utf8(class_path);
        report_error("未找到班级文件 %s（请写入班级名称，例如 14）",
                     u ? u : "(路径无法显示)");
        free(u);
        return 1;
    }
    fl.value = NULL;
    split_lines(class_text, add_first_cb, &fl);
    if (!fl.value) {
        char *u = p_to_utf8(class_path);
        report_error("班级文件 %s 是空的（请写入班级名称，例如 14）",
                     u ? u : "(路径无法显示)");
        free(u);
        return 1;
    }

    if (!opt.quiet) {
        char *u = p_to_utf8(list_path);
        printf("已从 %s 读取 %lu 个姓名\n", u ? u : "?", (unsigned long)names.n);
        free(u);
        printf("班级：%s\n", fl.value);
    }

    /* ---- 渲染 ---- */
    buf_init(&students);
    buf_putc(&students, '[');
    for (i = 0; i < (int)names.n; i++) {
        if (i)
            buf_putc(&students, ',');
        buf_putc(&students, '"');
        json_escape(&students, names.v[i]);
        buf_putc(&students, '"');
    }
    buf_putc(&students, ']');

    buf_init(&cls);
    html_escape(&cls, fl.value);

    buf_init(&page);
    {
        Buf t1, t2;
        buf_init(&t1);
        render(&t1, PAGE_TEMPLATE, "{{CLASS}}", cls.p);
        buf_init(&t2);
        render(&t2, t1.p, "{{VERSION}}", APP_VERSION);
        render(&page, t2.p, "{{STUDENTS}}", students.p);
        buf_free(&t1);
        buf_free(&t2);
    }

    if (!opt.quiet)
        printf("正在生成临时页面...\n");

    /* ---- 写临时页：先试程序目录，不行退到系统临时目录 ---- */
    page_path = write_page(dir, page.p, page.len);
    if (!page_path) {
        pchar *td = temp_dir();
        if (td) {
            page_path = write_page(td, page.p, page.len);
            free(td);
        }
    }
    if (!page_path) {
        report_error("无法写入临时 HTML 文件（程序目录和系统临时目录都写不进去）");
        return 1;
    }

    if (!opt.quiet) {
        fputs("临时文件: ", stdout);
        fprint_path(stdout, page_path);
        fputc('\n', stdout);
    }

    if (opt.no_open) {
        fprint_path(stdout, page_path);
        fputc('\n', stdout);
        return 0;
    }

    if (open_browser(page_path) != 0) {
        if (!opt.quiet)
            fputs("未能自动打开浏览器，请手动打开上面的文件\n", stdout);
    } else if (!opt.quiet) {
        fputs("已调用系统默认浏览器打开\n", stdout);
    }

    if (opt.keep)
        return 0;

    sleep_ms(opt.delay_ms);
    p_remove(page_path);
    if (!opt.quiet)
        fputs("临时文件已删除\n", stdout);

    return 0;
}
