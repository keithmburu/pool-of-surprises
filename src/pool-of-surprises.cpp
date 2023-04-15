/**
 * @file collision-physics.cpp
 * @author Keith Mburu
 * @date 2023-04-02
 * @brief Implements collision physics between sprites
 */

#include <cmath>
#include <string>
#include <vector>
#include <map>
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
  // vec3 gravity;
};

class Viewer : public Window {
public:
  Viewer() : Window() {
    _width = 500;
    _height = 500;
    _viewVolumeSide = _radius = 500;
    for (string effect : _chaosEffects) {
      _chaosEffectStatus[effect] = false;
    }
  }

  void setup() {
    setWindowSize(_width, _height);
    srand(time(nullptr));

    loadTextures();
    loadCubemaps();
    loadShaders();
    loadMeshes();

    createPoolBalls();
    createHoles();
  }

  void loadTextures() {
    for (int i = 0; i < 16; i++) {
      renderer.loadTexture("ball_" + to_string(i + 1), "../textures/pool-balls/ball_" + to_string(i + 1) + ".png", 0);
    }
    renderer.loadTexture("trajectoryBall", "../textures/pool-balls/ParticleBokeh.png", 0);
    renderer.loadTexture("pool-table", "../textures/pool-table/PoolTable_poolTable_BaseColor.png", 0);
    renderer.loadTexture("pool-table-normal", "../textures/pool-table/PoolTable_poolTable_Normal.png", 1);
    renderer.loadTexture("cue-stick", "../textures/cue-stick/Cue_diff.png", 0);
    renderer.loadTexture("explosion", "../textures/sprite-sheets/explosion.png", 0);
  }

  void loadCubemaps() {
    // renderer.loadCubemap("blue-photo-studio", "../cubemaps/blue-photo-studio", 5);
    // renderer.loadCubemap("colorful-studio", "../cubemaps/colorful-studio", 5);
    // renderer.loadCubemap("shanghai-bund", "../cubemaps/shanghai-bund", 5);
    renderer.loadCubemap("sea-cubemap", "../cubemaps/sea-cubemap", 5);
  }

  void loadShaders() {
    // renderer.loadShader("phong-pixel", "../shaders/phong-pixel.vs", "../shaders/phong-pixel.fs");
    // renderer.loadShader("texture", "../shaders/texture.vs", "../shaders/texture.fs");
    renderer.loadShader("cubemap", "../shaders/cubemap.vs", "../shaders/cubemap.fs");
    renderer.loadShader("billboard-animated", 
      "../shaders/billboard-animated.vs", 
      "../shaders/billboard-animated.fs");
  }

