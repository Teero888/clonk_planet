#include <C4Include.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string.h>
#include <thread>
#include <algorithm>

struct CGLSurface {
  int w, h, pitch;
  uint8_t *bits;
  GLuint tex;
  GLuint fbo;
  bool dirty_cpu;
  bool dirty_gpu;
};

// Global instance
CStdDDraw *lpDDraw = NULL;
static GLFWwindow *g_window = NULL;
static GLuint g_paletteTex = 0;
static GLuint g_shaderProgram = 0;
static GLuint g_vao = 0, g_vbo = 0;
extern std::thread::id g_mainThreadId;

extern "C" void *GetGLFWWindow() { return (void *)g_window; }

static void DebugLog(const char *msg) {
  std::ofstream out("/tmp/clonk_debug.log", std::ios::app);
  out << "[Thread " << std::this_thread::get_id() << "] " << msg << std::endl;
}

static void glfwErrorCallback(int error, const char *description) {
  char buf[1024];
  sprintf(buf, "GLFW Error (%d): %s", error, description);
  DebugLog(buf);
  std::cerr << buf << std::endl;
}

static const char *vertexShaderSrc = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
uniform mat4 projection;
void main() {
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

static const char *fragmentShaderSrc = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
uniform usampler2D texImage;
uniform sampler1D texPalette;
void main() {
    ivec2 itexCoord = ivec2(TexCoord * vec2(textureSize(texImage, 0)));
    uint idx = texelFetch(texImage, itexCoord, 0).r;
    if (idx == 0u) discard;
    FragColor = texelFetch(texPalette, int(idx), 0);
}
)";

static GLuint CompileShader(GLenum type, const char *source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);
  int success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    DebugLog("Shader Compile Error");
    DebugLog(infoLog);
  }
  return shader;
}

static void GetOrthoMatrix(float *mat, float left, float right, float bottom, float top) {
  memset(mat, 0, 16 * sizeof(float));
  mat[0] = 2.0f / (right - left);
  mat[5] = 2.0f / (top - bottom);
  mat[10] = -1.0f;
  mat[12] = -(right + left) / (right - left);
  mat[13] = -(top + bottom) / (top - bottom);
  mat[15] = 1.0f;
}

static void SyncSurfaceGPU(CGLSurface *surf) {
  if (surf->dirty_cpu && surf->tex && std::this_thread::get_id() == g_mainThreadId) {
    glTextureSubImage2D(surf->tex, 0, 0, 0, surf->w, surf->h, GL_RED_INTEGER, GL_UNSIGNED_BYTE, surf->bits);
    surf->dirty_cpu = false;
  }
}

static inline void SyncSurfaceCPU(CGLSurface *surf) {}

CStdDDraw::CStdDDraw() {
  lpPrimary = NULL;
  lpBack = NULL;
  lpDirectDraw = NULL;
  lpPalette = NULL;
  lpClipper = NULL;
  lpWindowClipper = NULL;
  fBackAttached = FALSE;
  fPageLock = FALSE;
  Pattern = NULL;
  PatWdt = PatHgt = PatWdtMod = PatHgtMod = 0;
  PatQuickMod = FALSE;
  Default();
  lpDDraw = this;
}

CStdDDraw::~CStdDDraw() {
  Clear();
  if (lpDDraw == this)
    lpDDraw = NULL;
}

void CStdDDraw::Default() {
  ClipX1 = ClipY1 = 0;
  ClipX2 = 800;
  ClipY2 = 600;
}

void CStdDDraw::Clear() {
  if (std::this_thread::get_id() != g_mainThreadId)
    return;
  DebugLog("DDraw Clear");
  if (g_window) {
    glfwDestroyWindow(g_window);
    g_window = NULL;
  }
  glfwTerminate();
}

