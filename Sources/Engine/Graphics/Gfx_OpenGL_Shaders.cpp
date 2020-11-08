#include <Engine/Graphics/GfxLibrary.h>
#include <Engine/Graphics/Gfx_OpenGL_Shaders.h>
#include <Engine/Math/Matrix.h>

class GfxProgramPrivate {
public:
  GfxProgramPrivate(const char *vertexShader, const char *fragmentShader)
    : vertexShader(vertexShader),
      fragmentShader(fragmentShader) {};

  GLuint pgmObject = 0;
  const char *vertexShader, *fragmentShader;
  // uniforms
  GLint projMat;
  GLint modelViewMat;
  GLint mainTexture;
  GLint enableTexture;
  GLint enableAlphaTest;
  GLint withReflectionMapping;
  GLint withSpecularMapping;
  GLint isUnlite;
  GLint stretch;
  GLint offset;
  GLint lerpRatio;
  GLint texCorr;
  GLint color;
  GLint lightObj;
  GLint colorAmbient;
  GLint colorLight;
  GLint viewer;
  GLint objectRotation;
  GLint objectToView;
};

static GfxShadersUniforms gfxCurrentUniforms;

static GfxProgramPrivate *_currentProgram = 0;

GfxProgram gfxMakeShaderProgram(const char *vertexShader, const char *fragmentShader) {
  return (GfxProgram) new GfxProgramPrivate(vertexShader, fragmentShader);
}

GLuint compileShader(GLenum type, const char *name, const char *source) {
  GLuint shader = pglCreateShader(type);
  pglShaderSource(shader, 1, &source, nullptr);
  pglCompileShader(shader);

  // check status
  GLint success;
  pglGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char buffer[2001] = "";
    pglGetShaderInfoLog(shader, 2000, nullptr, buffer);
    static char error[2501];
    snprintf(error, 2500, "Cannot compile %s: %s", name, buffer);
    throw error;
  }

  return shader;
}

void gfxUseProgram(GfxProgram _program) {
  GfxProgramPrivate *pgm = (GfxProgramPrivate *) _program;
  _currentProgram = pgm;
  if (!pgm) {
    pglUseProgram(0);
    return;
  }

  if (!pgm->pgmObject) {
    pgm->pgmObject = pglCreateProgram();
    pglAttachShader(pgm->pgmObject, compileShader(GL_VERTEX_SHADER, "vertex shader", pgm->vertexShader));
    pglAttachShader(pgm->pgmObject, compileShader(GL_FRAGMENT_SHADER, "fragment shader", pgm->fragmentShader));
    pglLinkProgram(pgm->pgmObject);
    pglUseProgram(pgm->pgmObject);

    // get uniforms
    pgm->projMat = pglGetUniformLocation(pgm->pgmObject, "projMat");
    pgm->modelViewMat = pglGetUniformLocation(pgm->pgmObject, "modelViewMat");
    pgm->mainTexture = pglGetUniformLocation(pgm->pgmObject, "mainTexture");
    if (pgm->mainTexture >= 0) pglUniform1i(pgm->mainTexture, 0);
    pgm->enableTexture = pglGetUniformLocation(pgm->pgmObject, "enableTexture");
    pgm->enableAlphaTest = pglGetUniformLocation(pgm->pgmObject, "enableAlphaTest");

    pgm->withReflectionMapping = pglGetUniformLocation(pgm->pgmObject, "withReflectionMapping");
    pgm->withSpecularMapping = pglGetUniformLocation(pgm->pgmObject, "withSpecularMapping");

    pgm->stretch = pglGetUniformLocation(pgm->pgmObject, "stretch");
    pgm->offset = pglGetUniformLocation(pgm->pgmObject, "offset");
    pgm->lerpRatio = pglGetUniformLocation(pgm->pgmObject, "lerpRatio");
    pgm->texCorr = pglGetUniformLocation(pgm->pgmObject, "texCorr");
    pgm->color = pglGetUniformLocation(pgm->pgmObject, "color");
    pgm->lightObj = pglGetUniformLocation(pgm->pgmObject, "lightObj");
    pgm->colorAmbient = pglGetUniformLocation(pgm->pgmObject, "colorAmbient");
    pgm->colorLight = pglGetUniformLocation(pgm->pgmObject, "colorLight");
    pgm->viewer = pglGetUniformLocation(pgm->pgmObject, "viewer");
    pgm->isUnlite = pglGetUniformLocation(pgm->pgmObject, "isUnlite");
    pgm->objectRotation = pglGetUniformLocation(pgm->pgmObject, "objectRotation");
    pgm->objectToView = pglGetUniformLocation(pgm->pgmObject, "objectToView");

  } else {
    pglUseProgram(pgm->pgmObject);
  }
}

SLONG gfxGetAttribLocation(GfxProgram _program, const char *name) {
  GfxProgramPrivate *pgm = (GfxProgramPrivate *) _program;
  GLint result = pglGetAttribLocation(pgm->pgmObject, name);
  return result;
}

