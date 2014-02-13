#include "Common.h"

using namespace std;
using namespace gl;
using namespace OVR;

const Resource STEREO_IMAGES[] = {
  IMAGES_STEREO_VLCSNAP_2_13_11_16_12H41M46S16__PNG,
  IMAGES_STEREO_VLCSNAP_2_13_11_16_19H47M52S211_PNG,
  IMAGES_STEREO_VLCSNAP_2_13_11_16_2_H_8M17S111_PNG,
  IMAGES_STEREO_VLCSNAP_2_13_11_16_2_H_9M17S243_PNG,
  IMAGES_STEREO_VLCSNAP_2_13_11_16_2_H_9M43S38_PNG,
};

const int STEREO_IMAGE_COUNT = 5;

class StereoscopicImages : public RiftGlfwApp {
protected:
  Texture2dPtr texture;
  GeometryPtr quadGeometries[2];
  glm::mat4 projections[2];
  ProgramPtr program;
  int imageIndex;
  bool stereo;
  glm::uvec2 imageSize;

public:

  StereoscopicImages() {
    imageIndex = 0;
    stereo = true;

    OVR::HMDInfo hmdInfo;
    Rift::getHmdInfo(ovrManager, hmdInfo);
    float lensOffset = 1.0f
      - (2.0f * hmdInfo.LensSeparationDistance / hmdInfo.HScreenSize);

    float lensDistance =
      hmdInfo.LensSeparationDistance /
      hmdInfo.HScreenSize;
    lensOffset =
      1.0f - (2.0f * lensDistance);

    for (int i = 0; i < 2; ++i) {
      float eyeLensOffset = i == 0 ? -lensOffset : lensOffset;
      projections[i] = glm::ortho(
        -1.0f + eyeLensOffset, 1.0f + eyeLensOffset,
        -1.0f / eyeAspect, 1.0f / eyeAspect);
    }
  }

  void initGl() {
    GlfwApp::initGl();
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    program = GlUtils::getProgram(
      Resource::SHADERS_TEXTURED_VS,
      Resource::SHADERS_TEXTURED_FS);

    GlUtils::getImageAsTexture(texture, STEREO_IMAGES[imageIndex], imageSize);

    float imageAspectRatio = (float)imageSize.x / (float)imageSize.y;
    glm::vec2 geometryMax(1.0f, 1.0f / imageAspectRatio);
    glm::vec2 geometryMin = geometryMax * -1.0f;
    glm::vec2 textureMin = glm::vec2(0, 0);
    glm::vec2 textureMax = glm::vec2(0.5, 1.0);
    quadGeometries[0] =
      GlUtils::getQuadGeometry(
      geometryMin, geometryMax,
      textureMin, textureMax);

    textureMin = glm::vec2(0.5, 0.0);
    textureMax = glm::vec2(1.0, 1.0);
    quadGeometries[1] =
      GlUtils::getQuadGeometry(
      geometryMin, geometryMax,
      textureMin, textureMax);
  }

  void onKey(int key, int scancode, int action, int mods) {
    if (GLFW_PRESS != action) {
      return;
    }

    switch (key) {
    case GLFW_KEY_O:
      nextImage(1);
      break;

    case GLFW_KEY_P:
      nextImage(-1);
      break;

    case GLFW_KEY_S:
      stereo = !stereo;
      break;

    default:
      GlfwApp::onKey(key, scancode, action, mods);
      break;
    }
  }

  void nextImage(int increment) {
    imageIndex = (imageIndex + STEREO_IMAGE_COUNT + increment) % STEREO_IMAGE_COUNT;
    GlUtils::getImageAsTexture(texture, STEREO_IMAGES[imageIndex], imageSize);
  }

  virtual void draw() {
    glClear(GL_COLOR_BUFFER_BIT);

    program->use();
    texture->bind();
    glm::uvec2 position(0, 0);
    gl::viewport(position, eyeSize);
    for (int i = 0; i < 2; ++i) {
      viewport(i);
      program->setUniform("Projection", projections[i]);
      int eyeIndex = stereo ? i : 0;
      quadGeometries[eyeIndex]->bindVertexArray();
      quadGeometries[eyeIndex]->draw();
    }
    VertexArray::unbind();
    Texture2d::unbind();
    Program::clear();
  }
};

RUN_OVR_APP(StereoscopicImages)