BOOL CStdDDraw::Init(HWND hWnd, BOOL Fullscreen, int iResX, int iResY, BOOL fUsePageLock) {
  // Note: ignore hWnd, we use GLFW
  g_mainThreadId = std::this_thread::get_id();
  DebugLog("DDraw Init Start");

  if (iResX <= 0 || iResX > 4096)
    iResX = 640;
  if (iResY <= 0 || iResY > 4096)
    iResY = 480;

  glfwSetErrorCallback(glfwErrorCallback);

  if (!glfwInit())
    return FALSE;
  printf("glfwInit successful\n");
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  if (getenv("CLONK_TEST_HEADLESS"))
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  else
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
  glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

  printf("glfwCreateWindow(%dx%d)...\n", iResX, iResY);
  // Force windowed mode to avoid driver crashes reported by user
  g_window = glfwCreateWindow(iResX, iResY, "Clonk Planet", NULL, NULL);
  if (!g_window)
    return FALSE;
  printf("glfwCreateWindow successful\n");

  glfwMakeContextCurrent(g_window);
  glfwSwapInterval(1);

  printf("glewInit...\n");
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK)
    return FALSE;
  printf("glewInit successful\n");
  glGetError();

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glCreateTextures(GL_TEXTURE_1D, 1, &g_paletteTex);
  glTextureStorage1D(g_paletteTex, 1, GL_RGB8, 256);
  glTextureParameteri(g_paletteTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTextureParameteri(g_paletteTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTextureParameteri(g_paletteTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  uint8_t dummyPal[768] = {0};
  glTextureSubImage1D(g_paletteTex, 0, 0, 256, GL_RGB, GL_UNSIGNED_BYTE, dummyPal);

  GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexShaderSrc);
  GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
  g_shaderProgram = glCreateProgram();
  glAttachShader(g_shaderProgram, vs);
  glAttachShader(g_shaderProgram, fs);
  glLinkProgram(g_shaderProgram);

  int linkSuccess;
  glGetProgramiv(g_shaderProgram, GL_LINK_STATUS, &linkSuccess);
  if (!linkSuccess) {
    char infoLog[512];
    glGetProgramInfoLog(g_shaderProgram, 512, NULL, infoLog);
    DebugLog("Shader link error");
    DebugLog(infoLog);
    return FALSE;
  }
  glDeleteShader(vs);
  glDeleteShader(fs);

  glCreateVertexArrays(1, &g_vao);
  glCreateBuffers(1, &g_vbo);
  glVertexArrayVertexBuffer(g_vao, 0, g_vbo, 0, 4 * sizeof(float));
  glEnableVertexArrayAttrib(g_vao, 0);
  glEnableVertexArrayAttrib(g_vao, 1);
  glVertexArrayAttribFormat(g_vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribFormat(g_vao, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float));
  glVertexArrayAttribBinding(g_vao, 0, 0);
  glVertexArrayAttribBinding(g_vao, 1, 0);

  lpPrimary = CreateSurface(iResX, iResY);
  lpBack = CreateSurface(iResX, iResY);

  DebugLog("DDraw Init Successful");
  return TRUE;
}

static void DrawHardwareQuad(CGLSurface *src, CGLSurface *dst, int tx, int ty, int twdt, int thgt, int sx = 0, int sy = 0, int swdt = -1, int shgt = -1, bool useScissor = true) {
  if (!src || !src->tex) return;
  if (swdt == -1) swdt = src->w;
  if (shgt == -1) shgt = src->h;
  
  SyncSurfaceGPU(src);
  
  if (dst) {
    SyncSurfaceGPU(dst); // Ensure target is up to date if cpu modified it before this GPU draw
    glBindFramebuffer(GL_FRAMEBUFFER, dst->fbo);
    glViewport(0, 0, dst->w, dst->h);
    dst->dirty_gpu = true;
  } else {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    int w, h;
    glfwGetFramebufferSize(g_window, &w, &h);
    glViewport(0, 0, w, h);
  }

  if (useScissor && lpDDraw && dst) {
    glEnable(GL_SCISSOR_TEST);
    int scX = std::max(0, lpDDraw->ClipX1);
    int scY = std::max(0, lpDDraw->ClipY1);
    int scW = std::max(0, lpDDraw->ClipX2 - lpDDraw->ClipX1 + 1);
    int scH = std::max(0, lpDDraw->ClipY2 - lpDDraw->ClipY1 + 1);
    glScissor(scX, dst->h - scY - scH, scW, scH);
  } else {
    glDisable(GL_SCISSOR_TEST);
  }

  glUseProgram(g_shaderProgram);
  
  float projection[16];
  if (dst) GetOrthoMatrix(projection, 0.0f, dst->w, dst->h, 0.0f);
  else {
    int w, h;
    glfwGetFramebufferSize(g_window, &w, &h);
    GetOrthoMatrix(projection, 0.0f, w, h, 0.0f);
  }
  glUniformMatrix4fv(glGetUniformLocation(g_shaderProgram, "projection"), 1, GL_FALSE, projection);

  glBindTextureUnit(0, src->tex);
  glUniform1i(glGetUniformLocation(g_shaderProgram, "texImage"), 0);
  glBindTextureUnit(1, g_paletteTex);
  glUniform1i(glGetUniformLocation(g_shaderProgram, "texPalette"), 1);

  float u1 = (float)sx / src->w;
  float v1 = (float)sy / src->h;
  float u2 = (float)(sx + swdt) / src->w;
  float v2 = (float)(sy + shgt) / src->h;

  float vertices[] = {
    (float)tx,        (float)ty,        u1, v1,
    (float)(tx+twdt), (float)ty,        u2, v1,
    (float)(tx+twdt), (float)(ty+thgt), u2, v2,
    (float)(tx+twdt), (float)(ty+thgt), u2, v2,
    (float)tx,        (float)(ty+thgt), u1, v2,
    (float)tx,        (float)ty,        u1, v1
  };

  glBindVertexArray(g_vao);
  glNamedBufferData(g_vbo, sizeof(vertices), vertices, GL_STREAM_DRAW);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glDisable(GL_SCISSOR_TEST);
}

BOOL CStdDDraw::PageFlip() {
  if (std::this_thread::get_id() != g_mainThreadId)
    return FALSE;
  if (!g_window || !lpBack)
    return FALSE;

  CGLSurface *surf = (CGLSurface *)lpBack;
  if (surf->w <= 0 || surf->h <= 0)
    return FALSE;

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  DrawHardwareQuad(surf, nullptr, 0, 0, surf->w, surf->h, 0, 0, -1, -1, false);

  glfwSwapBuffers(g_window);
  glfwPollEvents();

  if (glfwWindowShouldClose(g_window)) {
    DebugLog("Window close requested");
    exit(0);
  }

  return TRUE;
}

BOOL CStdDDraw::SetPrimaryPalette(BYTE *pBuf) {
  if (std::this_thread::get_id() != g_mainThreadId)
    return FALSE;
  if (!g_paletteTex || !pBuf)
    return FALSE;
  glTextureSubImage1D(g_paletteTex, 0, 0, 256, GL_RGB, GL_UNSIGNED_BYTE, pBuf);
  return TRUE;
}

BOOL CStdDDraw::SetPrimaryPaletteQuad(BYTE *pBuf) {
  if (std::this_thread::get_id() != g_mainThreadId)
    return FALSE;
  if (!g_paletteTex || !pBuf)
    return FALSE;
  BYTE rgb[768];
  for (int i = 0; i < 256; i++) {
    rgb[i * 3 + 0] = pBuf[i * 4 + 2];
    rgb[i * 3 + 1] = pBuf[i * 4 + 1];
    rgb[i * 3 + 2] = pBuf[i * 4 + 0];
  }
  glTextureSubImage1D(g_paletteTex, 0, 0, 256, GL_RGB, GL_UNSIGNED_BYTE, rgb);
  return TRUE;
}

BOOL CStdDDraw::AttachPrimaryPalette(SURFACE sfcSurface) { return TRUE; }
BOOL CStdDDraw::GetPrimaryClipper(int &rX1, int &rY1, int &rX2, int &rY2) {
  rX1 = ClipX1;
  rY1 = ClipY1;
  rX2 = ClipX2;
  rY2 = ClipY2;
  return TRUE;
}
BOOL CStdDDraw::SetPrimaryClipper(int iX1, int iY1, int iX2, int iY2) {
  ClipX1 = iX1;
  ClipY1 = iY1;
  ClipX2 = iX2;
  ClipY2 = iY2;
  return TRUE;
}
BOOL CStdDDraw::SubPrimaryClipper(int iX1, int iY1, int iX2, int iY2) {
  ClipX1 = std::max(ClipX1, iX1);
  ClipY1 = std::max(ClipY1, iY1);
  ClipX2 = std::min(ClipX2, iX2);
  ClipY2 = std::min(ClipY2, iY2);
  return TRUE;
}
BOOL CStdDDraw::RestorePrimaryClipper() { return TRUE; }
BOOL CStdDDraw::NoPrimaryClipper() {
  ClipX1 = ClipY1 = 0;
  ClipX2 = 4096;
  ClipY2 = 4096;
  return TRUE;
}
BOOL CStdDDraw::ApplyPrimaryClipper(SURFACE sfcSurface) { return TRUE; }
BOOL CStdDDraw::DetachPrimaryClipper(SURFACE sfcSurface) { return TRUE; }

BYTE *CStdDDraw::LockSurface(SURFACE sfcSurface, int &lPitch, int *lpSfcWdt, int *lpSfcHgt) {
  if (!sfcSurface)
    return NULL;
  CGLSurface *s = (CGLSurface *)sfcSurface;
  lPitch = s->pitch;
  if (lpSfcWdt)
    *lpSfcWdt = s->w;
  if (lpSfcHgt)
    *lpSfcHgt = s->h;
  
  if (s->dirty_gpu && s->fbo && std::this_thread::get_id() == g_mainThreadId) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, s->fbo);
    glReadPixels(0, 0, s->w, s->h, GL_RED_INTEGER, GL_UNSIGNED_BYTE, s->bits);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    s->dirty_gpu = false;
  }
  
  s->dirty_cpu = true;
  return s->bits;
}

