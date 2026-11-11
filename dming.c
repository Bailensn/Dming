#if !defined(_WIN32)
#  ifndef _DEFAULT_SOURCE
#    define _DEFAULT_SOURCE 1
#  endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

static const char PAGE_TEMPLATE[] =
	"<!DOCTYPE html>\n"
	"<html lang='zh-CN'>\n"
	"<head>\n"
	"<meta charset='UTF-8'>\n"
	"<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n"
	"<title>{{CLASS}}班点名</title>\n"
	"<style>\n"
	"* {\n"
	"margin: 0;\n"
	"padding: 0;\n"
	"box-sizing: border-box;\n"
	"}\n"
	"body {\n"
	"font-family: 'PingFang SC', 'Microsoft YaHei', 'Segoe UI', sans-serif;\n"
	"background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);\n"
	"overflow: hidden;\n"
	"height: 100vh;\n"
	"display: flex;\n"
	"justify-content: center;\n"
	"align-items: center;\n"
	"position: relative;\n"
	"}\n"
	".main-container {\n"
	"width: 100vw;\n"
	"height: 100vh;\n"
	"display: flex;\n"
	"flex-direction: column;\n"
	"justify-content: center;\n"
	"align-items: center;\n"
	"position: relative;\n"
	"z-index: 1;\n"
	"padding: 20px;\n"
	"}\n"
	".app-title {\n"
	"font-size: 36px;\n"
	"font-weight: 700;\n"
	"color: #fff;\n"
	"margin-bottom: 40px;\n"
	"text-align: center;\n"
	"letter-spacing: 2px;\n"
	"text-shadow: 2px 2px 4px rgba(0,0,0,0.2);\n"
	"}\n"
	".app-title span {\n"
	"color: #ffd93d;\n"
	"}\n"
	".name-card {\n"
	"width: 80vw;\n"
	"height: 30vh;\n"
	"max-width: 800px;\n"
	"min-height: 180px;\n"
	"background-color: #ffffff;\n"
	"border-radius: 25px;\n"
	"box-shadow: 0 20px 40px rgba(0, 0, 0, 0.1);\n"
	"display: flex;\n"
	"justify-content: center;\n"
	"align-items: center;\n"
	"margin-bottom: 50px;\n"
	"position: relative;\n"
	"overflow: hidden;\n"
	"transition: all 0.3s ease;\n"
	"border: none;\n"
	"}\n"
	".name-card:hover {\n"
	"transform: translateY(-5px);\n"
	"box-shadow: 0 25px 50px rgba(0, 0, 0, 0.15);\n"
	"}\n"
	".name-card.bounce {\n"
	"animation: bounce 0.5s cubic-bezier(0.68, -0.55, 0.265, 1.55);\n"
	"}\n"
	"@keyframes bounce {\n"
	"0% { transform: scale(1); }\n"
	"30% { transform: scale(1.01); }\n"
	"50% { transform: scale(0.995); }\n"
	"70% { transform: scale(1.005); }\n"
	"100% { transform: scale(1); }\n"
	"}\n"
	".name-text {\n"
	"font-size: 90px;\n"
	"font-weight: 800;\n"
	"text-align: center;\n"
	"white-space: nowrap;\n"
	"overflow: hidden;\n"
	"text-overflow: ellipsis;\n"
	"padding: 0 40px;\n"
	"letter-spacing: 2px;\n"
	"background: linear-gradient(\n"
	"90deg,\n"
	"#2c3e50 0%,\n"
	"#2c3e50 25%,\n"
	"#3498db 50%,\n"
	"#e74c3c 75%,\n"
	"#2c3e50 100%\n"
	");\n"
	"background-size: 200% auto;\n"
	"-webkit-background-clip: text;\n"
	"background-clip: text;\n"
	"-webkit-text-fill-color: transparent;\n"
	"animation: textShine 3s linear infinite;\n"
	"}\n"
	"@keyframes textShine {\n"
	"0% { background-position: 0% center; }\n"
	"100% { background-position: 200% center; }\n"
	"}\n"
	".roll-card {\n"
	"background: white;\n"
	"border-radius: 30px;\n"
	"box-shadow: 0 20px 40px rgba(0, 0, 0, 0.1);\n"
	"display: flex;\n"
	"align-items: center;\n"
	"justify-content: center;\n"
	"padding: 0;\n"
	"cursor: pointer;\n"
	"user-select: none;\n"
	"transition: all 0.3s ease;\n"
	"border: none;\n"
	"overflow: hidden;\n"
	"position: relative;\n"
	"}\n"
	".roll-card:hover {\n"
	"transform: translateY(-3px);\n"
	"box-shadow: 0 25px 50px rgba(0, 0, 0, 0.15);\n"
	"}\n"
	".roll-card:active {\n"
	"transform: translateY(1px);\n"
	"box-shadow: 0 10px 20px rgba(0, 0, 0, 0.1);\n"
	"}\n"
	".roll-icon {\n"
	"width: 30px;\n"
	"height: 30px;\n"
	"margin: 15px;\n"
	"color: #3498db;\n"
	"}\n"
	".roll-text {\n"
	"font-size: 20px;\n"
	"color: #000000;\n"
	"margin: 15px 15px 15px 0;\n"
	"font-weight: 600;\n"
	"}\n"
	".settings-btn {\n"
	"position: absolute;\n"
	"top: 40px;\n"
	"right: 40px;\n"
	"width: 20px;\n"
	"height: 20px;\n"
	"cursor: pointer;\n"
	"color: #7f8c8d;\n"
	"transition: all 0.3s ease;\n"
	"z-index: 10;\n"
	"}\n"
	".settings-btn:hover {\n"
	"color: #3498db;\n"
	"transform: scale(1.1);\n"
	"}\n"
	"@media (max-width: 1024px) {\n"
	".name-card { width: 85vw; height: 28vh; }\n"
	".name-text { font-size: 80px; }\n"
	"}\n"
	"@media (max-width: 768px) {\n"
	".name-card { width: 90vw; height: 25vh; border-radius: 20px; }\n"
	".name-text { font-size: 60px; padding: 0 20px; }\n"
	".app-title { font-size: 28px; margin-bottom: 30px; }\n"
	".roll-text { font-size: 18px; }\n"
	"}\n"
	"@media (max-width: 480px) {\n"
	".name-card { width: 95vw; height: 22vh; border-radius: 15px; }\n"
	".name-text { font-size: 48px; }\n"
	".app-title { font-size: 24px; margin-bottom: 20px; }\n"
	".roll-text { font-size: 16px; }\n"
	".settings-btn { top: 20px; right: 20px; }\n"
	"}\n"
	"@media (max-height: 600px) {\n"
	".name-card { height: 35vh; margin-bottom: 30px; }\n"
	".name-text { font-size: 70px; }\n"
	"}\n"
	".toast {\n"
	"position: fixed;\n"
	"bottom: 30px;\n"
	"left: 50%;\n"
	"transform: translateX(-50%) translateY(100px);\n"
	"background: rgba(44, 62, 80, 0.95);\n"
	"color: white;\n"
	"padding: 15px 30px;\n"
	"border-radius: 25px;\n"
	"font-size: 16px;\n"
	"font-weight: 500;\n"
	"opacity: 0;\n"
	"transition: transform 0.3s, opacity 0.3s;\n"
	"z-index: 1000;\n"
	"box-shadow: 0 10px 25px rgba(0, 0, 0, 0.2);\n"
	"backdrop-filter: blur(10px);\n"
	"}\n"
	".toast.show {\n"
	"transform: translateX(-50%) translateY(0);\n"
	"opacity: 1;\n"
	"}\n"
	".loading {\n"
	"display: inline-block;\n"
	"width: 20px;\n"
	"height: 20px;\n"
	"border: 3px solid rgba(52, 152, 219, 0.3);\n"
	"border-radius: 50%;\n"
	"border-top-color: #3498db;\n"
	"animation: spin 1s ease-in-out infinite;\n"
	"margin-right: 10px;\n"
	"}\n"
	"@keyframes spin {\n"
	"to { transform: rotate(360deg); }\n"
	"}\n"
	".controls {\n"
	"display: flex;\n"
	"gap: 20px;\n"
	"margin-top: 20px;\n"
	"}\n"
	".control-btn {\n"
	"padding: 12px 30px;\n"
	"font-size: 18px;\n"
	"border: none;\n"
	"border-radius: 12px;\n"
	"background: #fff;\n"
	"color: #333;\n"
	"cursor: pointer;\n"
	"transition: all 0.3s ease;\n"
	"font-weight: 600;\n"
	"box-shadow: 0 4px 15px rgba(0, 0, 0, 0.1);\n"
	"}\n"
	".control-btn:hover {\n"
	"transform: translateY(-2px);\n"
	"box-shadow: 0 6px 20px rgba(0, 0, 0, 0.15);\n"
	"}\n"
	".control-btn:active {\n"
	"transform: translateY(1px);\n"
	"}\n"
	"#startStopBtn {\n"
	"background: linear-gradient(135deg, #3498db, #2980b9);\n"
	"color: white;\n"
	"}\n"
	"#resetBtn {\n"
	"background: linear-gradient(135deg, #e74c3c, #c0392b);\n"
	"color: white;\n"
	"}\n"
	".info-text {\n"
	"color: rgba(255, 255, 255, 0.8);\n"
	"margin-top: 20px;\n"
	"font-size: 14px;\n"
	"}\n"
	"</style>\n"
	"</head>\n"
	"<body>\n"
	"<div class='main-container'>\n"
	"<span style='color: rgba(201, 208, 244, 1.0); text-align: center;'>version {{VERSION}}</span>\n"
	"<h1 class='app-title'><span id='classNum'>{{CLASS}}</span>班点名</h1>\n"
	"<div class='name-card' id='nameCard'>\n"
	"<div class='name-text' id='currentName'>准备就绪</div>\n"
	"</div>\n"
	"<div class='controls'>\n"
	"<button class='control-btn' id='startStopBtn'>开始点名</button>\n"
	"<button class='control-btn' id='resetBtn'>重置名单</button>\n"
	"</div>\n"
	"<div class='info-text' id='remainText'>剩余 0 / 0 人</div>\n"
	"<div style='color: rgba(255, 255, 255, 0.6); font-size: 14px; margin-top: 20px; text-align: center;'>\n"
	"by 白异山\n"
	"</div>\n"
	"</div>\n"
	"<div class='toast' id='toast'></div>\n"
	"<script>\n"
	"let originalStudents = {{STUDENTS}};\n"
	"function rosterHash(list) {\n"
	"let h = 5381;\n"
	"const s = list.slice().sort().join('\\u0001');\n"
	"for (let i = 0; i < s.length; i++) h = ((h * 33) ^ s.charCodeAt(i)) >>> 0;\n"
	"return h.toString(36);\n"
	"}\n"
	"const STORAGE_KEY = 'dming.picked.v1.' + rosterHash(originalStudents);\n"
	"const STATE_TTL_MS = {{STATE_TTL}};\n"
	"const TTL_TEXT = STATE_TTL_MS >= 3600000 ? Math.round(STATE_TTL_MS / 3600000) + ' 小时' : Math.round(STATE_TTL_MS / 60000) + ' 分钟';\n"
	"let stateExpired = false;\n"
	"function loadState() {\n"
	"try {\n"
	"const raw = localStorage.getItem(STORAGE_KEY);\n"
	"if (!raw) return null;\n"
	"const st = JSON.parse(raw);\n"
	"if (!st || !Array.isArray(st.picked) || typeof st.t !== 'number') return null;\n"
	"if (Date.now() - st.t > STATE_TTL_MS) {\n"
	"stateExpired = true;\n"
	"localStorage.removeItem(STORAGE_KEY);\n"
	"return null;\n"
	"}\n"
	"return st.picked;\n"
	"} catch (e) { return null; }\n"
	"}\n"
	"let picked = new Set((loadState() || []).filter(function (n) { return originalStudents.indexOf(n) !== -1; }));\n"
	"function savePicked() {\n"
	"try { localStorage.setItem(STORAGE_KEY, JSON.stringify({ t: Date.now(), picked: Array.from(picked) })); } catch (e) {}\n"
	"}\n"
	"function touchTime() {\n"
	"try {\n"
	"const raw = localStorage.getItem(STORAGE_KEY);\n"
	"const st = raw ? JSON.parse(raw) : null;\n"
	"if (st && Array.isArray(st.picked)) {\n"
	"st.t = Date.now();\n"
	"localStorage.setItem(STORAGE_KEY, JSON.stringify(st));\n"
	"} else if (picked.size > 0) {\n"
	"savePicked();\n"
	"}\n"
	"} catch (e) {}\n"
	"}\n"
	"function syncFromStorage() {\n"
	"if (!storageOk) return;\n"
	"const latest = loadState();\n"
	"if (latest === null) return;\n"
	"picked = new Set(latest.filter(function (n) { return originalStudents.indexOf(n) !== -1; }));\n"
	"students = shuffleArray(originalStudents.filter(function (n) { return !picked.has(n); }));\n"
	"if (currentIndex >= students.length) currentIndex = 0;\n"
	"updateRemain();\n"
	"}\n"
	"touchTime();\n"
	"let students = originalStudents.filter(function (n) { return !picked.has(n); });\n"
	"let storageOk = true;\n"
	"try { localStorage.setItem('dming.probe', '1'); localStorage.removeItem('dming.probe'); } catch (e) { storageOk = false; }\n"
	"let currentIndex = 0;\n"
	"let isRolling = false;\n"
	"let rollInterval = null;\n"
	"let rollSpeed = 5;\n"
	"const nameDisplay = document.getElementById('currentName');\n"
	"const nameCard = document.getElementById('nameCard');\n"
	"const startStopBtn = document.getElementById('startStopBtn');\n"
	"const resetBtn = document.getElementById('resetBtn');\n"
	"const remainText = document.getElementById('remainText');\n"
	"function shuffleArray(array) {\n"
	"for (let i = array.length - 1; i > 0; i--) {\n"
	"const j = Math.floor(Math.random() * (i + 1));\n"
	"[array[i], array[j]] = [array[j], array[i]];\n"
	"}\n"
	"return array;\n"
	"}\n"
	"function updateRemain() {\n"
	"remainText.textContent = '剩余 ' + students.length + ' / ' + originalStudents.length + ' 人' + (picked.size > 0 ? '（已点 ' + picked.size + ' 人）' : '') + (storageOk ? '' : '｜浏览器不允许本地记录，重开页面会清零');\n"
	"}\n"
	"function resetStudents() {\n"
	"if (isRolling) {\n"
	"clearInterval(rollInterval);\n"
	"isRolling = false;\n"
	"startStopBtn.textContent = '开始点名';\n"
	"startStopBtn.style.background = 'linear-gradient(135deg, #3498db, #2980b9)';\n"
	"}\n"
	"picked.clear();\n"
	"savePicked();\n"
	"students = shuffleArray([...originalStudents]);\n"
	"currentIndex = 0;\n"
	"nameDisplay.textContent = '准备就绪';\n"
	"updateRemain();\n"
	"}\n"
	"function autoNewRound() {\n"
	"picked.clear();\n"
	"savePicked();\n"
	"students = shuffleArray([...originalStudents]);\n"
	"currentIndex = 0;\n"
	"updateRemain();\n"
	"}\n"
	"function showToast(msg) {\n"
	"const toast = document.getElementById('toast');\n"
	"toast.textContent = msg;\n"
	"toast.classList.add('show');\n"
	"clearTimeout(showToast.timer);\n"
	"showToast.timer = setTimeout(() => {\n"
	"toast.classList.remove('show');\n"
	"}, 1800);\n"
	"}\n"
	"students = shuffleArray([...students]);\n"
	"updateRemain();\n"
	"resetBtn.addEventListener('click', function() {\n"
	"resetStudents();\n"
	"showToast('名单已重置，共 ' + students.length + ' 人');\n"
	"});\n"
	"startStopBtn.addEventListener('click', function() {\n"
	"touchTime();\n"
	"if (isRolling) {\n"
	"clearInterval(rollInterval);\n"
	"startStopBtn.textContent = '开始点名';\n"
	"startStopBtn.style.background = 'linear-gradient(135deg, #3498db, #2980b9)';\n"
	"isRolling = false;\n"
	"syncFromStorage();\n"
	"if (students.length === 0 && originalStudents.length > 0) {\n"
	"autoNewRound();\n"
	"}\n"
	"if (currentIndex >= students.length) currentIndex = 0;\n"
	"const selectedName = students[currentIndex];\n"
	"nameDisplay.textContent = selectedName;\n"
	"nameCard.classList.add('bounce');\n"
	"setTimeout(() => {\n"
	"nameCard.classList.remove('bounce');\n"
	"}, 300);\n"
	"if (selectedName && students.length > 0) {\n"
	"const indexToRemove = students.indexOf(selectedName);\n"
	"if (indexToRemove !== -1) {\n"
	"students.splice(indexToRemove, 1);\n"
	"picked.add(selectedName);\n"
	"savePicked();\n"
	"if (students.length === 0) {\n"
	"autoNewRound();\n"
	"showToast('全部点完，名单已重置');\n"
	"}\n"
	"if (currentIndex >= students.length) {\n"
	"currentIndex = 0;\n"
	"}\n"
	"}\n"
	"updateRemain();\n"
	"}\n"
	"} else {\n"
	"syncFromStorage();\n"
	"if (originalStudents.length === 0) {\n"
	"showToast('该名单无效，请重新获取');\n"
	"return;\n"
	"}\n"
	"if (students.length === 0) {\n"
	"autoNewRound();\n"
	"}\n"
	"currentIndex = 0;\n"
	"startStopBtn.textContent = '停止点名';\n"
	"startStopBtn.style.background = 'linear-gradient(135deg, #e74c3c, #c0392b)';\n"
	"isRolling = true;\n"
	"rollInterval = setInterval(() => {\n"
	"if (students.length === 0) return;\n"
	"nameDisplay.textContent = students[currentIndex];\n"
	"currentIndex = (currentIndex + 1) % students.length;\n"
	"}, rollSpeed);\n"
	"}\n"
	"});\n"
	"window.addEventListener('storage', function (e) {\n"
	"if (e.key !== STORAGE_KEY) return;\n"
	"syncFromStorage();\n"
	"});\n"
	"window.addEventListener('load', function() {\n"
	"updateRemain();\n"
	"if (stateExpired) {\n"
	"showToast('上次记录已过期（超过 ' + TTL_TEXT + '），已开新一轮');\n"
	"} else if (originalStudents.length === 0) {\n"
	"alert('该名单无效，请重新获取');\n"
	"} else if (students.length === 0) {\n"
	"autoNewRound();\n"
	"showToast('全部点完，名单已重置');\n"
	"} else if (picked.size > 0) {\n"
	"showToast('已恢复上次进度：已点 ' + picked.size + ' 人');\n"
	"}\n"
	"});\n"
	"setInterval(touchTime, 60000);\n"
	"let pingFailed = false;\n"
	"function pingServer() {\n"
	"if (location.protocol !== 'http:') return;\n"
	"fetch('/__ping', { cache: 'no-store' }).then(function (r) {\n"
	"if (!r.ok) throw new Error('bad status');\n"
	"pingFailed = false;\n"
	"}).catch(function () {\n"
	"if (pingFailed) return;\n"
	"pingFailed = true;\n"
	"showToast('本地服务已停止，刷新页面需要重新运行 dming');\n"
	"});\n"
	"}\n"
	"pingServer();\n"
	"setInterval(pingServer, 30000);\n"
	"document.addEventListener('visibilitychange', function () {\n"
	"if (document.hidden) return;\n"
	"syncFromStorage();\n"
	"touchTime();\n"
	"pingServer();\n"
	"});\n"
	"</script>\n"
	"</body>\n"
	"</html>\n"
