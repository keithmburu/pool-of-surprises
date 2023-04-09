/**
 * @file collision-physics.cpp
 * @author Keith Mburu
 * @date 2023-04-02
 * @brief Implements collision physics between sprites
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

struct Ball {
  int id;
  glm::vec3 pos;
  glm::vec3 vel;
  glm::vec4 color;
  float size;
};

class Viewer : public Window {
public:
  Viewer() : Window() {
    _width = _height = 500;
    _radius = _viewVolumeSide;
    srand(elapsedTime());
  }

  void setup() {
    setWindowSize(_width, _height);
    createBalls();
    renderer.setDepthTest(false);
    renderer.blendMode(agl::DEFAULT);
    renderer.loadTexture("ball", "../textures/white-circle.png", 0);
    renderer.loadShader("phong-pixel", "../shaders/phong-pixel.vs", "../shaders/phong-pixel.fs");
  }

void createBalls()
  {
    for (int i = 0; i < _numBalls; i++) {
      Ball ball;
      ball.id = i;
      ball.pos.x = pow(-1, rand()) * (rand() % (_viewVolumeSide / 2));
      ball.pos.y = pow(-1, rand()) * (rand() % (_viewVolumeSide / 2));
      // ball.vel = vec3(random(-0.005, 0.005), random(-0.005, 0.005), 0);
      ball.vel = vec3(0);
      ball.color.x = std::max(rand() % 256, 64) / 255.0f;
      ball.color.y = std::max(rand() % 256, 64) / 255.0f;
      ball.color.z = std::max(rand() % 256, 64) / 255.0f;
      ball.color.w = 1.0;
      ball.size = 50.0;
      _Balls.push_back(ball);
    }
  }

 void updateBalls()
  {
    std::vector<Ball> newBalls = _Balls;
    for (int i = 0; i < _numBalls; i++) {
      Ball ball = _Balls[i];
      Ball newBall = newBalls[i];
      for (int j = 0; j < _numBalls; j++) {
        Ball otherBall = _Balls[j];
        if (ball.id != otherBall.id && length(ball.pos - otherBall.pos) <= 3.5) {
          newBall.vel = (otherBall.vel - ball.vel) / 2.0f;
          break;
        } 
      }
      newBall.pos += newBall.vel * dt();
      if (newBall.pos.x < -250 || newBall.pos.x > 250) {
        newBall.pos.x = (newBall.pos.x < -250)? -250 : newBall.pos.x;
        newBall.pos.x = (newBall.pos.x > 250)? 250 : newBall.pos.x;
        newBall.vel.x = -newBall.vel.x;
      }
      if (newBall.pos.y < -250 || newBall.pos.y > 250) {
        newBall.pos.y = (newBall.pos.y < -250)? -250 : newBall.pos.y;
        newBall.pos.y = (newBall.pos.y > 250)? 250 : newBall.pos.y;
        newBall.vel.y = -newBall.vel.y;
      }
      // cout << "pos: " << newBall.pos << "  vel: " << newBall.vel << endl;
      newBalls[i] = newBall;
    }
    _Balls = newBalls;
  }

 void drawBalls()
  {
    vec3 cameraPos = renderer.cameraPosition();

    // sort
    for (int i = 1; i < _Balls.size(); i++)
    {
      Ball ball1 = _Balls[i];
      Ball ball2 = _Balls[i - 1];
      float dSqr1 = length2(ball1.pos - cameraPos);
      float dSqr2 = length2(ball2.pos - cameraPos);
      if (dSqr2 < dSqr1)
      {
        _Balls[i] = ball2;
        _Balls[i - 1] = ball1;
      }
    }

    // draw
    renderer.texture("image", "ball");
    for (int i = 0; i < _numBalls; i++)
    {
      // renderer.sprite(_Balls[i].pos, _Balls[i].color, _Balls[i].size, 0.0);
      renderer.push();
      renderer.translate(_Balls[i].pos);
      renderer.scale(vec3(_Balls[i].size));
      // setting phong shading uniforms
      setupReflections();
      renderer.sphere();
      renderer.pop();
    }
  }

void setupReflections() {
  renderer.setUniform("ViewMatrix", renderer.viewMatrix());
  renderer.setUniform("ProjMatrix", renderer.projectionMatrix());
  renderer.setUniform("lightPos", _lightPos);
  renderer.setUniform("lightColor", _lightColor.x, _lightColor.y, _lightColor.z);
  renderer.setUniform("materialColor", vec4(1.0, 0, 0, 1.0));
  renderer.setUniform("eyePos", _camPos);
  float ka = 0.1, kd = 0.7, ks = 0.6;
  float phongExp = 50.0;
  renderer.setUniform("ka", ka);
  renderer.setUniform("kd", kd);
  renderer.setUniform("ks", ks);
  renderer.setUniform("phongExp", phongExp);
}

void mouseMotion(int x, int y, int dx, int dy) {
      if (_leftClick) {
        if (_launching) {
          _launchVel += vec3(-dx, dy, 0);
          // cout << _launchVel << endl;
        } else if (!_orbiting) {
          float closestDist = 99999999;
          float closestDistIdx = -1;
          for (int i = 0; i < _numBalls; i++) {
            // int clickX = (x / 10.0f) - 25;
            // int clickY = -((y / 10.0f) - 25);
            int clickX = x - (_viewVolumeSide / 2);
            int clickY = -(y - (_viewVolumeSide / 2));
            cout << _Balls[i].pos << " vs " << clickX << " " << clickY << "  diff: " << abs(_Balls[i].pos.x - clickX) << " " << abs(_Balls[i].pos.y - clickY) << " dist: " << length(_Balls[i].pos - vec3(clickX, clickY, 0)) << endl;
            if (length(_Balls[i].pos - vec3(clickX, clickY, 0)) < 30) {
              cout << "threshold crossed" << endl;
              float dist = length(_Balls[i].pos - vec3(clickX, clickY, 0));
              if (dist < closestDist) {
                closestDist = dist;
                closestDistIdx = i;
              }
              _launching = true;
            } 
          }
          if (_launching) {
            _Balls[closestDistIdx].vel = vec3(0);
            _Balls[closestDistIdx].color /= 2.0f;
            _activeBall = closestDistIdx;
            _launchVel = vec3(-dx, dy, 0);
            // cout << _launchVel << endl;
          } else {
            // orbit
            _orbiting = true;
          }
        }
        if (_orbiting) {
          float ONE_DEG = 0.017; // one degree in radians
          _elevation += dy * (M_PI / 180);
          if (_elevation > (M_PI / 2) - ONE_DEG) {
             _elevation = (M_PI / 2) - ONE_DEG;
          } else if (_elevation < -((M_PI / 2) - ONE_DEG)) {
             _elevation = -((M_PI / 2) - ONE_DEG);
          }
          _azimuth -= dx * (M_PI / 180);
          if (_azimuth > 2 * M_PI) {
             _azimuth = 0;
          } else if (_azimuth < 0) {
             _azimuth = 2 * M_PI;
          }
        }
      }
   }

   void mouseDown(int button, int mods) {
      if (button == GLFW_MOUSE_BUTTON_LEFT) {
         _leftClick = true;
         _leftClickTime = elapsedTime();
      }
   }

   void mouseUp(int button, int mods) {
      if (button == GLFW_MOUSE_BUTTON_LEFT) {
        _leftClick = false;
        if (_launching) {
          // cout << "active ball: " << _activeBall << "  launchVel: " << _launchVel << endl;
          _Balls[_activeBall].vel = _launchVel;
          _Balls[_activeBall].color *= 2.0f;
          _launching = false;
        } else if (_orbiting) {
          _orbiting = false;
        }
      }
   }

  void scroll(float dx, float dy) {
    // scroll up or down to zoom in or out
    _radius -= 10 * dy;
    if (_radius < 1) {
       _radius = 1;
    }
    // scroll sideways to slowly pan sideways
    _azimuth += 10 * dx * (M_PI / 180);
    if (_azimuth > 2 * M_PI) {
       _azimuth = 0;
    } else if (_azimuth < 0) {
       _azimuth = 2 * M_PI;
    }
  }

  void keyUp(int key, int mods) {
    if (key == GLFW_KEY_R) {
      for (int i = 0; i < _numBalls; i++) {
        _Balls[i].vel = vec3(0);
      }  
    }
  }

  void draw() {
    // renderer.beginShader("sprite");
    renderer.beginShader("phong-pixel");

    float aspect = ((float)width()) / height();
    renderer.perspective(glm::radians(60.0f), aspect, 0.1f, _viewVolumeSide * 2);

    float camPosX = _radius * sin(_azimuth) * cos(_elevation);
    float camPosY = _radius * sin(_elevation);
    float camPosZ = _radius * cos(_azimuth) * cos(_elevation);
    _camPos = vec3(camPosX, camPosY, camPosZ);
    renderer.lookAt(_camPos, _lookPos, _up);

    float lightPosX = _radius * sin(_azimuth + M_PI / 4) * cos(_elevation + M_PI / 4);
    float lightPosY = _radius * sin(_elevation + M_PI / 4);
    float lightPosZ = _radius * cos(_azimuth + M_PI / 4) * cos(_elevation + M_PI / 4);
    _lightPos = vec3(lightPosX, lightPosY, lightPosZ);

    updateBalls();
    drawBalls();

    renderer.endShader();
    // renderer.endShader();
  }

protected:

  int _viewVolumeSide = 500;
  vec3 _camPos = vec3(0, 0, _viewVolumeSide);
  vec3 _lookPos = vec3(0, 0, 0);
  vec3 _up = vec3(0, 1, 0);

  std::vector<Ball> _Balls;
  int _numBalls = 10;
  int _width;
  int _height;
  int _radius;

  bool _leftClick = false;
  float _leftClickTime;
  bool _launching = false;
  int _activeBall;
  vec3 _launchVel;
  vec3 _lightPos = vec3(_viewVolumeSide, _viewVolumeSide, _viewVolumeSide);
  vec3 _lightColor = vec3(1.0, 1.0, 1.0);
  float _azimuth = 0;
  float _elevation = 0;
  float _orbiting = false;
};

int main(int argc, char** argv)
{
  Viewer viewer;
  viewer.run();
  return 0;
}
