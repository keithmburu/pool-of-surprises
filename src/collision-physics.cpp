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
    srand(time(nullptr));

    // renderer.loadTexture("ball", "../textures/white-circle.png", 0);
    for (int i = 0; i < 16; i++) {
      renderer.loadTexture("ball_" + to_string(i + 1), "../textures/ball_" + to_string(i + 1) + ".png", 0);
    }
    renderer.loadTexture("trajectoryBall", "../textures/ParticleBokeh.png", 0);
    renderer.loadTexture("pool-table", "../textures/PoolTable_poolTable_BaseColor.png", 0);
    renderer.loadTexture("cue-stick", "../textures/Cue_diff.png", 0);
    // renderer.loadTexture("cow", "../textures/droplets-texture.jpeg", 0);
    renderer.loadShader("phong-pixel", "../shaders/phong-pixel.vs", "../shaders/phong-pixel.fs");
    renderer.loadShader("texture", "../shaders/texture.vs", "../shaders/texture.fs");
    renderer.loadShader("cubemap", "../shaders/cubemap.vs", "../shaders/cubemap.fs");

    _poolTableMesh = PLYMesh("../models/pool-table.ply");
    _cueStickMesh = PLYMesh("../models/cue-stick.ply");
    // _poolTableMesh = PLYMesh("../models/cow.ply");

    _tableScaleVector = scaleVector(_poolTableMesh, "pool-table");
    _tableCenterVector = centerVector(_poolTableMesh, "pool-table");

    _stickScaleVector = scaleVector(_cueStickMesh, "cue-stick");
    _stickCenterVector = centerVector(_cueStickMesh, "cue-stick");

    createBalls();
  }

  vec3 scaleVector(PLYMesh mesh, string meshName) {
    vec3 minBounds = mesh.minBounds();
    vec3 maxBounds = mesh.maxBounds();
    float windowX = abs(minBounds[0]) + (maxBounds[0]);
    float windowY = abs(minBounds[1]) + (maxBounds[1]);
    float windowZ = abs(minBounds[2]) + (maxBounds[2]);
    float maxDimension = std::max(std::max(windowX, windowY), windowZ);
    float scaleFactor = _viewVolumeSide / maxDimension;
    if (meshName == "pool-table") {
      _tableLength = int(windowY * scaleFactor);
      _tableWidth = int(windowX * scaleFactor);
    } else if (meshName == "cue-stick") {
      _stickLength = int(windowY * scaleFactor);
    }
    return vec3(scaleFactor);
  }

 vec3 centerVector(PLYMesh mesh, string meshName) {
    vec3 minBounds = mesh.minBounds();
    vec3 maxBounds = mesh.maxBounds();
    float centroidX = (minBounds[0] + maxBounds[0]) / 2.0f;
    float centroidY = (minBounds[1] + maxBounds[1]) / 2.0f;
    float centroidZ = (minBounds[2] + maxBounds[2]) / 2.0f;
    return vec3(-centroidX, -centroidY, -centroidZ);
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
      // ball.color.x = std::max(rand() % 256, 64) / 255.0f;
      // ball.color.y = std::max(rand() % 256, 64) / 255.0f;
      // ball.color.z = std::max(rand() % 256, 64) / 255.0f;
      // ball.color.w = 1.0;
      ball.color = vec4(1.0);
      ball.size = _viewVolumeSide / 20;
      _balls.push_back(ball);
    }
    for (int i = 0; i < 6; i++) {
      vec3 hole;
      // hole.x = ((float(i % 3) / 2) * _tableLength) - (_tableLength / 2);
      hole.x = (i % 3) == 0? -(_tableLength / 2) + 50 : (i % 3) == 1? 0 :(_tableLength / 2) - 50;
      hole.y = (i < 3)? (_tableWidth / 2) - 50 : -(_tableWidth / 2) + 50;
      hole.z = 0;
      cout << hole << endl;
      _holes.push_back(hole);
    }
  }

  void updateBalls()
  {
    std::vector<Ball> newBalls = _balls;
    for (int i = 0; i < _numBalls; i++) {
      Ball ball = _balls[i];
      Ball newBall = newBalls[i];
      for (int j = 0; j < _numBalls; j++) {
        Ball otherBall = _balls[j];
        if (ball.id != otherBall.id && length(ball.pos - otherBall.pos) <= _viewVolumeSide / 20) {
          newBall.vel = (otherBall.vel - ball.vel) / 2.0f;
          break;
        } 
      }
      newBall.pos += newBall.vel * dt();
      int xThresh = (_tableLength - 75) / 2;
      if (newBall.pos.x < -xThresh || newBall.pos.x > xThresh) {
        newBall.pos.x = (newBall.pos.x < -xThresh)? -xThresh : newBall.pos.x;
        newBall.pos.x = (newBall.pos.x > xThresh)? xThresh : newBall.pos.x;
        newBall.vel.x = -newBall.vel.x;
      }
      int yThresh = (_tableWidth - 75) / 2;
      if (newBall.pos.y < -yThresh || newBall.pos.y > yThresh) {
        newBall.pos.y = (newBall.pos.y < -yThresh)? -yThresh : newBall.pos.y;
        newBall.pos.y = (newBall.pos.y > yThresh)? yThresh : newBall.pos.y;
        newBall.vel.y = -newBall.vel.y;
      }
      for (int i = 0; i < 6; i++) {
        if (length(_holes[i] - newBall.pos) < _viewVolumeSide / 100) {
            newBall.pos.x = pow(-1, rand()) * (rand() % ((_tableLength - 100) / 2));
            newBall.pos.y = pow(-1, rand()) * (rand() % ((_tableWidth - 100) / 2));
            newBall.vel = vec3(0);
            newBall.size = _viewVolumeSide / 20;
        } else if (length(_holes[i] - newBall.pos) < _viewVolumeSide / 75) {
            // newBall.vel += 0.1f * (_holes[i] - newBall.pos);
            newBall.size -= 0.5;
        } 
      }
      // cout << "pos: " << newBall.pos << "  vel: " << newBall.vel << endl;
      newBalls[i] = newBall;
    }
    _balls = newBalls;
  }

  void drawTable() {
    renderer.setUniform("materialColor", vec4(1));
    renderer.setUniform("poolBall", false);
    renderer.texture("image", "pool-table");
    renderer.push();
    // renderer.texture("image", "cow");
    // center table, align it horizontally, and scale to fit view volume
    renderer.translate(vec3(0, 0, -75));  
    renderer.scale(_tableScaleVector);   
    // renderer.rotate(vec3(0, M_PI_4, M_PI_2));   
    renderer.rotate(vec3(0, 0, M_PI_2));
    renderer.translate(_tableCenterVector);
    renderer.mesh(_poolTableMesh);
    renderer.pop();
  }

  void drawCueStick() {
    if (_launching) {
      renderer.setUniform("materialColor", vec4(1));
      renderer.setUniform("poolBall", false);
      renderer.texture("image", "cue-stick");
      renderer.push();
      // center cue stick, align it horizontally, and scale to fit view volume
      int stickEnd = _stickLength / 2;
      vec3 stickEndPos = vec3(stickEnd, 0, 0);
      renderer.translate(_balls[_activeBall].pos - stickEndPos);
      renderer.translate(vec3(0, 0, 100));
      renderer.rotate(_launchVel);
      renderer.scale(_stickScaleVector);   
      renderer.rotate(vec3(0, M_PI_2, 0));   
      renderer.translate(_stickCenterVector);
      renderer.mesh(_cueStickMesh);
      renderer.pop();
    }
  }

  void drawBalls()
  {
    for (int i = 0; i < _numBalls; i++)
    {
      renderer.setUniform("materialColor", _balls[i].color);
      renderer.setUniform("poolBall", true);
      // renderer.texture("image", "ball");
      renderer.texture("image", "ball_" + to_string(i + 1));
      renderer.push();
      renderer.translate(_balls[i].pos);
      renderer.rotate(vec3(0, M_PI_2, M_PI_2));
      renderer.scale(vec3(_balls[i].size));
      // setting phong shading uniforms
      renderer.sphere();
      renderer.pop();
      // vec4 eyePos = renderer.viewMatrix() * vec4(_balls[i].pos, 1.0);
      // vec2 screenPos = vec2(eyePos.x, eyePos.y);
      // renderer.text(to_string(_balls[i].id), screenPos.x + (_height / 2), _width - (screenPos.y + (_width / 2)));
    }

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
  }

  void setupReflections() {
    renderer.setUniform("ViewMatrix", renderer.viewMatrix());
    renderer.setUniform("ProjMatrix", renderer.projectionMatrix());
    renderer.setUniform("lightPos", _lightPos);
    renderer.setUniform("lightColor", _lightColor.x, _lightColor.y, _lightColor.z);
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
          _trajectoryBalls[i].pos = _balls[_activeBall].pos + ((1.0f / (i+1)) * _launchVel);
        }
      } else if (!_orbiting) {
        float closestDist = 99999999;
        float closestDistIdx = -1;
        int clickX = x - (_width / 2);
        int clickY = -(y - (_height / 2));
        cout << "Click pos: " << clickX << " " << clickY << endl;
        for (int i = 0; i < _numBalls; i++) {
          vec4 eyePos = renderer.viewMatrix() * vec4(_balls[i].pos, 1.0);
          vec2 screenPos = vec2(eyePos.x, eyePos.y);
          // cout << i << ") " << screenPos << " vs " << clickX << " " << clickY << " dist: " << length(screenPos - vec2(clickX, clickY)) << endl;
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
          _balls[_activeBall].vel = vec3(0);
          _balls[_activeBall].color /= 2.0f;
          _launchVel = vec3(-dx, dy, 0);
          // vec4 launchVel = renderer.viewMatrix() * vec4(-dx, dy, 0, 0);
          // _launchVel = vec3(launchVel.x, launchVel.y, launchVel.z);
          // cout << _launchVel << endl;
          for (int i = 0; i < 5; i++) {
            Ball trajectoryBall;
            trajectoryBall.pos = _balls[_activeBall].pos + ((1.0f / (i+1)) * _launchVel);
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
        if (_elevation > (M_PI_2) - ONE_DEG) {
           _elevation = (M_PI_2) - ONE_DEG;
        } else if (_elevation < -((M_PI_2) - ONE_DEG)) {
           _elevation = -((M_PI_2) - ONE_DEG);
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
        _balls[_activeBall].vel = _launchVel;
        _balls[_activeBall].color *= 2.0f;
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
        _balls[i].vel = vec3(0);
      }  
    }
  }

  void draw() {
    float aspect = ((float) width()) / height();
    renderer.perspective(glm::radians(60.0f), aspect, 0.1f, _viewVolumeSide * 2);
 
    _camPos = updatePos(0);
    renderer.lookAt(_camPos, _lookPos, _up);

    _lightPos = updatePos(M_PI_4);

    renderer.beginShader("phong-pixel");
    setupReflections();
    drawTable();
    drawCueStick();
    updateBalls();
    drawBalls();
    renderer.endShader();

  }

protected:

  int _viewVolumeSide;
  vec3 _camPos = vec3(0, 0, _viewVolumeSide);
  vec3 _lookPos = vec3(0, 0, 0);
  vec3 _up = vec3(0, 1, 0);

  std::vector<Ball> _balls;
  int _numBalls = 16;
  int _width;
  int _height;
  int _radius;

  bool _leftClick = false;
  bool _launching = false;
  std::vector<Ball> _trajectoryBalls;
  // std::vector<Ball> _trailingBalls;
  std::vector<vec3> _holes;
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
  PLYMesh _cueStickMesh;
  vec3 _tableScaleVector;
  vec3 _stickScaleVector;
  vec3 _tableCenterVector;
  vec3 _stickCenterVector;
  int _stickLength;
};

int main(int argc, char** argv)
{
  Viewer viewer;
  viewer.run();
  return 0;
}