;

#define APP_NAME    "dming"
#define APP_VERSION "4.1.3"
#define APP_DESC    "课堂随机点名器"

#define DEFAULT_STATE_TTL_MS (7 * 60 * 1000)
#define MAX_TEMP_TRIES   32

/* 页面通过 http://127.0.0.1:<端口>/ 提供。localStorage 按 origin 隔离，
 * 所以「端口固定」是点名进度不丢的前提：端口一变 origin 就变，等于换了台机器。
 * 只在 loopback 上监听，不触达局域网，也不会弹防火墙授权。 */
#define DEFAULT_PORT      47123
#define DEFAULT_BEAT_MS   (5 * 60 * 1000)
#define CONN_RCV_TIMEOUT  200
#define CONN_SND_TIMEOUT  5000
#define MAX_REQ_BYTES     8192

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  include <windows.h>
#  include <shellapi.h>
#  include <wchar.h>
typedef wchar_t pchar;
#  define PLIT_(s) L##s
#  define PATH_SEP L'\\'
#else
#  include <sys/types.h>
#  include <unistd.h>
#  include <sys/stat.h>
#  include <sys/wait.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <signal.h>
#  include <errno.h>
#  include <limits.h>
typedef char pchar;
#  define PLIT_(s) s
#  define PATH_SEP '/'
#endif
#define PLIT(s) PLIT_(s)

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