  void loadMeshes() {
    _poolTableMesh = PLYMesh("../models/pool-table.ply");
    _tableScaleVector = scaleVector(_poolTableMesh, "pool-table");
    _tableCenterVector = centerVector(_poolTableMesh, "pool-table");

    _cueStickMesh = PLYMesh("../models/cue-stick.ply");
    _stickScaleVector = scaleVector(_cueStickMesh, "cue-stick");
    _stickCenterVector = centerVector(_cueStickMesh, "cue-stick");
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
      _stickLength = int(windowZ * scaleFactor);
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

  void createPoolBalls()
  {
    for (int i = 0; i < _numBalls; i++) {
      Ball ball;
      ball.id = i;
      ball.pos.x = pow(-1, rand()) * (rand() % ((_tableLength - 100) / 2));
      ball.pos.y = pow(-1, rand()) * (rand() % ((_tableWidth - 100) / 2));
      // ball.pos.x = (float(i + 1) / _numBalls) * (_tableLength - 100);
      // ball.pos.x -= _tableLength / 2 - 50;
      // ball.pos.y = (float(i + 1) / _numBalls) * (_tableWidth - 100);  
      // ball.pos.y -= _tableWidth / 2 - 50;
      ball.vel = vec3(0);
      ball.color = vec4(1.0);
      ball.size = _viewVolumeSide / 20;
      // ball.gravity = vec3(0, 0, -0.05);
      _balls.push_back(ball);
    }
  }

  void createTrajecBalls() {
    for (int i = 0; i < 5; i++) {
      Ball trajectoryBall;
      trajectoryBall.pos = _balls[_activeBall].pos + ((1.0f / (i+1)) * _launchVel);
      trajectoryBall.color = vec4(0.8);
      trajectoryBall.size = 5 + (4 - i);
      _trajectoryBalls.push_back(trajectoryBall);
    }
  }

  void createHoles() {
    for (int i = 0; i < 6; i++) {
      vec3 hole;
      // hole.x = ((float(i % 3) / 2) * _tableLength) - (_tableLength / 2);
      hole.x = (i % 3) == 0? -(_tableLength / 2) + 50 : (i % 3) == 1? 0 :(_tableLength / 2) - 50;
      hole.y = (i < 3)? (_tableWidth / 2) - 50 : -(_tableWidth / 2) + 50;
      hole.z = 0;
      // cout << hole << endl;
      _holes.push_back(hole);
    }
  }

  void updatePoolBalls()
  {
    std::vector<Ball> newBalls;
    for (int i = 0; i < _numBalls; i++) {
      Ball ball = _balls[i];
      Ball newBall;
      newBall = holeDetection(ball);
      newBall = collisionDetection(newBall);
      newBall.pos += newBall.vel * dt();
      if (_chaosEffectStatus["tiltedTable"]) newBall.pos += vec3(10, 0, 0) * dt();
      newBall = boundaryDetection(newBall);
      // friction
      newBall.vel *= _chaosEffectStatus["moreFriction"]? 0.8f : 0.99f;
      // cout << "pos: " << newBall.pos << "  vel: " << newBall.vel << endl;
      newBalls.push_back(newBall);
    }
    _balls = newBalls;
  }

  Ball collisionDetection(Ball ball) {
    for (int j = 0; j < _numBalls; j++) {
      Ball otherBall = _balls[j];
      if (ball.id != otherBall.id && length(ball.pos - otherBall.pos) <= (_sphereDefaultRadius * ball.size + _sphereDefaultRadius * otherBall.size)) {
        // ball.pos += std::min<float>(_sphereDefaultRadius * ball.size, (ball.pos - otherBall.pos) / 2.0f);
        // ball.pos += (ball.pos - otherBall.pos) * (ball.size / (ball.size + otherBall.size));
        float overlap = (_sphereDefaultRadius * ball.size + _sphereDefaultRadius * otherBall.size) - length(ball.pos - otherBall.pos);
        ball.pos += normalize(ball.pos - otherBall.pos) * overlap / 2.0f;
        ball.vel = (otherBall.vel - ball.vel) / 2.0f;
        break;
      } 
    }
    return ball;
  }

  Ball boundaryDetection(Ball ball) {
    int ballRadius = _sphereDefaultRadius * ball.size;
    int xThresh = (_tableLength - 75) / 2;
    int ballLeft = ball.pos.x - ballRadius;
    int ballRight = ball.pos.x + ballRadius;
    if (ballLeft < -xThresh || ballRight > xThresh) {
      ball.pos.x += (ballLeft < -xThresh)? -xThresh - ballLeft : 0;
      ball.pos.x += (ballRight > xThresh)? xThresh - ballRight: 0;
      ball.vel.x = _chaosEffectStatus["stickyWalls"]? 0 : -ball.vel.x;
      ball.vel.y = _chaosEffectStatus["stickyWalls"]? 0 : ball.vel.y;
    }
    int yThresh = (_tableWidth - 75) / 2;
    int ballBottom = ball.pos.y - ballRadius;
    int ballTop = ball.pos.y + ballRadius;
    if (ball.pos.y < -yThresh || ball.pos.y > yThresh) {
      ball.pos.y += (ballBottom < -xThresh)? -yThresh - ballBottom : 0;
      ball.pos.y += (ballTop > xThresh)? yThresh - ballTop: 0;
      ball.vel.y = _chaosEffectStatus["stickyWalls"]? 0 : -ball.vel.y;
      ball.vel.x = _chaosEffectStatus["stickyWalls"]? 0 : ball.vel.x;
    }
    return ball;
  }

  Ball holeDetection(Ball ball) {
    for (int i = 0; i < 6; i++) {
      if (length(_holes[i] - ball.pos) < _viewVolumeSide / 100) {
          ball.pos.x = pow(-1, rand()) * (rand() % ((_tableLength - 100) / 2));
          ball.pos.y = pow(-1, rand()) * (rand() % ((_tableWidth - 100) / 2));
          ball.vel = vec3(0);
          ball.size = _viewVolumeSide / 20;
      } else if (length(_holes[i] - ball.pos) < _viewVolumeSide / 75) {
          // ball.vel += 0.1f * (_holes[i] - ball.pos);
          ball.size -= 0.5;
      } 
    }
    return ball;
  }

  int launchDetection(int x, int y) {
    float closestDist = 99999999;
    float closestDistIdx = -1;
    int clickX = x - (_width / 2);
    int clickY = -(y - (_height / 2));
    cout << "click pos: " << clickX << " " << clickY << endl;
    for (int i = 0; i < _numBalls; i++) {
      vec4 ballEyePos = renderer.viewMatrix() * vec4(_balls[i].pos, 1.0);
      vec2 ballScreenPos = vec2(ballEyePos.x, ballEyePos.y);
      // cout << i << ") " << screenPos << " vs " << clickX << " " << clickY << " dist: " << length(screenPos - vec2(clickX, clickY)) << endl;
      if (length(ballScreenPos - vec2(clickX, clickY)) < 30) {
        cout << "threshold crossed" << endl;
        float dist = length(ballScreenPos - vec2(clickX, clickY));
        if (dist < closestDist) {
          closestDist = dist;
          closestDistIdx = i;
          _launching = true;
        }
      } 
    }
    cout << endl;
    return closestDistIdx;
  }
  void drawPoolTable() {
    renderer.setUniform("materialColor", vec4(1));
    renderer.setUniform("poolBall", false);
    renderer.texture("image", "pool-table");
    // renderer.texture("bumpMap", "pool-table-normal");
    renderer.push();
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
      renderer.translate(-_launchVel * 0.2f);
      renderer.translate(-_launchVel * (0.5f * _stickLength / length(_launchVel)));
      // cout << _launchVel << " " << -_launchVel * (0.5f * _stickLength / length(_launchVel)) << endl;
      vec4 ballEyePos = renderer.viewMatrix() * vec4(_balls[_activeBall].pos, 1.0);
      vec3 ballScreenPos = vec3(ballEyePos.x, ballEyePos.y, 0);
      renderer.translate(ballScreenPos);
      renderer.rotate(vec3(0, 0, atan2(_launchVel.y, _launchVel.x) - M_PI_2));
      // renderer.translate(vec3(0, 0, 50));
      renderer.scale(_stickScaleVector); 
      renderer.rotate(vec3(0, M_PI_2, M_PI_2));   
      renderer.translate(_stickCenterVector);
      renderer.mesh(_cueStickMesh);
      renderer.pop(); 
    }
  }