void CStdDDraw::UnLockSurface(SURFACE sfcSurface) {}

BOOL CStdDDraw::GetSurfaceSize(SURFACE sfcSurface, int &iWdt, int &iHgt) {
  if (!sfcSurface)
    return FALSE;
  CGLSurface *s = (CGLSurface *)sfcSurface;
  iWdt = s->w;
  iHgt = s->h;
  return TRUE;
}

HDC CStdDDraw::GetSurfaceDC(SURFACE sfcSurface) { return NULL; }

BOOL CStdDDraw::WipeSurface(SURFACE sfcSurface) {
  if (!sfcSurface)
    return FALSE;
  CGLSurface *s = (CGLSurface *)sfcSurface;
  memset(s->bits, 0, s->pitch * s->h);
  s->dirty_cpu = true;
  return TRUE;
}

SURFACE CStdDDraw::DuplicateSurface(SURFACE sfcSurface) {
  if (!sfcSurface)
    return NULL;
  CGLSurface *s = (CGLSurface *)sfcSurface;
  SURFACE ns = CreateSurface(s->w, s->h);
  if (!ns)
    return NULL;
  CGLSurface *ns2 = (CGLSurface *)ns;
  
  if (s->dirty_gpu && s->fbo && std::this_thread::get_id() == g_mainThreadId) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, s->fbo);
    glReadPixels(0, 0, s->w, s->h, GL_RED_INTEGER, GL_UNSIGNED_BYTE, s->bits);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    s->dirty_gpu = false;
  }
  
  memcpy(ns2->bits, s->bits, s->pitch * s->h);
  ns2->dirty_cpu = true;
  return ns;
}