#ifndef _WIN32

static int strip_last_segment(char *p)
{
    char *s = strrchr(p, '/');
    if (!s)
        return -1;
    if (s == p)
        s[1] = '\0';
    else
        *s = '\0';
    return 0;
}

static pchar *dir_from_argv0(const char *a0)
{
    if (!a0 || !*a0)
        return NULL;

    if (strchr(a0, '/')) {

        char real[PATH_MAX];
        if (!realpath(a0, real))
            return NULL;
        if (strip_last_segment(real) != 0)
            return NULL;
        return pdup(real);
    }

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

    (void)argv0;

    n = GetModuleFileNameW(NULL, buf, (DWORD)(sizeof buf / sizeof buf[0]));
    if (n == 0 || n >= (DWORD)(sizeof buf / sizeof buf[0]))
        return NULL;
    s = wcsrchr(buf, L'\\');
    if (!s)
        return NULL;
    *s = L'\0';
    return pdup(buf);
#else
    {
        char buf[4096];
        ssize_t n = readlink("/proc/self/exe", buf, sizeof buf - 1);
        if (n > 0) {
            const char *base;
            buf[n] = '\0';
            base = strrchr(buf, '/');
            base = base ? base + 1 : buf;

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

static pchar *temp_dir(void)
{
#ifdef _WIN32
    wchar_t buf[MAX_PATH + 2];
    DWORD n = GetTempPathW((DWORD)(sizeof buf / sizeof buf[0]), buf);
    if (n == 0 || n > (DWORD)(sizeof buf / sizeof buf[0]))
        return NULL;

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

static int is_blank(unsigned char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' ||
           c == '\v' || c == '\f';
}

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

#ifdef _WIN32
static int open_browser(const pchar *path)
{
    HINSTANCE r = ShellExecuteW(NULL, L"open", path, NULL, NULL, SW_SHOWNORMAL);
    return ((INT_PTR)r > 32) ? 0 : -1;
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
        _exit(127);
    }

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
    char *p = (char *)path;
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

/* ------------------------------------------------------------------ *
 * 本地页面服务（只监听 127.0.0.1）
 *
 * 为什么值得用 socket 换掉 file://：页面里的点名进度存在 localStorage，
 * 而 localStorage 按 origin 隔离。file:// 下各浏览器行为不一致（有的把每个
 * 文件当独立 origin，有的干脆禁用），http origin 才是所有内核都认的稳定身份。
 * 代价是端口必须固定，见 DEFAULT_PORT。
 * ------------------------------------------------------------------ */

#ifdef _WIN32
typedef SOCKET sock_t;
#  define SOCK_BAD  INVALID_SOCKET
#  define sock_close closesocket
/* 老 SDK / Win9x 目标下 winsock2.h 可能不给这个宏，缺了就用官方等价定义 */
#  ifndef SO_EXCLUSIVEADDRUSE
#    define SO_EXCLUSIVEADDRUSE ((int)(~SO_REUSEADDR))
#  endif
#else
typedef int sock_t;
#  define SOCK_BAD  (-1)
#  define sock_close close
#endif

static int net_global_init(void)
{
#ifdef _WIN32
    WSADATA w;
    return WSAStartup(MAKEWORD(2, 2), &w) == 0 ? 0 : -1;
#else
    /* 浏览器中途关闭连接时不能让进程被 SIGPIPE 打死 */
    signal(SIGPIPE, SIG_IGN);
    return 0;
#endif
}

static void net_global_fini(void)
{
#ifdef _WIN32
    WSACleanup();
#endif
}

static pchar *p_from_ascii(const char *s)
{
#ifdef _WIN32
    size_t n = strlen(s), i;
    wchar_t *w = (wchar_t *)xmalloc((n + 1) * sizeof(wchar_t));
    for (i = 0; i < n; i++)
        w[i] = (wchar_t)(unsigned char)s[i];
    w[n] = 0;
    return w;
#else
    return pdup(s);
#endif
}

static void sock_timeout(sock_t s)
{
#ifdef _WIN32
    DWORD rcv = CONN_RCV_TIMEOUT, snd = CONN_SND_TIMEOUT;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&rcv, sizeof rcv);
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char *)&snd, sizeof snd);
#else
    struct timeval tv;
    tv.tv_sec = CONN_RCV_TIMEOUT / 1000;
    tv.tv_usec = (long)(CONN_RCV_TIMEOUT % 1000) * 1000L;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
    tv.tv_sec = CONN_SND_TIMEOUT / 1000;
    tv.tv_usec = (long)(CONN_SND_TIMEOUT % 1000) * 1000L;
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof tv);
#endif
}

