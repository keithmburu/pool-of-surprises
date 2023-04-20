/**
 * @file pool-of-surprises.cpp
 * @author Keith Mburu
 * @date 2023-04-18
 * @brief Implements a version of pool that's not quite right
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

struct Ball
{
  int id;
  glm::vec3 pos;
  glm::vec3 vel;
  glm::vec4 color;
  float size;
};

class Viewer : public Window
{
public:
  Viewer() : Window()
  {
    setWindowSize(500, 500);
  }

  void setup()
  {
    srand(time(nullptr));
    renderer.fontSize(20);
    renderer.fontColor(glm::vec4(0.98, 0.94, 0.82, 0.8));

    loadTextures();
    loadCubemaps();
    loadShaders();
    loadMeshes();

    createPoolBalls();
    createHoles();

    for (string effect : _chaosEffects)
    {
      _chaosEffectStatus[effect] = false;
    }
  }

  void loadTextures()
  {
    for (int i = 0; i < 16; i++)
    {
      renderer.loadTexture("ball_" + to_string(i + 1), "../textures/pool-balls/ball_" + to_string(i + 1) + ".png", 0);
    }
    renderer.loadTexture("trajectoryBall", "../textures/pool-balls/ParticleBokeh.png", 0);
    renderer.loadTexture("pool-table", "../textures/pool-table/PoolTable_poolTable_BaseColor.png", 0);
    renderer.loadTexture("pool-table-normal", "../textures/pool-table/PoolTable_poolTable_Normal.png", 0);
    renderer.loadTexture("cue-stick", "../textures/cue-stick/Cue_diff.png", 0);
    renderer.loadTexture("explosion", "../textures/sprite-sheets/explosion.png", 0);
    renderer.loadTexture("eye", "../textures/eye-of-sauron/eye.jpg", 0);
    renderer.loadTexture("logo", "../textures/pool-of-surprises-logo.png", 0);
  }

  void loadCubemaps()
  {
    // renderer.loadCubemap("blue-photo-studio", "../cubemaps/blue-photo-studio", 5);
    // renderer.loadCubemap("colorful-studio", "../cubemaps/colorful-studio", 5);
    renderer.loadCubemap("shanghai-bund", "../cubemaps/shanghai-bund", 5);
    // renderer.loadCubemap("sea-cubemap", "../cubemaps/sea-cubemap", 5);
    // renderer.loadCubemap("pure-sky", "../cubemaps/pure-sky", 5);
  }

  void loadShaders()
  {
    // renderer.loadShader("phong-pixel", "../shaders/phong-pixel.vs", "../shaders/phong-pixel.fs");
    renderer.loadShader("texture", "../shaders/texture.vs", "../shaders/texture.fs");
    renderer.loadShader("cubemap", "../shaders/cubemap.vs", "../shaders/cubemap.fs");
    renderer.loadShader("billboard-animated", "../shaders/billboard-animated.vs", "../shaders/billboard-animated.fs");
    renderer.loadShader("fluid", "../shaders/fluid.vs", "../shaders/fluid.fs");
    renderer.loadShader("billboard", "../shaders/billboard.vs", "../shaders/billboard.fs");
  }

  void loadMeshes()
  {
    _poolTableMesh = PLYMesh("../models/pool-table.ply");
    _tableScaleVector = scaleVector(_poolTableMesh, "pool-table");
    _tableCenterVector = centerVector(_poolTableMesh, "pool-table");

    _cueStickMesh = PLYMesh("../models/cue-stick.ply");
    _stickScaleVector = scaleVector(_cueStickMesh, "cue-stick");
    _stickCenterVector = centerVector(_cueStickMesh, "cue-stick");

    _eyeMesh = PLYMesh("../models/eye.ply");
    _eyeScaleVector = scaleVector(_eyeMesh, "eye");
    _eyeCenterVector = centerVector(_eyeMesh, "eye");
  }

  vec3 scaleVector(PLYMesh mesh, string meshName)
  {
    vec3 minBounds = mesh.minBounds();
    vec3 maxBounds = mesh.maxBounds();
    float windowX = abs(minBounds[0]) + (maxBounds[0]);
    float windowY = abs(minBounds[1]) + (maxBounds[1]);
    float windowZ = abs(minBounds[2]) + (maxBounds[2]);
    float maxDimension = std::max(std::max(windowX, windowY), windowZ);
    float scaleFactor = _viewVolumeSide / maxDimension;
    if (meshName == "pool-table")
    {
      _tableLength = int(windowY * scaleFactor);
      _tableWidth = int(windowX * scaleFactor);
    }
    else if (meshName == "cue-stick")
    {
      _stickLength = int(windowZ * scaleFactor);
    }
    else if (meshName == "eye")
    {
      _eyeDiameter = int(windowX * scaleFactor);
    }
    return vec3(scaleFactor);
  }

  vec3 centerVector(PLYMesh mesh, string meshName)
  {
    vec3 minBounds = mesh.minBounds();
    vec3 maxBounds = mesh.maxBounds();
    float centroidX = (minBounds[0] + maxBounds[0]) / 2.0f;
    float centroidY = (minBounds[1] + maxBounds[1]) / 2.0f;
    float centroidZ = (minBounds[2] + maxBounds[2]) / 2.0f;
    return vec3(-centroidX, -centroidY, -centroidZ);
  }

  void createPoolBalls()
  {
    for (int i = 0; i < _numBalls; i++)
    {
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
      ball.size = _ballDefaultSize;
      _balls.push_back(ball);
    }
  }

  void createTrajecBalls()
  {
    for (int i = 0; i < 5; i++)
    {
      Ball trajectoryBall;
      trajectoryBall.pos = _balls[_activeBall].pos + ((1.0f / (i + 1)) * _launchVel);
      trajectoryBall.color = vec4(0.8);
      trajectoryBall.size = 5 + (4 - i);
      _trajectoryBalls.push_back(trajectoryBall);
    }
  }

  void createHoles()
  {
    for (int i = 0; i < 6; i++)
    {
      vec3 hole;
      hole.x = (i % 3) == 0 ? -(_tableLength / 2) + 50 : (i % 3) == 1 ? 0
                                                                      : (_tableLength / 2) - 50;
      hole.y = (i < 3) ? (_tableWidth / 2) - 50 : -(_tableWidth / 2) + 50;
      hole.z = 0;
      _holes.push_back(hole);
    }
  }

  void updatePoolBalls()
  {
    std::vector<Ball> newBalls;
    for (int i = 0; i < _numBalls; i++)
    {
      Ball ball = _balls[i];
      Ball newBall;
      if (ball.size == _viewVolumeSide / 10) {
        // floating up to glorb
        vec3 glorbPos = vec3(0, 0, 200);
        float glorbRadius = (_viewVolumeSide / 4) * _eyeDiameterModifier * 0.5;
        if (length(glorbPos - newBall.pos) <= glorbRadius) {
          newBall.vel = vec3(0);
          newBall.size = 0;
        }
      } else {
        newBall = holeDetection(ball);
        newBall = collisionDetection(newBall);
        newBall.pos += newBall.vel * dt();
        if (_chaosEffectStatus["Tilt-a-Table"]) 
          newBall.pos += vec3(10, 0, 0) * dt();
        newBall = boundaryDetection(newBall);
        // friction
        newBall.vel *= _chaosEffectStatus["Friction Affliction"] ? 0.8f : 0.99f;
      }
      newBalls.push_back(newBall);
    }
    _balls = newBalls;
  }

  Ball collisionDetection(Ball ball)
  {
    for (int j = 0; j < _numBalls; j++)
    {
      Ball otherBall = _balls[j];
      if (ball.id != otherBall.id && length(ball.pos - otherBall.pos) <= (_sphereDefaultRadius * ball.size + _sphereDefaultRadius * otherBall.size))
      {
        float overlap = (_sphereDefaultRadius * ball.size + _sphereDefaultRadius * otherBall.size) - length(ball.pos - otherBall.pos);
        ball.pos += normalize(ball.pos - otherBall.pos) * overlap / 2.0f;
        ball.vel = (otherBall.vel - ball.vel) / 2.0f;
        // if (length(otherBall.vel) != 0) ball.vel = otherBall.vel;
        // else ball.vel = vec3(ball.vel.x, ball.vel.x, 1.0);
        // break;
      }
    }
    return ball;
  }

  Ball boundaryDetection(Ball ball)
  {
    int ballRadius = _sphereDefaultRadius * ball.size;
    int xThresh = (_tableLength - 75) / 2;
    int ballLeft = ball.pos.x - ballRadius;
    int ballRight = ball.pos.x + ballRadius;
    if (ballLeft < -xThresh || ballRight > xThresh)
    {
      ball.pos.x += (ballLeft < -xThresh) ? -xThresh - ballLeft : 0;
      ball.pos.x += (ballRight > xThresh) ? xThresh - ballRight : 0;
      ball.vel.x = _chaosEffectStatus["Sticky Situation"] ? 0 : -ball.vel.x;
      ball.vel.y = _chaosEffectStatus["Sticky Situation"] ? 0 : ball.vel.y;
    }
    int yThresh = (_tableWidth - 75) / 2;
    int ballBottom = ball.pos.y - ballRadius;
    int ballTop = ball.pos.y + ballRadius;
    if (ball.pos.y < -yThresh || ball.pos.y > yThresh)
    {
      ball.pos.y += (ballBottom < -xThresh) ? -yThresh - ballBottom : 0;
      ball.pos.y += (ballTop > xThresh) ? yThresh - ballTop : 0;
      ball.vel.y = _chaosEffectStatus["Sticky Situation"] ? 0 : -ball.vel.y;
      ball.vel.x = _chaosEffectStatus["Sticky Situation"] ? 0 : ball.vel.x;
    }
    return ball;
  }

  Ball holeDetection(Ball ball)
  {
    for (int i = 0; i < 6; i++)
    {
      if (length(_holes[i] - ball.pos) < _viewVolumeSide / 150)
      {
        _congratsMessage = congratsMessages[ball.id];
        vec3 glorbPos = vec3(0, 0, 200);
        ball.vel = 0.5f * (glorbPos - ball.pos);
        ball.size *= 2;
        _eyeDiameterModifier += 0.02;
        _numBallsSunk += 1;
        _congratsStartTime = elapsedTime() + 1;
      }
      else if (length(_holes[i] - ball.pos) < _viewVolumeSide / 50)
      {
        ball.vel = 10.0f * (_holes[i] - ball.pos);
      }
    }
    return ball;
  }

  int launchDetection(int clickX, int clickY)
  {
    float closestDist = 99999999;
    int closestDistIdx = -1;
    cout << "click pos: " << clickX << " " << clickY << endl;
    for (int i = 0; i < _numBalls; i++)
    {
      vec4 ballEyePos = renderer.viewMatrix() * vec4(_balls[i].pos, 1.0);
      vec2 ballScreenPos = vec2(ballEyePos.x, ballEyePos.y);
      // cout << i << ") " << screenPos << " vs " << clickX << " " << clickY << " dist: " << length(screenPos - vec2(clickX, clickY)) << endl;
      if (length(ballScreenPos - vec2(clickX, clickY)) < _ballDefaultSize * 2)
      {
        cout << "threshold crossed" << endl;
        float dist = length(ballScreenPos - vec2(clickX, clickY));
        if (dist < closestDist)
        {
          closestDist = dist;
          closestDistIdx = i;
          _launching = true;
        }
      }
    }
    cout << endl;
    return closestDistIdx;
  }

  void drawPoolTable()
  {
    renderer.setUniform("MaterialColor", vec4(1));
    renderer.texture("Image", "pool-table");
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

  void drawCueStick()
  {
    if (_launching)
    {
      renderer.setUniform("MaterialColor", vec4(1));
      renderer.texture("Image", "cue-stick");
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

  void drawPoolBalls()
  {
    renderer.setUniform("PoolBall", true);
    for (int i = 0; i < _numBalls; i++)
    {
      renderer.setUniform("MaterialColor", _balls[i].color);
      renderer.texture("Image", "ball_" + to_string(i + 1));
      renderer.push();
      renderer.translate(_balls[i].pos);
      renderer.rotate(vec3(0, M_PI_2, M_PI_2));
      renderer.scale(vec3(_balls[i].size));
      renderer.sphere();
      renderer.pop();
    }
    renderer.setUniform("PoolBall", false);
  }

  void drawTrajectoryBalls()
  {
    if (_launching)
    {
      renderer.texture("Image", "trajectoryBall");
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

  void drawEye()
  {
    renderer.setUniform("MaterialColor", vec4(1));
    renderer.setUniform("EyeOfSauron", true);
    renderer.texture("Image", "eye");
    renderer.push();
    renderer.translate(vec3(0, 0, 200));
    vec3 n;
    if ((!_launching && length(_balls[_activeBall].vel) < 5) || _orbiting) {
      n = normalize(_camPos - vec3(0, 0, 200));
      float thetaZ = atan2(n.y, -n.x) + M_PI_2;
      vec3 x2 = vec3(cos(thetaZ), -sin(thetaZ), 0);
      vec3 y2 = vec3(sin(thetaZ), cos(thetaZ), 0);
      vec3 z2 = vec3(0, 0, 1);
      mat3 R_z = mat3(x2, y2, z2);
      renderer.rotate(R_z);
    } else {
      n = normalize(_balls[_activeBall].pos - vec3(0, 0, 200));
      float thetaY = atan2(n.z, n.x) + M_PI_2;
      vec3 x2 = vec3(cos(thetaY), 0, sin(thetaY));
      vec3 y2 = vec3(0, 1, 0);
      vec3 z2 = vec3(-sin(thetaY), 0, cos(thetaY));
      mat3 R_y = mat3(x2, y2, z2);
      renderer.rotate(R_y);
    }
    float thetaX = atan2(n.z, -n.y) + M_PI_2;
    vec3 x1 = vec3(1, 0, 0);
    vec3 y1 = vec3(0, cos(thetaX), -sin(thetaX));
    vec3 z1 = vec3(0, sin(thetaX), cos(thetaX));
    mat3 R_x = mat3(x1, y1, z1);
    renderer.rotate(R_x);
    renderer.rotate(vec3(0, M_PI_2, 0));
    renderer.scale(vec3(_eyeDiameterModifier));
    renderer.scale(_eyeScaleVector);
    renderer.translate(_eyeCenterVector);
    renderer.mesh(_eyeMesh);
    renderer.pop();
    renderer.setUniform("EyeOfSauron", false);
  }

  void chaos()
  {
    if (_chaosEffectStatus["Hover Havoc"])
    {
      for (int i = 0; i < _numBalls; i++)
      {
        if (_balls[i].pos.z >= 40)
        {
          _balls[i].pos.z = 50.0f + 10 * sin(elapsedTime());
        }
      }
    }

    int newEffectThresh = 500;
    int frame = int(_time * 30);
    if (frame % newEffectThresh == 0)
    {
      _chaosAnimStartTime = elapsedTime() + 1;
      _chaosAnimation = true;
      string effect = _chaosEffects[rand() % _chaosEffects.size()];
      for (auto it = _chaosEffectStatus.begin(); it != _chaosEffectStatus.end(); it++)
      {
        if (it->first == effect)
        {
          if (it->second == false)
          {
            cout << "activating chaos effect: " << it->first << endl;
            _chaosEffectStatus[it->first] = true;
            _activeChaosEffect = effect;
            if (effect == "Hover Havoc")
            {
              gravityChaos();
            }
            else if (effect == "Biggie Smalls")
            {
              sizeChaos();
            }
          }
        }
        else
        {
          if (it->second == true)
          {
            _chaosEffectStatus[it->first] = false;
            if (it->first == "Hover Havoc")
            {
              resetGravity();
            }
            else if (it->first == "Biggie Smalls")
            {
              resetSize();
            }
          }
        }
      }
    }
    // cout << "|" << elapsedTime() - _chaosAnimStartTime << "|";
    if (elapsedTime() - _chaosAnimStartTime > 3)
    {
      _chaosAnimation = false;
    }
  }

  void gravityChaos()
  {
    for (int i = 0; i < _numBalls; i++)
    {
      if (rand() % 4 == 0)
      {
        _balls[i].pos.z = 50.0f + 10 * sin(elapsedTime());
      }
    }
  }

  void resetGravity()
  {
    for (int i = 0; i < _numBalls; i++)
    {
      _balls[i].pos.z = 0;
    }
  }

  void sizeChaos()
  {
    for (int i = 0; i < _numBalls; i++)
    {
      if (rand() % 4 == 0)
      {
        float prevSize = _balls[i].size;
        (rand() % 2)? _balls[i].size *= 3 : _balls[i].size /= 3; 
        _balls[i].pos.z += _sphereDefaultRadius * (_balls[i].size - prevSize);
      }
    }
  }

  void resetSize()
  {
    for (int i = 0; i < _numBalls; i++)
    {
      float prevSize = _balls[i].size;
      _balls[i].size = _ballDefaultSize;
      _balls[i].pos.z += _sphereDefaultRadius * (_balls[i].size - prevSize);
    }
  }

  void drawChaosTransition() {
    string message = _activeChaosEffect + " activated!";
    float x = 250 - renderer.textWidth(message) * 0.5f;
    float y = 450 + renderer.textHeight() * 0.25f;
    renderer.text(message, x, y);
    renderer.setDepthTest(false);
    renderer.blendMode(agl::ADD);
    renderer.beginShader("billboard-animated");
    renderer.texture("Image", "explosion");
    int numRows = 8;
    int numCols = 16;
    int frame = int(_time * 30) % (numRows * numCols);
    renderer.setUniform("Frame", frame);
    renderer.setUniform("Rows", numRows);
    renderer.setUniform("Cols", numCols);
    renderer.setUniform("TopToBottom", false);
    renderer.sprite(vec3(0, 250, 100), vec4(1.0f), 250.0);
    renderer.endShader();
    renderer.setDepthTest(true);
    renderer.blendMode(agl::DEFAULT);
  }

  void drawLogo() {
    renderer.push();
    renderer.beginShader("texture");
    renderer.texture("Image", "logo");
    // renderer.sprite(vec3(-100, -100, 200), vec4(1.0f), 150.0);
    renderer.translate(vec3(-200, 0, 125));
    vec3 n = normalize(_camPos - vec3(-200, 0, 125));
    float thetaX = atan2(n.z, -n.y) - M_PI_2;
    vec3 x1 = vec3(1, 0, 0);
    vec3 y1 = vec3(0, cos(thetaX), -sin(thetaX));
    vec3 z1 = vec3(0, sin(thetaX), cos(thetaX));
    mat3 R_x = mat3(x1, y1, z1);
    renderer.rotate(R_x);
    // renderer.rotate(vec3(0, M_PI_2, 0));
    renderer.scale(vec3(_viewVolumeSide / 4));
    renderer.setDepthTest(false);
    renderer.blendMode(agl::BLEND);
    renderer.setUniform("Color", vec4(0, 0, 0, 0.5));
    renderer.quad();
    renderer.blendMode(agl::ADD);
    renderer.setUniform("Color", vec4(1));
    renderer.quad();
    renderer.setDepthTest(true);
    renderer.blendMode(agl::DEFAULT);
    renderer.endShader();
    renderer.pop();
  }

  void drawFluid() {
    renderer.setDepthTest(false);
    renderer.blendMode(agl::ADD);
    renderer.beginShader("fluid");
    renderer.setUniform("Resolution", vec2(width(), height()));
    renderer.setUniform("Time", elapsedTime());
    vec4 ballEyePos = renderer.viewMatrix() * vec4(_balls[_activeBall].pos, 1.0);
    renderer.setUniform("BallPos", vec3(ballEyePos.x + 250, ballEyePos.y + 250, ballEyePos.z));
    renderer.push();
    renderer.translate(vec3(0, 0, -7.5));
    renderer.scale(vec3(_tableLength - 50, _tableWidth - 50, 1.0));
    renderer.rotate(vec3(0, 0, M_PI_2));
    renderer.translate(vec3(-0.5, -0.5, 0.0));
    renderer.quad();
    renderer.pop();
    renderer.endShader();
    renderer.setDepthTest(true);
    renderer.blendMode(agl::DEFAULT);
  }

  void drawSkybox(string cubemapName) {
    renderer.push();
    renderer.rotate(vec3(M_PI_2, 0, M_PI));
    renderer.setUniform("ModelMatrix", renderer.modelMatrix());
    renderer.cubemap("Cubemap", cubemapName);
    renderer.setUniform("Skybox", true);
    renderer.skybox(_viewVolumeSide * 5);
    renderer.setUniform("Skybox", false);
    renderer.pop();
  }

  void setupReflections()
  {
    renderer.setUniform("ViewMatrix", renderer.viewMatrix());
    renderer.setUniform("ProjMatrix", renderer.projectionMatrix());
    renderer.setUniform("LightPos", _lightPos);
    renderer.setUniform("LightColor", _lightColor.x, _lightColor.y, _lightColor.z);
    renderer.setUniform("EyePos", _camPos);
    float ka = 0.1, kd = 0.7, ks = 0.6;
    float phongExp = 50.0;
    renderer.setUniform("Ka", ka);
    renderer.setUniform("Kd", kd);
    renderer.setUniform("Ks", ks);
    renderer.setUniform("PhongExp", phongExp);
  }

  vec3 updateOrbitPos(float offset)
  {
    float x = _radius * sin(_azimuth + offset) * cos(_elevation + offset);
    float y = _radius * sin(_elevation + offset);
    float z = _radius * cos(_azimuth + offset) * cos(_elevation + offset);
    return vec3(x, y, z);
  }

  void mouseMotion(int x, int y, int dx, int dy)
  {
    if (_leftClick)
    {
      if (_launching)
      {
        _launchVel += vec3(-dx, dy, 0);
        for (int i = 0; i < _trajectoryBalls.size(); i++)
        {
          _trajectoryBalls[i].pos = _balls[_activeBall].pos + ((1.0f / (i + 1)) * _launchVel);
        }
      }
      else if (!_orbiting)
      {
        int clickX = x - (width() / 2);
        int clickY = -(y - (height() / 2));
        int closestDistIdx = launchDetection(clickX, clickY);
        if (_launching)
        {
          _activeBall = closestDistIdx;
          _balls[_activeBall].vel = vec3(0);
          _balls[_activeBall].color /= 2.0f;
          _launchVel = vec3(-dx, dy, 0);
          createTrajecBalls();
        }
        else
        {
          _orbiting = true;
        }
      }
      if (_orbiting)
      {
        pan(dx, dy);
      }
    }
  }

  void pan(float dx, float dy)
  {
    float ONE_DEG = 0.017;
    _elevation += dy * (M_PI / 180);
    if (_elevation > (M_PI_2)-ONE_DEG)
    {
      _elevation = (M_PI_2)-ONE_DEG;
    }
    else if (_elevation < -((M_PI_2)-ONE_DEG))
    {
      _elevation = -((M_PI_2)-ONE_DEG);
    }
    _azimuth -= dx * (M_PI / 180);
    if (_azimuth > 2 * M_PI)
    {
      _azimuth = 0;
    }
    else if (_azimuth < 0)
    {
      _azimuth = 2 * M_PI;
    }
  }

  void mouseDown(int button, int mods)
  {
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
      _leftClick = true;
    }
  }

  void mouseUp(int button, int mods)
  {
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
      _leftClick = false;
      if (_launching)
      {
        if (_chaosEffectStatus["Get Gaslit"])
        {
          _balls[_activeBall].vel = vec3(_launchVel.x, -_launchVel.y, _launchVel.z);
        }
        else
        {
          _balls[_activeBall].vel = _launchVel;
        }
        _balls[_activeBall].color *= 2.0f;
        _launching = false;
        _launchVel = vec3(0);
        _trajectoryBalls.clear();
      }
      else if (_orbiting)
      {
        _orbiting = false;
      }
    }
  }

  void scroll(float dx, float dy)
  {
    // scroll up or down to zoom in or out
    _radius -= 10 * dy;
    if (_radius < 1)
    {
      _radius = 1;
    }
    // scroll sideways to slowly pan sideways
    _azimuth -= 10 * dx * (M_PI / 180);
    if (_azimuth > 2 * M_PI)
    {
      _azimuth = 0;
    }
    else if (_azimuth < 0)
    {
      _azimuth = 2 * M_PI;
    }
  }

  void keyUp(int key, int mods)
  {
    if (key == GLFW_KEY_R)
    {
      for (int i = 0; i < _numBalls; i++)
      {
        _balls[i].vel = vec3(0);
      }
    }
  }

  void draw()
  {
    float aspect = ((float)width()) / height();
    renderer.perspective(glm::radians(60.0f), aspect, 0.1f, _viewVolumeSide * 10);

    _camPos = updateOrbitPos(0);
    renderer.lookAt(_camPos, _lookPos, _up);

    _lightPos = updateOrbitPos(M_PI_4);

    _time += dt();

    if (elapsedTime() - _congratsStartTime < 5)
    {
      float x = 250 - renderer.textWidth(_congratsMessage) * 0.5f;
      float y = 475 + renderer.textHeight() * 0.25f;
      renderer.text(_congratsMessage, x, y);
    }

    string message = to_string(_numBallsSunk) + " balls sunk";
    float x = 450 - renderer.textWidth(message);
    float y = 50 + renderer.textHeight() * 0.25f;
    renderer.text(message, x, y);

    renderer.beginShader("cubemap");

    // setting phong shading uniforms
    setupReflections();
    drawSkybox("shanghai-bund");
    renderer.push();
    // renderer.rotate(vec3(-M_PI_2, 0, -M_PI));
    // renderer.rotate(vec3(-M_PI_2, 0, 0));
    if (_chaosAnimation) drawChaosTransition();
    drawPoolTable();
    // drawFluid();
    drawCueStick();
    updatePoolBalls();
    drawPoolBalls();
    drawTrajectoryBalls();
    drawLogo();
    drawEye();
    // glBindTexture(GL_TEXTURE_2D, renderer.fontTexId());
    // renderer.texture("Image", "fontTexture");
    chaos();
    renderer.pop();
    
    renderer.endShader();

  }

protected:
  int _viewVolumeSide = 500;
  int _radius = 500;
  float _sphereDefaultRadius = 0.5;
  float _ballDefaultSize = _viewVolumeSide / 20;

  vec3 _camPos = vec3(0, 0, -_viewVolumeSide);
  vec3 _lookPos = vec3(0, 0, 0);
  vec3 _up = vec3(0, 1, 0);
  float _azimuth = 0;
  float _elevation = 0;
  float _orbiting = false;
  vec3 _lightPos = vec3(_viewVolumeSide, _viewVolumeSide, _viewVolumeSide);
  vec3 _lightColor = vec3(1.0, 1.0, 1.0);

  std::vector<Ball> _balls;
  int _numBalls = 16;
  int _numBallsSunk = 0;

  bool _leftClick = false;
  bool _launching = false;
  std::vector<Ball> _trajectoryBalls;
  // std::vector<Ball> _trailingBalls;
  std::vector<vec3> _holes;
  int _activeBall = 0;
  vec3 _launchVel = vec3(0);

  PLYMesh _poolTableMesh;
  vec3 _tableScaleVector;
  vec3 _tableCenterVector;
  int _tableLength = 0;
  int _tableWidth = 0;
  PLYMesh _cueStickMesh;
  vec3 _stickScaleVector;
  vec3 _stickCenterVector;
  int _stickLength;
  PLYMesh _eyeMesh;
  vec3 _eyeScaleVector;
  vec3 _eyeCenterVector;
  int _eyeDiameter;
  float _eyeDiameterModifier = 0.25;


  vector<string> _chaosEffects = {"Plain Jane", "Sticky Situation", "Hover Havoc", "Biggie Smalls", "Friction Affliction", "Tilt-a-Table", "Get Gaslit"};
  map<string, bool> _chaosEffectStatus;
  string _activeChaosEffect = "Plain Jane";
  float _time = 0.0f;
  bool _chaosAnimation = false;
  float _chaosAnimStartTime = 9999;

  float _congratsStartTime = 9999;
  string _congratsMessage;

  vector<string> congratsMessages = {
    "Hole in one! Oops, wrong game..."
    "I would give you a round of applause if I could!",
    "Glorb is impressed, but still hungry!",
    "Being that good can't be legal; I have my an eye on you!",
    "I haven't seen that kind of precision since the time I played Jenga with a neurosurgeon!",
    "You're making those balls disappear faster than I can blink!",
    "Is this your full-time job or what?",
    "Don't let your head get too big, there's only room for one giant ego here!",
    "My God, someone get this guy in the olympics!",
    "That shot was so clean, I could eat off of it!",
    "I could do this too you know, if I had body parts...",
    "That shot was so brutal it made the other balls cry!",
    "Now you see it, now you don't!",
    "You're like the Michael Jordan of Tabletop Trick Shots!",
    "You've got more talent in your little finger than I have in my entire iris!",
    "That was too perfect, are you sure you're not cheating?"
  };
};

int main(int argc, char **argv)
{
  Viewer viewer;
  viewer.run();
  return 0;
}