void CStdDDraw::DestroySurface(SURFACE sfcSurface) {
  if (!sfcSurface)
    return;
  CGLSurface *s = (CGLSurface *)sfcSurface;
  if (std::this_thread::get_id() == g_mainThreadId) {
    if (s->tex) glDeleteTextures(1, &s->tex);
    if (s->fbo) glDeleteFramebuffers(1, &s->fbo);
  }
  if (s->bits)
    delete[] s->bits;
  delete s;
}

SURFACE CStdDDraw::CreateSurface(int iWdt, int iHgt) {
  if (iWdt <= 0 || iWdt > 4096)
    iWdt = 1;
  if (iHgt <= 0 || iHgt > 4096)
    iHgt = 1;
  CGLSurface *s = new CGLSurface();
  s->w = iWdt;
  s->h = iHgt;
  s->pitch = iWdt;
  s->bits = new uint8_t[s->pitch * s->h];
  memset(s->bits, 0, s->pitch * s->h);

  if (std::this_thread::get_id() == g_mainThreadId && g_window) {
    glCreateTextures(GL_TEXTURE_2D, 1, &s->tex);
    glTextureStorage2D(s->tex, 1, GL_R8UI, s->w, s->h);
    glTextureParameteri(s->tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(s->tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(s->tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(s->tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureSubImage2D(s->tex, 0, 0, 0, s->w, s->h, GL_RED_INTEGER, GL_UNSIGNED_BYTE, s->bits);
    
    glCreateFramebuffers(1, &s->fbo);
    glNamedFramebufferTexture(s->fbo, GL_COLOR_ATTACHMENT0, s->tex, 0);
  } else {
    s->tex = 0;
    s->fbo = 0;
  }
  s->dirty_cpu = false;
  s->dirty_gpu = false;
  return (SURFACE)s;
}

void CStdDDraw::SurfaceShiftColor(SURFACE sfcSfc, int iShift) {
  if (!sfcSfc)
    return;
  CGLSurface *s = (CGLSurface *)sfcSfc;
  SyncSurfaceCPU(s);
  for (int i = 0; i < s->pitch * s->h; i++) {
    if (s->bits[i])
      s->bits[i] = (BYTE)BoundBy((int)s->bits[i] + iShift, 0, 255);
  }
  s->dirty_cpu = true;
}

void CStdDDraw::SurfaceShiftColorRange(SURFACE sfcSfc, int iRngLo, int iRngHi, int iShift) {
  if (!sfcSfc)
    return;
  CGLSurface *s = (CGLSurface *)sfcSfc;
  for (int i = 0; i < s->pitch * s->h; i++) {
    if (Inside((int)s->bits[i], iRngLo, iRngHi)) {
      s->bits[i] = (BYTE)BoundBy((int)s->bits[i] + iShift, 0, 255);
    }
  }
  s->dirty_cpu = true;
}

void CStdDDraw::SurfaceAllowColor(SURFACE sfcSfc, BYTE iRngLo, BYTE iRngHi, BOOL fAllowZero) {
  BYTE *sfcbuf, *lbuf;
  int xcnt, ycnt, pitch, wdt, hgt;
  if (iRngHi < iRngLo) return;
  if (!(sfcbuf = LockSurface(sfcSfc, pitch, &wdt, &hgt))) return;
  for (ycnt = 0; ycnt < hgt; ycnt++) {
    lbuf = sfcbuf + pitch * ycnt;
    for (xcnt = 0; xcnt < wdt; xcnt++, lbuf++) {
      if ((*lbuf != 0) || !fAllowZero) {
        if ((*lbuf < iRngLo) || (*lbuf > iRngHi)) {
          *lbuf = iRngLo + *lbuf % (iRngHi - iRngLo + 1);
        }
      }
    }
  }
  UnLockSurface(sfcSfc);
}

BOOL CStdDDraw::BlitFast(SURFACE sfcSource, int fx, int fy, SURFACE sfcTarget, int tx, int ty, int wdt, int hgt) { return Blit(sfcSource, fx, fy, wdt, hgt, sfcTarget, tx, ty, wdt, hgt, TRUE); }

BOOL CStdDDraw::Blit(SURFACE sfcSource, int fx, int fy, int fwdt, int fhgt, SURFACE sfcTarget, int tx, int ty, int twdt, int thgt, BOOL fSrcColKey) {
  if (!sfcSource || !sfcTarget)
    return FALSE;
  if (twdt <= 0 || thgt <= 0 || twdt > 4096 || thgt > 4096)
    return FALSE;
  CGLSurface *src = (CGLSurface *)sfcSource;
  CGLSurface *dst = (CGLSurface *)sfcTarget;

  SyncSurfaceCPU(src);
  SyncSurfaceCPU(dst);

  int clipX1 = std::max(0, ClipX1);
  int clipX2 = std::min(dst->w - 1, ClipX2);
  int clipY1 = std::max(0, ClipY1);
  int clipY2 = std::min(dst->h - 1, ClipY2);

  int startY = std::max(0, clipY1 - ty);
  int endY = std::min(thgt - 1, clipY2 - ty);
  int startX = std::max(0, clipX1 - tx);
  int endX = std::min(twdt - 1, clipX2 - tx);

  for (int y = startY; y <= endY; y++) {
    int dy = ty + y;
    int sy = fy + (y * fhgt / thgt);
    if (sy < 0 || sy >= src->h)
      continue;

    uint8_t *pSrcLine = src->bits + sy * src->pitch;
    uint8_t *pDstLine = dst->bits + dy * dst->pitch;

    for (int x = startX; x <= endX; x++) {
      int sx = fx + (x * fwdt / twdt);
      if (sx < 0 || sx >= src->w)
        continue;

      uint8_t c = pSrcLine[sx];
      if (!fSrcColKey || c != 0) {
        pDstLine[tx + x] = c;
      }
    }
  }
  dst->dirty_cpu = true;
  return TRUE;
}

BOOL CStdDDraw::BlitRotate(SURFACE sfcSource, int fx, int fy, int fwdt, int fhgt, SURFACE sfcTarget, int tx, int ty, int twdt, int thgt, int iAngle) {
  if (!sfcSource || !sfcTarget)
    return FALSE;
  CGLSurface *src = (CGLSurface *)sfcSource;
  CGLSurface *dst = (CGLSurface *)sfcTarget;

  SyncSurfaceCPU(src);
  SyncSurfaceCPU(dst);

  double angle = (double)iAngle * 3.14159265 / 180.0;
  double ca = cos(-angle);
  double sa = sin(-angle);

  int midX = tx + twdt / 2;
  int midY = ty + thgt / 2;

  for (int y = 0; y < thgt; y++) {
    int dy = ty + y;
    if (dy < std::max(0, ClipY1) || dy > std::min(dst->h - 1, ClipY2))
      continue;
    for (int x = 0; x < twdt; x++) {
      int dx = tx + x;
      if (dx < std::max(0, ClipX1) || dx > std::min(dst->w - 1, ClipX2))
        continue;

      double rx = (double)(dx - midX);
      double ry = (double)(dy - midY);

      int sx = fx + fwdt / 2 + (int)(rx * ca - ry * sa);
      int sy = fy + fhgt / 2 + (int)(rx * sa + ry * ca);

      if (sx >= fx && sx < fx + fwdt && sy >= fy && sy < fy + fhgt) {
        uint8_t c = src->bits[sy * src->pitch + sx];
        if (c != 0) {
          dst->bits[dy * dst->pitch + dx] = c;
        }
      }
    }
  }
  dst->dirty_cpu = true;
  return TRUE;
}

BOOL CStdDDraw::BlitRotateOriginal(SURFACE sfcSource, int fx, int fy, int fwdt, int fhgt, SURFACE sfcTarget, int tx, int ty, int twdt, int thgt, int iAngle) {
  return BlitRotate(sfcSource, fx, fy, fwdt, fhgt, sfcTarget, tx, ty, twdt, thgt, iAngle);
}

BOOL CStdDDraw::BlitSurface(SURFACE sfcSurface, SURFACE sfcTarget, int tx, int ty) {
  int w, h;
  if (!GetSurfaceSize(sfcSurface, w, h))
    return FALSE;
  return Blit(sfcSurface, 0, 0, w, h, sfcTarget, tx, ty, w, h, TRUE);
}

BOOL CStdDDraw::BlitSurfaceTile(SURFACE sfcSurface, SURFACE sfcTarget, int iToX, int iToY, int iToWdt, int iToHgt, int iOffsetX, int iOffsetY, BOOL fSrcColKey) {
  int sw, sh;
  if (!GetSurfaceSize(sfcSurface, sw, sh))
    return FALSE;

  for (int y = 0; y < iToHgt; y++) {
    int ty = iToY + y;
    int sy = (y + iOffsetY) % sh;
    if (sy < 0)
      sy += sh;
    for (int x = 0; x < iToWdt; x++) {
      int tx = iToX + x;
      int sx = (x + iOffsetX) % sw;
      if (sx < 0)
        sx += sw;

      uint8_t c = GetPixel(sfcSurface, sx, sy);
      if (!fSrcColKey || c != 0) {
        SetPixel(sfcTarget, tx, ty, c);
      }
    }
  }
  return TRUE;
}

BOOL CStdDDraw::BlitSurface2Window(SURFACE sfcSource, int fX, int fY, int fWdt, int fHgt, HWND hWnd, int tX, int tY, int tWdt, int tHgt) { return TRUE; }

BOOL CStdDDraw::InitFont(const char *szFontName, int iSize) { return Font.Init(NULL, szFontName, iSize); }
BOOL CStdDDraw::TextExtent(const char *szString, int &rWdt, int &rHgt) { return Font.TextExtent(szString, rWdt, rHgt); }
int CStdDDraw::TextHeight(const char *szText) { return Font.GetTextHeight(szText); }
int CStdDDraw::TextWidth(const char *szText) { return Font.GetTextWidth(szText); }
BOOL CStdDDraw::TextOut(const char *szText, SURFACE sfcDest, int iTx, int iTy, int iFCol, int iBCol, BYTE byForm) { return Font.TextOut(szText, sfcDest, iTx, iTy, iFCol, iBCol, byForm); }
BOOL CStdDDraw::StringOut(const char *szText, SURFACE sfcDest, int iTx, int iTy, int iFCol, int iBCol, BYTE byForm) { return Font.StringOut(szText, sfcDest, iTx, iTy, iFCol, iBCol, byForm); }

BOOL CStdDDraw::SetPixel(SURFACE sfcDest, int tx, int ty, BYTE col) {
  if (!sfcDest)
    return FALSE;
  CGLSurface *dst = (CGLSurface *)sfcDest;
  if (tx >= 0 && tx < dst->w && ty >= 0 && ty < dst->h) {
    dst->bits[ty * dst->pitch + tx] = col;
    dst->dirty_cpu = true;
  }
  return TRUE;
}

BYTE CStdDDraw::GetPixel(SURFACE sfcSource, int fx, int fy) {
  if (!sfcSource)
    return 0;
  CGLSurface *src = (CGLSurface *)sfcSource;
  SyncSurfaceCPU(src);
  if (fx >= 0 && fx < src->w && fy >= 0 && fy < src->h) {
    return src->bits[fy * src->pitch + fx];
  }
  return 0;
}

void CStdDDraw::DrawBox(SURFACE sfcDest, int x1, int y1, int x2, int y2, BYTE col) {
  if (!sfcDest)
    return;
  CGLSurface *dst = (CGLSurface *)sfcDest;
  SyncSurfaceCPU(dst);
  
  if (x1 < std::max(0, ClipX1)) x1 = std::max(0, ClipX1);
  if (x2 > std::min(dst->w - 1, ClipX2)) x2 = std::min(dst->w - 1, ClipX2);
  if (y1 < std::max(0, ClipY1)) y1 = std::max(0, ClipY1);
  if (y2 > std::min(dst->h - 1, ClipY2)) y2 = std::min(dst->h - 1, ClipY2);
  
  if (x1 > x2 || y1 > y2) return;

  for (int y = y1; y <= y2; y++) {
    uint8_t *pLine = dst->bits + y * dst->pitch;
    if (!Pattern) {
      for (int x = x1; x <= x2; x++) {
        pLine[x] = col;
      }
    } else {
      uint8_t *bypPatLine;
      if (PatQuickMod) {
        bypPatLine = Pattern + PatWdt * (y & PatHgtMod);
        for (int x = x1; x <= x2; x++) {
          pLine[x] = col + bypPatLine[x & PatWdtMod];
        }
      } else {
        bypPatLine = Pattern + PatWdt * (y % PatHgt);
        for (int x = x1; x <= x2; x++) {
          pLine[x] = col + bypPatLine[x % PatWdt];
        }
      }
    }
  }
  dst->dirty_cpu = true;
}

void CStdDDraw::DrawBoxColorTable(SURFACE sfcDest, int x1, int y1, int x2, int y2, BYTE *bypColorTable) {
  if (!sfcDest || !bypColorTable)
    return;
  CGLSurface *dst = (CGLSurface *)sfcDest;
  if (x1 < std::max(0, ClipX1)) x1 = std::max(0, ClipX1);
  if (x2 > std::min(dst->w - 1, ClipX2)) x2 = std::min(dst->w - 1, ClipX2);
  if (y1 < std::max(0, ClipY1)) y1 = std::max(0, ClipY1);
  if (y2 > std::min(dst->h - 1, ClipY2)) y2 = std::min(dst->h - 1, ClipY2);
  if (x1 > x2 || y1 > y2) return;

  for (int y = y1; y <= y2; y++) {
    uint8_t *pLine = dst->bits + y * dst->pitch;
    for (int x = x1; x <= x2; x++) {
      pLine[x] = bypColorTable[pLine[x]];
    }
  }
  dst->dirty_cpu = true;
}

void CStdDDraw::DrawCircle(SURFACE sfcDest, int x, int y, int r, BYTE col) {
  if (!sfcDest)
    return;
  for (int i = 0; i < 360; i++) {
    double angle = i * 3.14159265 / 180.0;
    int px = x + (int)(r * cos(angle));
    int py = y + (int)(r * sin(angle));
    SetPixel(sfcDest, px, py, col);
  }
}

void CStdDDraw::DrawHorizontalLine(SURFACE sfcDest, int x1, int x2, int y, BYTE col) { 
  if (x1 > x2) std::swap(x1, x2);
  DrawBox(sfcDest, x1, y, x2, y, col); 
}

void CStdDDraw::DrawVerticalLine(SURFACE sfcDest, int x, int y1, int y2, BYTE col) { 
  if (y1 > y2) std::swap(y1, y2);
  DrawBox(sfcDest, x, y1, x, y2, col); 
}

void CStdDDraw::DrawFrame(SURFACE sfcDest, int x1, int y1, int x2, int y2, BYTE col) {
  DrawHorizontalLine(sfcDest, x1, x2, y1, col);
  DrawHorizontalLine(sfcDest, x1, x2, y2, col);
  DrawVerticalLine(sfcDest, x1, y1, y2, col);
  DrawVerticalLine(sfcDest, x2, y1, y2, col);
}

void CStdDDraw::DrawInline(SURFACE sfcDest, int iX1, int iY1, int iX2, int iY2, BYTE byCol, BYTE byOnCol, BYTE byAdjacentCol) {
  if (!sfcDest) return;
  CGLSurface *dst = (CGLSurface *)sfcDest;
  SyncSurfaceCPU(dst);
  
  if ((iX2 < std::max(0, ClipX1)) || (iX1 > std::min(dst->w - 1, ClipX2)) 
   || (iY2 < std::max(0, ClipY1)) || (iY1 > std::min(dst->h - 1, ClipY2))) {
    return;
  }
  
  if (iX1 < std::max(0, ClipX1)) iX1 = std::max(0, ClipX1); 
  if (iX2 > std::min(dst->w - 1, ClipX2)) iX2 = std::min(dst->w - 1, ClipX2); 
  if (iY1 < std::max(0, ClipY1)) iY1 = std::max(0, ClipY1); 
  if (iY2 > std::min(dst->h - 1, ClipY2)) iY2 = std::min(dst->h - 1, ClipY2);
  
  for (int cx = iX1; cx <= iX2; cx++) {
    for (int cy = iY1; cy <= iY2; cy++) {
      if (dst->bits[dst->pitch * cy + cx] == byOnCol) {
        if ((cx == iX1) || (dst->bits[dst->pitch * cy + cx - 1] == byAdjacentCol)
         || (cx == iX2) || (dst->bits[dst->pitch * cy + cx + 1] == byAdjacentCol)
         || (cy == iY1) || (dst->bits[dst->pitch * (cy - 1) + cx] == byAdjacentCol)
         || (cy == iY2) || (dst->bits[dst->pitch * (cy + 1) + cx] == byAdjacentCol)) {
          dst->bits[dst->pitch * cy + cx] = byCol;
        }
      }
    }
  }
  dst->dirty_cpu = true;
}

BOOL CStdDDraw::DrawLine(SURFACE sfcTarget, int x1, int y1, int x2, int y2, BYTE byCol) {
  int dx = abs(x2 - x1);
  int dy = abs(y2 - y1);
  int sx = (x1 < x2) ? 1 : -1;
  int sy = (y1 < y2) ? 1 : -1;
  int err = dx - dy;

  while (true) {
    SetPixel(sfcTarget, x1, y1, byCol);
    if (x1 == x2 && y1 == y2)
      break;
    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x1 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y1 += sy;
    }
  }
  return TRUE;
}

BOOL CStdDDraw::DrawPolygon(SURFACE sfcTarget, int iNum, int *ipVtx, int iCol) {
  if (iNum < 3)
    return FALSE;
  CGLSurface *dst = (CGLSurface *)sfcTarget;
  int top = dst->h, bottom = 0;
  for (int i = 0; i < iNum; i++) {
    if (ipVtx[i * 2 + 1] < top)
      top = ipVtx[i * 2 + 1];
    if (ipVtx[i * 2 + 1] > bottom)
      bottom = ipVtx[i * 2 + 1];
  }
  top = std::max(0, top);
  bottom = std::min(dst->h - 1, bottom);

  for (int y = top; y <= bottom; y++) {
    std::vector<int> nodes;
    int j = iNum - 1;
    for (int i = 0; i < iNum; i++) {
      if ((ipVtx[i * 2 + 1] < y && ipVtx[j * 2 + 1] >= y) || (ipVtx[j * 2 + 1] < y && ipVtx[i * 2 + 1] >= y)) {
        nodes.push_back(ipVtx[i * 2] + (y - ipVtx[i * 2 + 1]) * (ipVtx[j * 2] - ipVtx[i * 2]) / (ipVtx[j * 2 + 1] - ipVtx[i * 2 + 1]));
      }
      j = i;
    }
    std::sort(nodes.begin(), nodes.end());
    for (size_t i = 0; i < nodes.size(); i += 2) {
      if (i + 1 >= nodes.size())
        break;
      DrawHorizontalLine(sfcTarget, nodes[i], nodes[i + 1], y, iCol);
    }
  }
  return TRUE;
}

BOOL CStdDDraw::DefinePattern(SURFACE sfcSource) {
  int lPitch, lWdt, lHgt;
  BYTE *sfcbuf;
  if (!sfcSource) return FALSE;
  NoPattern();
  if (!(sfcbuf = LockSurface(sfcSource, lPitch, &lWdt, &lHgt))) return FALSE;
  PatWdt = lWdt; PatHgt = lHgt;
  int twdt, thgt;
  for (twdt = 1; twdt < lWdt; twdt <<= 1);
  for (thgt = 1; thgt < lHgt; thgt <<= 1);
  if ((twdt == lWdt) && (thgt == lHgt)) {
    PatQuickMod = TRUE; PatWdtMod = PatWdt - 1; PatHgtMod = PatHgt - 1;
  }
  if (!(Pattern = new BYTE[PatWdt * PatHgt])) {
    UnLockSurface(sfcSource);
    return FALSE;
  }
  for (int cy = 0; cy < PatHgt; cy++)
    memcpy(Pattern + cy * PatWdt, sfcbuf + cy * lPitch, PatWdt);
  UnLockSurface(sfcSource);
  return TRUE;
}

void CStdDDraw::NoPattern() {
  if (Pattern) delete[] Pattern;
  Pattern = NULL;
  PatQuickMod = FALSE;
}

BOOL CStdDDraw::CreatePrimaryPalette(SURFACE sfcAttachTo) { return TRUE; }
BOOL CStdDDraw::CreatePrimaryClipper() { return TRUE; }
BOOL CStdDDraw::CreatePrimarySurfaces(BOOL fFlipAttach) { return TRUE; }
BOOL CStdDDraw::Error(const char *szMsg) { return FALSE; }
BOOL CStdDDraw::SetDisplayMode(int iResX, int iResY, int iColorDepth) { return TRUE; }
BOOL CStdDDraw::SetCooperativeLevel(HWND hWnd, DWORD dwLevel) { return TRUE; }
BOOL CStdDDraw::CreateDirectDraw() { return TRUE; }
int CStdDDraw::SfcCall(HRESULT ddrval) { return 0; }
BOOL CStdDDraw::SurfaceSetColorKey(SURFACE sfcSurface, BYTE byCol) { return TRUE; }