static int send_all(sock_t s, const char *buf, size_t len)
{
    while (len > 0) {
        int chunk = (len > 65536) ? 65536 : (int)len;
        int n = send(s, buf, chunk, 0);
        if (n <= 0)
            return -1;
        buf += n;
        len -= (size_t)n;
    }
    return 0;
}

static int http_reply(sock_t s, int code, const char *reason,
                      const char *ctype, const char *body, size_t blen,
                      int head_only)
{
    char head[384];
    int hl = snprintf(head, sizeof head,
                      "HTTP/1.1 %d %s\r\n"
                      "Content-Type: %s\r\n"
                      "Content-Length: %lu\r\n"
                      "Cache-Control: no-store\r\n"
                      "Connection: close\r\n"
                      "\r\n",
                      code, reason, ctype, (unsigned long)blen);

    if (hl <= 0 || (size_t)hl >= sizeof head)
        return -1;
    if (send_all(s, head, (size_t)hl) != 0)
        return -1;
    if (!head_only && body && blen > 0 && send_all(s, body, blen) != 0)
        return -1;
    return 0;
}

static void serve_page(sock_t c, const pchar *page_path, int head_only)
{
    size_t len = 0;
    char *body = read_file(page_path, &len);

    if (!body) {
        static const char msg[] = "页面文件读不到，请重新运行 dming\n";
        http_reply(c, 500, "Internal Server Error", "text/plain; charset=utf-8",
                   msg, sizeof msg - 1, head_only);
        return;
    }
    /* 不加 Access-Control-Allow-Origin：别的站点就算能发请求也读不到响应 */
    http_reply(c, 200, "OK", "text/html; charset=utf-8", body, len, head_only);
    free(body);
}