  void drawPoolBalls() {
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
  }

  void drawTrajectoryBalls() {
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

  void chaos() {
    if (_chaosEffectStatus["floatingBalls"]) {
      for (int i = 0; i < _numBalls; i++) {
        if (_balls[i].pos.z >= 49) {
          _balls[i].pos.z = 50.0f + 10 * sin(elapsedTime());
        }
      }
    } 

    int newEffectThresh = std::max(300, rand() % 450);
    if (int((elapsedTime() + 1) / dt()) % newEffectThresh == 0) {
      _chaosAnimStartTime = elapsedTime() + 1;
      _chaosAnimation = true;
      string effect = _chaosEffects[rand() % _chaosEffects.size()];
      for (auto it = _chaosEffectStatus.begin(); it != _chaosEffectStatus.end(); it++) {
        if (it->first == effect) {
          if (it->second == false) {
            cout << "activating chaos effect: " << it->first << endl;
            _chaosEffectStatus[it->first] = true;
            _activeChaosEffect = effect;
            if (effect == "floatingBalls") {
              gravityChaos();
            } else if (effect == "largeBalls") {
              sizeChaos();
            }
          }
        } else {
          if (it->second == true) {
            cout << "deactivating chaos effect: " << it->first << endl;
            _chaosEffectStatus[it->first] = false;
            if (it->first == "floatingBalls") {
              resetGravity();
            } else if (it->first == "largeBalls") {
              resetSize();
            }
          }
        }
      }
    }
    // cout << "|" << elapsedTime() - _chaosAnimStartTime << "|";
    if (elapsedTime() - _chaosAnimStartTime > 3) {
      _chaosAnimation = false;
    } else {
      renderer.text(_activeChaosEffect + " activated!", 125, 150);
    }
  }

  void gravityChaos() {
    for (int i = 0; i < _numBalls; i++) {
      if (rand() % 4 == 0) {
        _balls[i].pos.z = 50.0f + 10 * sin(elapsedTime());
      }
    }
  }

  void resetGravity() {
    for (int i = 0; i < _numBalls; i++) {
      // _balls[i].gravity.z = -0.05f;
      _balls[i].pos.z = 0;
    }
  }

  void sizeChaos() {
    for (int i = 0; i < _numBalls; i++) {
      if (rand() % 4 == 0) {
        float prevSize = _balls[i].size;
        _balls[i].size *= 3;
        _balls[i].pos.z += _sphereDefaultRadius * (_balls[i].size - prevSize);
      }
    }
  }

  void resetSize() {
    for (int i = 0; i < _numBalls; i++) {
      float prevSize = _balls[i].size;
      _balls[i].size = _viewVolumeSide / 20;
      _balls[i].pos.z += _sphereDefaultRadius * (_balls[i].size - prevSize);
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
        int closestDistIdx = launchDetection(x, y);
        if (_launching) {
          _activeBall = closestDistIdx;
          // cout << "1) active ball: " << _activeBall << "  launchVel: " << _launchVel << endl;
          _balls[_activeBall].vel = vec3(0);
          _balls[_activeBall].color /= 2.0f;
          _launchVel = vec3(-dx, dy, 0);
          // vec4 launchVel = renderer.viewMatrix() * vec4(-dx, dy, 0, 0);
          // _launchVel = vec3(launchVel.x, launchVel.y, launchVel.z);
          // cout << _launchVel << endl;
          createTrajecBalls();
        } else {
          _orbiting = true;
        }
      }
      if (_orbiting) {
        pan(dx, dy);
      }
    }
  }

