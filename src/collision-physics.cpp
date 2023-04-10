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
#include <ctime>
#include "plymesh.h"

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
    _width = 500;
    _height = 500;
    _viewVolumeSide = 500;
  }

  void setup() {
    setWindowSize(_width, _height);
    _radius = _viewVolumeSide;
    // srand(time(nullptr));

    renderer.loadTexture("ball", "../textures/white-circle.png", 0);
    renderer.loadTexture("trajectoryBall", "../textures/ParticleBokeh.png", 0);
    renderer.loadTexture("pool-table", "../textures/PoolTable_poolTable_BaseColor.png", 0);
    renderer.loadTexture("cow", "../textures/droplets-texture.jpeg", 0);
    renderer.loadShader("phong-pixel", "../shaders/phong-pixel.vs", "../shaders/phong-pixel.fs");
    renderer.loadShader("texture", "../shaders/texture.vs", "../shaders/texture.fs");
    renderer.loadShader("cubemap", "../shaders/cubemap.vs", "../shaders/cubemap.fs");

    _poolTableMesh = PLYMesh("../models/pool-table.ply");
    // _poolTableMesh = PLYMesh("../models/cow.ply");

    vec3 minBounds = _poolTableMesh.minBounds();
    vec3 maxBounds = _poolTableMesh.maxBounds();
    float windowX = abs(minBounds[0]) + (maxBounds[0]);
    float windowY = abs(minBounds[1]) + (maxBounds[1]);
    float windowZ = abs(minBounds[2]) + (maxBounds[2]);
    // float xScaleFactor = _height / windowX;
    // float yScaleFactor = _width / windowY;
    // float zScaleFactor = _viewVolumeSide / windowZ;
    // float scaleFactor = std::min(std::min(xScaleFactor, yScaleFactor), zScaleFactor);
    float maxDimension = std::max(std::max(windowX, windowY), windowZ);
    float scaleFactor = _viewVolumeSide / maxDimension;
    _tableLength = int(windowY * scaleFactor);
    _tableWidth = int(windowX * scaleFactor);
    cout << _tableLength << " " << _tableWidth << endl;
    _scaleVector = vec3(scaleFactor);

    float centroidX = (minBounds[0] + maxBounds[0]) / 2.0f;
    float centroidY = (minBounds[1] + maxBounds[1]) / 2.0f;
    float centroidZ = (minBounds[2] + maxBounds[2]) / 2.0f;
    _centerVector = vec3(-centroidX, -centroidY, -centroidZ);

    createBalls();
  }

  void createBalls()
  {
    for (int i = 0; i < _numBalls; i++) {
      Ball ball;
      ball.id = i;
      ball.pos.x = pow(-1, rand()) * (rand() % ((_tableLength - 100) / 2));
      ball.pos.y = pow(-1, rand()) * (rand() % ((_tableWidth - 100) / 2));
      // ball.vel = vec3(random(-0.005, 0.005), random(-0.005, 0.005), 0);
      ball.vel = vec3(0);
      ball.color.x = std::max(rand() % 256, 64) / 255.0f;
      ball.color.y = std::max(rand() % 256, 64) / 255.0f;
      ball.color.z = std::max(rand() % 256, 64) / 255.0f;
      ball.color.w = 1.0;
      ball.size = _viewVolumeSide / 20;
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
        if (ball.id != otherBall.id && length(ball.pos - otherBall.pos) <= _viewVolumeSide / 20) {
          newBall.vel = (otherBall.vel - ball.vel) / 2.0f;
          break;
        } 
      }
      newBall.pos += newBall.vel * dt();
      int xThresh = (_tableLength - 100) / 2;
      if (newBall.pos.x < -xThresh || newBall.pos.x > xThresh) {
        newBall.pos.x = (newBall.pos.x < -xThresh)? -xThresh : newBall.pos.x;
        newBall.pos.x = (newBall.pos.x > xThresh)? xThresh : newBall.pos.x;
        newBall.vel.x = -newBall.vel.x;
      }
      int yThresh = (_tableWidth - 100) / 2;
      if (newBall.pos.y < -yThresh || newBall.pos.y > yThresh) {
        newBall.pos.y = (newBall.pos.y < -yThresh)? -yThresh : newBall.pos.y;
        newBall.pos.y = (newBall.pos.y > yThresh)? yThresh : newBall.pos.y;
        newBall.vel.y = -newBall.vel.y;
      }
      // cout << "pos: " << newBall.pos << "  vel: " << newBall.vel << endl;
      newBalls[i] = newBall;
    }
    _Balls = newBalls;
  }

  void drawTable() {
    renderer.push();
    renderer.beginShader("texture");
    renderer.texture("image", "pool-table");
    // renderer.texture("image", "cow");
    // center table, align it horizontally, and scale to fit view volume
    renderer.translate(vec3(0, 0, -100));  
    renderer.scale(_scaleVector);   
    // renderer.rotate(vec3(0, M_PI / 4, M_PI / 2));   
    renderer.rotate(vec3(0, 0, M_PI / 2));
    renderer.translate(_centerVector);
    renderer.mesh(_poolTableMesh);
    renderer.endShader();
    renderer.pop();
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
    renderer.push();
    // renderer.rotate(vec3(-M_PI / 4, 0, 0));  
    // renderer.translate(vec3(0, 0, 150));  

    renderer.texture("image", "ball");
    renderer.beginShader("phong-pixel");
    for (int i = 0; i < _numBalls; i++)
    {
      renderer.push();
      renderer.translate(_Balls[i].pos);
      renderer.scale(vec3(_Balls[i].size));
      // setting phong shading uniforms
      setupReflections(i);
      renderer.sphere();
      vec4 eyePos = renderer.viewMatrix() * vec4(_Balls[i].pos, 1.0);
      vec2 screenPos = vec2(eyePos.x, eyePos.y);
      renderer.text(to_string(_Balls[i].id), screenPos.x + (_height / 2), _width - (screenPos.y + (_width / 2)));
      renderer.pop();
    }
    renderer.endShader();

    if (_launching) {
      renderer.texture("image", "trajectoryBall");
      
      renderer.setDepthTest(false);
      renderer.blendMode(agl::ADD);
      renderer.beginShader("sprite");
      for (int i = 0; i < _trajectoryBalls.size(); i++)
      {
        renderer.sprite(_trajectoryBalls[i].pos, _trajectoryBalls[i].color, 
        _trajectoryBalls[i].size, 0.0);
      }
      renderer.endShader();
      renderer.setDepthTest(true);
      renderer.blendMode(agl::DEFAULT);
    }
 
    renderer.pop();
  }

  void setupReflections(int ballIdx) {
    renderer.setUniform("ViewMatrix", renderer.viewMatrix());
    renderer.setUniform("ProjMatrix", renderer.projectionMatrix());
    renderer.setUniform("lightPos", _lightPos);
    renderer.setUniform("lightColor", _lightColor.x, _lightColor.y, _lightColor.z);
    renderer.setUniform("materialColor", _Balls[ballIdx].color);
    renderer.setUniform("eyePos", _camPos);
    float ka = 0.1, kd = 0.7, ks = 0.6;
    float phongExp = 50.0;
    renderer.setUniform("ka", ka);
    renderer.setUniform("kd", kd);
    renderer.setUniform("ks", ks);
    renderer.setUniform("phongExp", phongExp);
  }

  vec3 updatePos(float offset) {
    float x = _radius * sin(_azimuth + offset) * cos(_elevation + offset);
    float y = _radius * sin(_elevation + offset);
    float z = _radius * cos(_azimuth + offset) * cos(_elevation + offset);
    return vec3(x, y, z);
  }

  void mouseMotion(int x, int y, int dx, int dy) {
    if (_leftClick) {
      if (_launching) {
        _launchVel += vec3(-dx, dy, 0);
        // cout << "3) active ball: " << _activeBall << "  launchVel: " << _launchVel << endl;
        // vec4 launchVel = vec4(_launchVel, 1.0) + (renderer.viewMatrix() * vec4(-dx, dy, 0, 0));
        // _launchVel = vec3(launchVel.x, launchVel.y, launchVel.z);
        // cout << _launchVel << endl;
        for (int i = 0; i < _trajectoryBalls.size(); i++) {
          _trajectoryBalls[i].pos = _Balls[_activeBall].pos + ((1.0f / (i+1)) * _launchVel);
        }
      } else if (!_orbiting) {
        float closestDist = 99999999;
        float closestDistIdx = -1;
        for (int i = 0; i < _numBalls; i++) {
          int clickX = x - (_width / 2);
          int clickY = -(y - (_height / 2));
          vec4 eyePos = renderer.viewMatrix() * vec4(_Balls[i].pos, 1.0);
          vec2 screenPos = vec2(eyePos.x, eyePos.y);
          cout << i << ") " << screenPos << " vs " << clickX << " " << clickY << " dist: " << length(screenPos - vec2(clickX, clickY)) << endl;
          if (length(screenPos - vec2(clickX, clickY)) < 30) {
            cout << "threshold crossed" << endl;
            float dist = length(screenPos - vec2(clickX, clickY));
            if (dist < closestDist) {
              closestDist = dist;
              closestDistIdx = i;
              _launching = true;
            }
          } 
        }
        cout << endl;
        if (_launching) {
          _activeBall = closestDistIdx;
          // cout << "1) active ball: " << _activeBall << "  launchVel: " << _launchVel << endl;
          _Balls[_activeBall].vel = vec3(0);
          _Balls[_activeBall].color /= 2.0f;
          _launchVel = vec3(-dx, dy, 0);
          // vec4 launchVel = renderer.viewMatrix() * vec4(-dx, dy, 0, 0);
          // _launchVel = vec3(launchVel.x, launchVel.y, launchVel.z);
          // cout << _launchVel << endl;
          for (int i = 0; i < 5; i++) {
            Ball trajectoryBall;
            trajectoryBall.pos = _Balls[_activeBall].pos + ((1.0f / (i+1)) * _launchVel);
            trajectoryBall.color = vec4(0.8);
            trajectoryBall.size = 5 + i;
            _trajectoryBalls.push_back(trajectoryBall);
          }
        } else {
          _orbiting = true;
        }
      }
      if (_orbiting) {
        float ONE_DEG = 0.017;
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
    }
  }

  void mouseUp(int button, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
      _leftClick = false;
      if (_launching) {
        // cout << "2) active ball: " << _activeBall << "  launchVel: " << _launchVel << endl;
        _Balls[_activeBall].vel = _launchVel;
        _Balls[_activeBall].color *= 2.0f;
        _launching = false;
        _trajectoryBalls.clear();
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
    _azimuth -= 10 * dx * (M_PI / 180);
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
    float aspect = ((float) width()) / height();
    renderer.perspective(glm::radians(60.0f), aspect, 0.1f, _viewVolumeSide * 2);
 
    _camPos = updatePos(0);
    renderer.lookAt(_camPos, _lookPos, _up);

    _lightPos = updatePos(M_PI / 4);

    drawTable();

    updateBalls();
    drawBalls();

  }

protected:

  int _viewVolumeSide;
  vec3 _camPos = vec3(0, 0, _viewVolumeSide);
  vec3 _lookPos = vec3(0, 0, 0);
  vec3 _up = vec3(0, 1, 0);

  std::vector<Ball> _Balls;
  int _numBalls = 10;
  int _width;
  int _height;
  int _radius;

  bool _leftClick = false;
  bool _launching = false;
  std::vector<Ball> _trajectoryBalls;
  // std::vector<Ball> _trailingBalls;
  int _activeBall;
  vec3 _launchVel;

  vec3 _lightPos = vec3(_viewVolumeSide, _viewVolumeSide, _viewVolumeSide);
  vec3 _lightColor = vec3(1.0, 1.0, 1.0);
  float _azimuth = 0;
  float _elevation = 0;
  float _orbiting = false;
  int _tableLength;
  int _tableWidth;

  PLYMesh _poolTableMesh;
  vec3 _centerVector;
  vec3 _scaleVector;
};

int main(int argc, char** argv)
{
  Viewer viewer;
  viewer.run();
  return 0;
}