static void serve_whoami(sock_t c, const pchar *page_path, int head_only)
{
    char *u = p_to_utf8(page_path);
    if (!u) {
        http_reply(c, 500, "Internal Server Error", "text/plain; charset=utf-8",
                   "?", 1, head_only);
        return;
    }
    http_reply(c, 200, "OK", "text/plain; charset=utf-8", u, strlen(u),
               head_only);
    free(u);
}

static void handle_conn(sock_t c, const pchar *page_path)
{
    char req[MAX_REQ_BYTES];
    char method[8], target[512];
    size_t got = 0;
    char *q;
    int head;

    for (;;) {
        int n;
        if (got + 1 >= sizeof req)
            break;
        n = recv(c, req + got, (int)(sizeof req - 1 - got), 0);
        if (n <= 0)
            break;              /* 超时或对端已关，就用已经收到的部分 */
        got += (size_t)n;
        req[got] = '\0';
        if (strstr(req, "\r\n\r\n") || strstr(req, "\n\n"))
            break;
    }
    req[got] = '\0';
    if (got == 0)
        return;                 /* 浏览器的预连接，什么都不发 */

    method[0] = target[0] = '\0';
    if (sscanf(req, "%7s %511s", method, target) != 2)
        return;

    q = strchr(target, '?');
    if (q)
        *q = '\0';

    if (strcmp(method, "GET") != 0 && strcmp(method, "HEAD") != 0) {
        http_reply(c, 405, "Method Not Allowed", "text/plain; charset=utf-8",
                   "GET only\n", 9, 0);
        return;
    }
    head = (strcmp(method, "HEAD") == 0);
    if (strcmp(target, "/favicon.ico") == 0) {
        http_reply(c, 204, "No Content", "text/plain; charset=utf-8", NULL, 0,
                   0);
        return;
    }
    if (strcmp(target, "/__ping") == 0) {
        /* 页面每 30s 打一次；只要请求进来就算「页面还在」 */
        http_reply(c, 204, "No Content", "text/plain; charset=utf-8", NULL, 0,
                   0);
        return;
    }
    if (strcmp(target, "/__whoami") == 0) {
        serve_whoami(c, page_path, head);
        return;
    }
    if (strcmp(target, "/") == 0 || strcmp(target, "/index.html") == 0) {
        serve_page(c, page_path, head);
        return;
    }
    http_reply(c, 404, "Not Found", "text/plain; charset=utf-8", "not found\n",
               10, 0);
}

