/**
 * @file sparkle-trail.cpp
 * @author Keith Mburu
 * @date 2023-04-02
 * @brief Implements star particle with trailing particles
 */

#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include "agl/window.h"
#include "unistd.h"

using namespace std;
using namespace glm;
using namespace agl;

struct Particle {
  glm::vec3 pos;
  glm::vec3 vel;
  glm::vec4 color;
  float size;
  float rot;
};

class Viewer : public Window {
public:
  Viewer() : Window() {
    _width = _height = 500;
    _radius = 20;
    srand(elapsedTime());
  }

  void setup() {
    setWindowSize(_width, _height);
    createSparkleTrail();
    renderer.setDepthTest(false);
    renderer.blendMode(agl::ADD);
    renderer.loadTexture("star", "../textures/star4.png", 0);
  }

void createSparkleTrail()
  {
    for (int i = 0; i < _numParticles; i++) {
      Particle particle;
      float time = elapsedTime();
      particle.pos = vec3(_radius * cos(time), _radius * sin(time), 0);
      vec3 nextPos = vec3(_radius * cos(time + dt()), _radius * sin(time + dt()), 0);
      if (i == 0) {
        particle.vel = nextPos - particle.pos;
        particle.color = vec4(1, 1, 1, 1);
      } else {
        particle.vel = particle.pos - nextPos;
        particle.vel.x += ((rand() % 10) / 10.0f) * particle.vel.x;
        particle.vel.y += ((rand() % 10) / 10.0f) * particle.vel.y;
        particle.color = vec4((rand() % 256)/255.0f, (rand() % 256)/255.0f, (rand() % 256)/255.0f, (rand() % 11) / 10.0f);
      }
      particle.size = 3.0;
      particle.rot = 0.0;
      mParticles.push_back(particle);
    }
  }

  void updateSparkleTrail()
  {
    for (int i = 1; i < mParticles.size(); i++) {
      Particle particle = mParticles[i];
      if (particle.color.w > 0) {
        particle.pos += particle.vel * dt();
        particle.color -= vec4(0, 0, 0, 0.002);
        particle.size += 0.001;
        particle.rot += 0.005;
      } else {
        particle.pos = mParticles[0].pos;
        particle.vel = -mParticles[0].vel;
        particle.vel.x += ((rand() % 10) / 10.0f) * particle.vel.x;
        particle.vel.y += ((rand() % 10) / 10.0f) * particle.vel.y;
        particle.color = vec4((rand() % 256)/255.0f, (rand() % 256)/255.0f, (rand() % 256)/255.0f, (rand() % 11) / 10.0f);
        particle.size = 3.0;
        particle.rot = 0.0;
      }
      mParticles[i] = particle;
    }

    mParticles[0].pos += mParticles[0].vel * dt();
    float time = elapsedTime();
    vec3 nextPos = vec3(_radius * cos(time + dt()), _radius * sin(time + dt()), 0);
    mParticles[0].vel = nextPos - mParticles[0].pos;
  }

  void drawSparkleTrail()
  {
    renderer.texture("image", "star");
    for (int i = 0; i < mParticles.size(); i++)
    {
      Particle particle = mParticles[i];
      renderer.sprite(particle.pos, particle.color, particle.size, particle.rot);
    }
  }

  void mouseMotion(int x, int y, int dx, int dy) {
  }

  void mouseDown(int button, int mods) {
  }

  void mouseUp(int button, int mods) {
  }

  void scroll(float dx, float dy) {
    eyePos.x += dy;
  }

  void keyUp(int key, int mods) {
  }

  void draw() {
    renderer.beginShader("sprite");

    float aspect = ((float)width()) / height();
    renderer.perspective(glm::radians(60.0f), aspect, 0.1f, 50.0f);
    renderer.lookAt(eyePos, lookPos, up);

    updateSparkleTrail();
    drawSparkleTrail();

    renderer.endShader();
  }

protected:

  vec3 eyePos = vec3(0, 0, 40);
  vec3 lookPos = vec3(0, 0, 0);
  vec3 up = vec3(0, 1, 0);

  std::vector<Particle> mParticles;
  int _numParticles = 20;
  int _width;
  int _height;
  int _radius;
};

int main(int argc, char** argv)
{
  Viewer viewer;
  viewer.run();
  return 0;
}