  void pan(float dx, float dy) {
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
        if (_chaosEffectStatus["invertedLaunch"]) {
          _balls[_activeBall].vel = vec3(_launchVel.x, -_launchVel.y, _launchVel.z);
        } else {
          _balls[_activeBall].vel = _launchVel;
        }
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
    renderer.perspective(glm::radians(60.0f), aspect, 0.1f, _viewVolumeSide * 10);
 
    _camPos = updatePos(0);
    renderer.lookAt(_camPos, _lookPos, _up);

    _lightPos = updatePos(M_PI_4);

    renderer.beginShader("cubemap");

    renderer.push();

    // cout << _chaosAnimation;
    if (_chaosAnimation) {
      renderer.setDepthTest(false);
      renderer.blendMode(agl::ADD);
      renderer.beginShader("billboard-animated");
      renderer.texture("image", "explosion");
      _time += dt();
      int numRows = 8;
      int numCols = 16;
      int frame = int(_time * 30) % (numRows * numCols);
      renderer.setUniform("Frame", frame);
      renderer.setUniform("Rows", numRows);
      renderer.setUniform("Cols", numCols);
      renderer.setUniform("TopToBottom", false);
      renderer.sprite(vec3(0, 0, 100), vec4(1.0f), 250.0);
      renderer.endShader();
      renderer.setDepthTest(true);
      renderer.blendMode(agl::DEFAULT);
    }

    // renderer.rotate(vec3(-M_PI_2, 0, 0));
    setupReflections();
    drawPoolTable();
    drawCueStick();
    updatePoolBalls();
    drawPoolBalls();
    drawTrajectoryBalls();
    chaos();
    renderer.pop();

    renderer.push();
    renderer.rotate(vec3(-M_PI_2, 0, 0));
    renderer.setUniform("ModelMatrix", renderer.modelMatrix());
    renderer.cubemap("cubemap", "sea-cubemap");
    renderer.setUniform("skybox", true);
    renderer.skybox(_viewVolumeSide * 5);
    renderer.setUniform("skybox", false);
    renderer.pop();

    renderer.endShader();

    // renderer.texture("fontTexture", "explosion");
    // renderer.text(" activated!", 125, 150);

    // renderer.beginShader("cubemap");
    // // renderer.cubemap("cubemap", "sea-cubemap");
    // renderer.cubemap("cubemap", "colorful-studio");
    // renderer.skybox(_viewVolumeSide * 5);
    // renderer.endShader();

  }

protected:

  int _width;
  int _height;
  int _viewVolumeSide;
  int _radius;
  float _sphereDefaultRadius = 0.5;

  vec3 _camPos = vec3(0, 0, _viewVolumeSide);
  vec3 _lookPos = vec3(0, 0, 0);
  vec3 _up = vec3(0, 1, 0);
  float _azimuth = 0;
  float _elevation = 0;
  float _orbiting = false;
  vec3 _lightPos = vec3(_viewVolumeSide, _viewVolumeSide, _viewVolumeSide);
  vec3 _lightColor = vec3(1.0, 1.0, 1.0);

  std::vector<Ball> _balls;
  int _numBalls = 16;

  bool _leftClick = false;
  bool _launching = false;
  std::vector<Ball> _trajectoryBalls;
  // std::vector<Ball> _trailingBalls;
  std::vector<vec3> _holes;
  int _activeBall;
  vec3 _launchVel;

  // vec3 _tableNormalForce = vec3(0, 0, 0.05);

  PLYMesh _poolTableMesh;
  int _tableLength;
  int _tableWidth;
  vec3 _tableScaleVector;
  vec3 _tableCenterVector;
  PLYMesh _cueStickMesh;
  int _stickLength;
  vec3 _stickScaleVector;
  vec3 _stickCenterVector;
  
  vector<string> _chaosEffects = {"vanilla", "stickyWalls", "floatingBalls", "largeBalls", "moreFriction", "tiltedTable", "invertedLaunch"};
  map<string, bool> _chaosEffectStatus;
  string _activeChaosEffect;
  float _time = 0.0f;
  bool _chaosAnimation = false;
  float _chaosAnimStartTime = 9999;

};

int main(int argc, char** argv)
{
  Viewer viewer;
  viewer.run();
  return 0;
}