static sock_t listen_loopback(int port)
{
    sock_t s;
    struct sockaddr_in a;
    int one = 1;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == SOCK_BAD)
        return SOCK_BAD;

#ifdef _WIN32
    /* Windows 上 SO_REUSEADDR 会允许两个进程抢同一个端口，反而破坏复用检测 */
    setsockopt(s, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char *)&one, sizeof one);
#else
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
#endif

    memset(&a, 0, sizeof a);
    a.sin_family = AF_INET;
    a.sin_port = htons((unsigned short)port);
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(s, (struct sockaddr *)&a, sizeof a) != 0 || listen(s, 16) != 0) {
        sock_close(s);
        return SOCK_BAD;
    }
    return s;
}

/* 端口上是不是有个活着的 dming？只做一次 connect 探测。 */
static int probe_local(int port)
{
    sock_t s;
    struct sockaddr_in a;
    int ok;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == SOCK_BAD)
        return 0;
    memset(&a, 0, sizeof a);
    a.sin_family = AF_INET;
    a.sin_port = htons((unsigned short)port);
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ok = (connect(s, (struct sockaddr *)&a, sizeof a) == 0);
    sock_close(s);
    return ok;
}

static char *http_get_local(int port, const char *path, size_t *out_len)
{
    sock_t s;
    struct sockaddr_in a;
    char req[256];
    char *buf = NULL;
    size_t cap = 0, len = 0;
    char *hdr;
    int rl;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == SOCK_BAD)
        return NULL;
    memset(&a, 0, sizeof a);
    a.sin_family = AF_INET;
    a.sin_port = htons((unsigned short)port);
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(s, (struct sockaddr *)&a, sizeof a) != 0) {
        sock_close(s);
        return NULL;
    }
    sock_timeout(s);

    rl = snprintf(req, sizeof req,
                  "GET %s HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n",
                  path);
    if (rl <= 0 || send_all(s, req, (size_t)rl) != 0) {
        sock_close(s);
        return NULL;
    }

    for (;;) {
        char tmp[1024];
        int n = recv(s, tmp, (int)sizeof tmp, 0);
        if (n <= 0)
            break;
        if (len + (size_t)n + 1 > cap) {
            cap = (len + (size_t)n + 1) * 2;
            buf = xrealloc(buf, cap);
        }
        memcpy(buf + len, tmp, (size_t)n);
        len += (size_t)n;
    }
    sock_close(s);

    if (!buf || len == 0) {
        free(buf);
        return NULL;
    }
    buf[len] = '\0';

    /* 只有 200 才算「那边确实是个 dming」；别的程序占着端口时这里就返回 NULL */
    if (strncmp(buf, "HTTP/1.", 7) != 0 || buf[9] != '2' || buf[10] != '0' ||
        buf[11] != '0') {
        free(buf);
        return NULL;
    }

    hdr = strstr(buf, "\r\n\r\n");
    if (!hdr) {
        free(buf);
        return NULL;
    }
    hdr += 4;
    if (*hdr == '\0') {
        free(buf);
        return NULL;
    }
    len -= (size_t)(hdr - buf);
    memmove(buf, hdr, len);
    buf[len] = '\0';
    if (out_len)
        *out_len = len;
    return buf;
}