void gfxUniform(const char *uniformName, float f0) {
  GfxProgramPrivate *pgm = _currentProgram;
  ASSERT(pgm);
  GLint uniformLocation = pglGetUniformLocation(pgm->pgmObject, uniformName);
  if (uniformLocation >= 0) {
    pglUniform1f(uniformLocation, f0);
  }
}

void gfxUniform(const char *uniformName, float f0, float f1) {
  GfxProgramPrivate *pgm = _currentProgram;
  ASSERT(pgm);
  GLint uniformLocation = pglGetUniformLocation(pgm->pgmObject, uniformName);
  if (uniformLocation >= 0) {
    pglUniform2f(uniformLocation, f0, f1);
  }
}

void gfxUniform(const char *uniformName, float f0, float f1, float f2) {
  GfxProgramPrivate *pgm = _currentProgram;
  ASSERT(pgm);
  GLint uniformLocation = pglGetUniformLocation(pgm->pgmObject, uniformName);
  if (uniformLocation >= 0) {
    pglUniform3f(uniformLocation, f0, f1, f2);
  }
}

void gfxUniform(const char *uniformName, float f0, float f1, float f2, float f3) {
  GfxProgramPrivate *pgm = _currentProgram;
  ASSERT(pgm);
  GLint uniformLocation = pglGetUniformLocation(pgm->pgmObject, uniformName);
  if (uniformLocation >= 0) {
    pglUniform4f(uniformLocation, f0, f1, f2, f3);
  }
}

void gfxUniform(const char *uniformName, GFXColor &color) {
  gfxUniform(uniformName, color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
}

void gfxUniform(const char *uniformName, const FLOATmatrix3D &matrix) {
  GfxProgramPrivate *pgm = _currentProgram;
  ASSERT(pgm);
  GLint uniformLocation = pglGetUniformLocation(pgm->pgmObject, uniformName);
  if (uniformLocation >= 0) {
    pglUniformMatrix3fv(uniformLocation, 1, GL_FALSE, (const float *) &matrix);
  }
}

inline void gfxSetUniformValue(GLint uniformLocation, bool value) {
  pglUniform1f(uniformLocation, value ? 1 : 0);
}

inline void gfxSetUniformValue(GLint uniformLocation, float value) {
  pglUniform1f(uniformLocation, value);
}

inline void gfxSetUniformValue(GLint uniformLocation, FLOAT2D value) {
  pglUniform2f(uniformLocation, value(1), value(2));
}

inline void gfxSetUniformValue(GLint uniformLocation, const FLOAT3D &value) {
  pglUniform3f(uniformLocation, value(1), value(2), value(3));
}

inline void gfxSetUniformValue(GLint uniformLocation, GFXColor value) {
  pglUniform4f(uniformLocation, value.r / 255.0f, value.g / 255.0f, value.b / 255.0f, value.a / 255.0f);
}

inline void gfxSetUniformValue(GLint uniformLocation, const FLOATmatrix3D &value) {
  pglUniformMatrix3fv(uniformLocation, 1, GL_FALSE, (const float *) &value);
}

inline BOOL operator!=(const GFXColor &v1, const GFXColor &v2) {
  return v1.abgr != v2.abgr;
};

void gfxSyncProgram(GfxShadersUniforms &params) {
  GfxProgramPrivate *pgm = _currentProgram;
  ASSERT(pgm);
  pglUseProgram(pgm->pgmObject);

  // update buffers
  pglUniformMatrix4fv(pgm->projMat, 1, GL_FALSE, _pGfx->GetProjMat());
  pglUniformMatrix4fv(pgm->modelViewMat, 1, GL_FALSE, _pGfx->GetModelViewMat());
  params.enableTexture = pglIsEnabled(GL_TEXTURE_2D);
  params.enableAlphaTest = pglIsEnabled(GL_ALPHA_TEST);

  gfxSetUniformValue(pgm->enableTexture, params.enableTexture);
  gfxSetUniformValue(pgm->enableAlphaTest, params.enableAlphaTest);
  gfxSetUniformValue(pgm->withReflectionMapping, params.withReflectionMapping);
  gfxSetUniformValue(pgm->withSpecularMapping, params.withSpecularMapping);
  gfxSetUniformValue(pgm->stretch, params.stretch);
  gfxSetUniformValue(pgm->offset, params.offset);
  gfxSetUniformValue(pgm->lerpRatio, params.lerpRatio);
  gfxSetUniformValue(pgm->texCorr, params.texCorr);
  gfxSetUniformValue(pgm->color, params.color);
  gfxSetUniformValue(pgm->lightObj, params.lightObj);
  gfxSetUniformValue(pgm->colorAmbient, params.colorAmbient);
  gfxSetUniformValue(pgm->colorLight, params.colorLight);
  gfxSetUniformValue(pgm->viewer, params.viewer);
  gfxSetUniformValue(pgm->isUnlite, params.isUnlite);
  gfxSetUniformValue(pgm->objectRotation, params.objectRotation);
  gfxSetUniformValue(pgm->objectToView, params.objectToView);
}