static int serve_loop(sock_t ls, const pchar *page_path, int beat_ms)
{
    time_t last = time(NULL);

    for (;;) {
        fd_set rd;
        struct timeval tv;
        int wait_ms, rc;
        sock_t c;

        if (beat_ms > 0) {
            long idle = (long)(time(NULL) - last) * 1000L;
            wait_ms = beat_ms - (int)idle;
            if (wait_ms <= 0)
                return 0;       /* 页面不在了（心跳断了），自己收摊 */
        } else {
            wait_ms = 1000;
        }
        if (wait_ms > 1000)
            wait_ms = 1000;

        FD_ZERO(&rd);
        FD_SET(ls, &rd);
        tv.tv_sec = wait_ms / 1000;
        tv.tv_usec = (long)(wait_ms % 1000) * 1000L;

        rc = select((int)(ls + 1), &rd, NULL, NULL, &tv);
        if (rc < 0) {
#ifndef _WIN32
            if (errno == EINTR)
                continue;
#endif
            return -1;
        }
        if (rc == 0)
            continue;

        c = accept(ls, NULL, NULL);
        if (c == SOCK_BAD)
            continue;
        last = time(NULL);
        sock_timeout(c);
        handle_conn(c, page_path);
        sock_close(c);
    }
}

typedef struct {
    int no_open;
    int quiet;
    int port;
    int beat_ms;
    int state_ttl_ms;
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
        "页面地址（在浏览器里打开这个）：\n"
        "  http://127.0.0.1:%d/\n"
        "\n"
        "点名进度保存在这个地址的 localStorage 里，所以端口要保持不变。\n"
        "\n"
        "选项：\n"
        "  -no-open          不自动打开浏览器（仍然起本地服务，并打印地址）\n"
        "  -port <端口>      本地服务端口（默认 %d；换端口会让上次的记录失联）\n"
        "  -heartbeat <时间> 页面心跳断了多久就停止服务（默认 5m；0 表示一直服务）\n"
        "  -state-ttl <时间> 上次点名记录的有效期（默认 7m；可写 45m / 90m / 7200s）\n"
        "  -quiet            安静模式，只输出页面地址\n"
        "  -h, --help        显示本帮助\n"
        "  -v, --version     显示版本\n",
        APP_NAME, APP_VERSION, APP_DESC, argv0,
        DEFAULT_PORT, DEFAULT_PORT);
}

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
    } else if (strcmp(end, "m") == 0) {
        if (v > 1440L)
            return -1;
        ms = v * 60L * 1000L;
    } else if (strcmp(end, "h") == 0) {
        if (v > 24L)
            return -1;
        ms = v * 3600L * 1000L;
    } else {
        return -1;
    }

    *out_ms = (int)ms;
    return 0;
}

static pchar *make_temp_path(const pchar *dir, int attempt)
{
    char name[64];
    if (attempt == 0) {
        snprintf(name, sizeof name, "DMING.html");
    } else {
        long r = (long)((time(NULL) * 1000 + attempt * 7919) % 100000000L);
        snprintf(name, sizeof name, "DMING%ld.html", r);
    }
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

static pchar *write_page(const pchar *dir, const char *data, size_t len)
{
    int attempt;
    for (attempt = 0; attempt < MAX_TEMP_TRIES; attempt++) {
        pchar *path = make_temp_path(dir, attempt);
        FILE *f;

        if (attempt > 0 && p_exists(path)) {
            free(path);
            continue;
        }
        f = p_fopen_write(path);
        if (!f) {
            free(path);
            return NULL;
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
    sock_t ls;
    char url[80];
    int i;

#ifdef _WIN32

    SetConsoleOutputCP(CP_UTF8);
#endif

    opt.no_open = 0;
    opt.quiet = 0;
    opt.port = DEFAULT_PORT;
    opt.beat_ms = DEFAULT_BEAT_MS;
    opt.state_ttl_ms = DEFAULT_STATE_TTL_MS;

    for (i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (strcmp(a, "-no-open") == 0 || strcmp(a, "--no-open") == 0) {
            opt.no_open = 1;
        } else if (strcmp(a, "-quiet") == 0 || strcmp(a, "--quiet") == 0) {
            opt.quiet = 1;
        } else if (strcmp(a, "-port") == 0 || strcmp(a, "--port") == 0) {
            char *end = NULL;
            long v;
            if (i + 1 >= argc) {
                fputs("错误：-port 后面要跟端口号，例如 47123\n", stderr);
                return 2;
            }
            v = strtol(argv[++i], &end, 10);
            if (end == argv[i] || *end != '\0' || v < 1 || v > 65535) {
                fprintf(stderr, "错误：端口 '%s' 不合法（1-65535）\n", argv[i]);
                return 2;
            }
            opt.port = (int)v;
        } else if (strcmp(a, "-heartbeat") == 0 || strcmp(a, "--heartbeat") == 0 ||
                   strcmp(a, "-idle") == 0 || strcmp(a, "--idle") == 0) {
            if (i + 1 >= argc) {
                fputs("错误：-heartbeat 后面要跟一个时间，例如 5m 或 0\n", stderr);
                return 2;
            }
            if (parse_delay(argv[++i], &opt.beat_ms) != 0) {
                fputs("错误：-heartbeat 的时间格式不对（例如 30s / 5m / 0）\n", stderr);
                return 2;
            }
        } else if (strcmp(a, "-state-ttl") == 0 || strcmp(a, "--state-ttl") == 0) {
            if (i + 1 >= argc) {
                fputs("错误：-state-ttl 后面要跟一个时间，例如 45m 或 2h\n", stderr);
                return 2;
            }
            if (parse_delay(argv[++i], &opt.state_ttl_ms) != 0) {
                fputs("错误：-state-ttl 的时间格式不对（例如 45m / 90m / 2h）\n", stderr);
                return 2;
            }
        } else if (strcmp(a, "-delay") == 0 || strcmp(a, "--delay") == 0 ||
                   strcmp(a, "-keep") == 0 || strcmp(a, "--keep") == 0) {
            fputs("提示：页面不再自动删除，-delay / -keep 已废弃（忽略）\n", stderr);
            if ((strcmp(a, "-delay") == 0 || strcmp(a, "--delay") == 0) &&
                i + 1 < argc) {
                i++;
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

    dir = exe_dir(argv0);
    if (!dir) {
        report_error("无法确定程序所在目录");
        return 1;
    }

    list_path = pjoin(dir, PLIT("p.txt"));
    class_path = pjoin(dir, PLIT("class.txt"));

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
        Buf t3;
        char ttl[32];
        render(&t2, t1.p, "{{VERSION}}", APP_VERSION);
        snprintf(ttl, sizeof ttl, "%d", opt.state_ttl_ms);
        buf_init(&t3);
        render(&t3, t2.p, "{{STATE_TTL}}", ttl);
        render(&page, t3.p, "{{STUDENTS}}", students.p);
        buf_free(&t1);
        buf_free(&t2);
        buf_free(&t3);
    }

    if (!opt.quiet)
        printf("正在生成页面...\n");

    page_path = write_page(dir, page.p, page.len);
    if (!page_path) {
        pchar *td = temp_dir();
        if (td) {
            page_path = write_page(td, page.p, page.len);
            free(td);
        }
    }
    if (!page_path) {
        report_error("无法写入页面文件（程序目录和系统临时目录都写不进去）");
        return 1;
    }

    snprintf(url, sizeof url, "http://127.0.0.1:%d/", opt.port);

    if (net_global_init() != 0) {
        report_error("无法初始化网络");
        return 1;
    }

    ls = listen_loopback(opt.port);
    if (ls == SOCK_BAD) {
        /* 端口被占：可能是上一次的 dming 还在服务，那就复用它的 origin，
         * 但只有「服务的是同一个页面文件」时复用才是对的。 */
        char *mine = NULL, *theirs = NULL;
        size_t tlen = 0;
        int same = 0;

        if (probe_local(opt.port)) {
            theirs = http_get_local(opt.port, "/__whoami", &tlen);
            mine = p_to_utf8(page_path);
            same = (mine && theirs && strcmp(mine, theirs) == 0);
        }
        if (same) {
            if (!opt.quiet) {
                printf("%s 已经在运行（端口 %d），直接复用它的记录。\n",
                       APP_NAME, opt.port);
                printf("地址: %s\n", url);
            } else {
                printf("%s\n", url);
            }
            if (!opt.no_open) {
                pchar *purl = p_from_ascii(url);
                open_browser(purl);
                free(purl);
            }
            free(mine);
            free(theirs);
            net_global_fini();
            return 0;
        }
        if (theirs) {
            report_error("端口 %d 上运行的 dming 服务的是另一个页面：%s\n"
                         "先把那个窗口关掉，或换端口：-port 47124（换端口会让上次的记录失联）",
                         opt.port, theirs);
        } else {
            report_error("端口 %d 已被占用（不是 dming）。换一个：-port 47124"
                         "（换端口会让上次的点名记录失联）", opt.port);
        }
        free(mine);
        free(theirs);
        net_global_fini();
        return 1;
    }

    if (!opt.quiet) {
        fputs("页面文件: ", stdout);
        fprint_path(stdout, page_path);
        fputc('\n', stdout);
        printf("地址: %s\n", url);
        fputs("点名进度存在这个地址下。页面每 30 秒发一次心跳，\n"
              "关掉页面后 5 分钟（-heartbeat 可调）服务自动停止；Ctrl+C 立即停止。\n\n",
              stdout);
    } else {
        printf("%s\n", url);
    }
    fflush(stdout);

    if (!opt.no_open) {
        pchar *purl = p_from_ascii(url);
        if (open_browser(purl) != 0 && !opt.quiet)
            fputs("未能自动打开浏览器，请手动访问上面的地址\n", stdout);
        free(purl);
    }

    {
        int rc = serve_loop(ls, page_path, opt.beat_ms);
        sock_close(ls);
        net_global_fini();
        if (!opt.quiet) {
            if (rc == 0)
                fputs("页面心跳断了，服务已停止\n", stdout);
            else
                fputs("服务异常结束\n", stdout);
        }
    }
    return 0;
}
