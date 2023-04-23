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
  vec3 pos;
  vec3 vel;
  vec3 rot;
  vec4 color;
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
    renderer.fontColor(vec4(0.98, 0.94, 0.82, 1));

    loadTextures();
    loadCubemaps();
    loadShaders();
    loadMeshes();

    createPoolBalls();
    createHoles();

    for (string effect : _chaosEffects)
    {
      _chaosStatus[effect] = false;
    }
  }

  void loadTextures()
  {
    for (int i = 0; i < std::min(_numBalls, 16); i++)
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
    renderer.loadShader("eye", "../shaders/eye-of-sauron.vs", "../shaders/eye-of-sauron.fs");
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

  void startGame() {
    float timer = elapsedTime();
    renderer.fontSize(width() / 13);
    string message; float x; float y;
    if (timer < 22) {
      if (timer < 9) {
        message = "Hey, you there!";
      } else if (timer < 13.5) {
        message = "Welcome to Omicron Persei 8, stranger. I'm Glorb.";
      } else if (timer < 17.5) {
        message = "How about a game of pool while you find your bearings?";
      } else if (timer < 22) {
        message = "I warn you, it might not be quite like you expect...";
      }
      x = width() / 2 - renderer.textWidth(message) * 0.5f;
      y = height() * 0.9 + renderer.textHeight() * 0.25f;
      renderer.text(message, x, y);
      _glorbPos.y += 0.5 * sin(10 * timer);
    } else {
      _showLogo = true;
      _elevation -= (width() / 500) * (M_PI / 180);
    }
    if (_elevation < M_PI_4 * 0.75) _startGame = false;
  }

  void createPoolBalls()
  {
    float k = (float) 3 / 1;
    float a = (_tableWidth - 100) / 2;
    for (int i = 0; i < _numBalls; i++)
    {
      Ball ball;
      ball.id = i;
      float theta = ((float) i / _numBalls) * M_PI;
      float r = a * cos(k * theta);
      ball.pos.x = r * cos(theta);
      ball.pos.y = r * sin(theta);
      // ball.pos.x = pow(-1, rand()) * (rand() % ((_tableLength - 100) / 2));
      // ball.pos.y = pow(-1, rand()) * (rand() % ((_tableWidth - 100) / 2));
      // ball.pos.x = (float(i + 1) / _numBalls) * (_tableLength - 100);
      // ball.pos.x -= _tableLength / 2 - 50;
      // ball.pos.y = (float(i + 1) / _numBalls) * (_tableWidth - 100);
      // ball.pos.y -= _tableWidth / 2 - 50;
      ball.vel = vec3(0);
      ball.rot = vec3(0);
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
    for (int i = 0; i < _numBalls; i++)
    {
      Ball ball = _balls[i];
      if (ball.size == 2 * _ballDefaultSize) {
        // floating up to glorb
        // float glorbRadius = (_viewVolumeSide / 4) * _eyeDiameterModifier * 0.5;
        float glorbRadius = _eyeScaleVector.x * _eyeDiameterModifier * 0.5;
        if (length(_glorbPos - ball.pos) <= glorbRadius) {
          ball.pos = vec3(0, 1000, 200);
          ball.vel = vec3(0);
          ball.size = 0;
          // _balls.erase(_balls.begin() + i);
          // _numBalls -= 1;
          _eyeDiameterModifier += 0.02;
          _eyeColor = vec4(0, 1, 0, 0.5);
        }
      } else {
        // friction
        ball.vel *= _chaosStatus["Friction Affliction"] ? 0.8f : 0.99f;
        if (_chaosStatus["Tilt-a-Table"]) ball.vel += vec3(1, 0, 0);
        bool sinking = holeDetection(ball);
        if (!sinking) {
          bool collided = false;
          for (int j = i + 1; j < _numBalls && !collided; j++)
          {
            collided = collisionDetection(i, j);
          }
          boundaryDetection(ball);
        }
      }
      vec3 dist = ball.vel * dt();
      ball.pos += dist;
      ball.rot += vec3(-dist.y, dist.x, 0) * float((M_PI * 2) / (M_PI * ball.size * _sphereRadius * 2));
      _balls[i] = ball;
    }
  }

  bool collisionDetection(int i, int j)
  {
    Ball ball1 = _balls[i];
    Ball ball2 = _balls[j];
    float overlap = _sphereRadius * (ball1.size + ball2.size) - length(ball1.pos - ball2.pos);
    if (overlap > 0.1)
    {
      vec3 normal = normalize(ball1.pos - ball2.pos);
      ball1.pos += normal * overlap / 2.0f;
      ball2.pos -= normal * overlap / 2.0f;
      // cout << "collision " << elapsedTime() << endl;
      // ball1.vel = 2 * dot(ball1.vel, normal) * normal - ball1.vel;
      // vec3 relativeVel = (ball2.vel - ball1.vel) / 2.0f;
      // vec3 newRelativeVel = reflect(relativeVel, normal);
      // ball1.vel = newRelativeVel;
      // vec3 beginningSumOfVels = ball1.vel + ball2.vel;
      if (length(ball1.vel) >= 25 || length(ball2.vel) >= 25) {
        cout << ball1.vel << " vs " << ball2.vel << endl;
      }
      if (length(ball1.vel) >= 25 && length(ball2.vel) >= 25) {
        ball1.vel = reflect(ball1.vel, normal);
        ball2.vel = reflect(ball2.vel, -normal);
        // cout << "both reflect" << endl;
      } else if (length(ball1.vel) < 25 && length(ball2.vel) >= 25) {
        ball1.vel = ball2.vel / 2.0f;
        ball2.vel = reflect(ball2.vel, -normal) / 2.0f;
        // cout << "ball2 reflects" << endl;
      } else if (length(ball2.vel) < 25 && length(ball1.vel) >= 25) {
        // cout << ball2.vel;
        ball2.vel = ball1.vel / 2.0f;
        // cout << " vs " << ball2.vel;
        ball1.vel = reflect(ball1.vel, normal) / 2.0f;
        // cout << " vs " << ball2.vel << endl;
        // cout << "ball reflects" << endl;
      }
      if (length(ball1.vel) >= 25 || length(ball2.vel) >= 25) {
        cout << ball1.vel << " vs " << ball2.vel << endl << endl;
      }
      // ball1.vel = (ball2.vel - ball1.vel) / 2.0f;
      // ball2.vel = (ball1.vel - ball2.vel) / 2.0f;
      _balls[i] = ball1;
      _balls[j] = ball2;
      return true;
      // vec3 endingSumOfVels = ball1.vel + ball2.vel;
      // cout << beginningSumOfVels << " vs " << endingSumOfVels << endl;
      // newRelativeVel = reflect(-relativeVel, -normal);
      // ball2.vel = newRelativeVel;
      // ball1.vel = (ball2.vel - ball1.vel) / 2.0f;
      // if (length(ball2.vel) != 0) ball1.vel = ball2.vel;
      // else ball1.vel = vec3(ball1.vel.x, ball1.vel.x, 1.0);
    } else {
      return false;
    }
  }

  void boundaryDetection(Ball& ball)
  {
    float ballRadius = _sphereRadius * ball.size;
    float xThresh = (_tableLength - 75) / 2.0f;
    float ballLeft = ball.pos.x - ballRadius;
    float ballRight = ball.pos.x + ballRadius;
    if (ballLeft < -xThresh || ballRight > xThresh)
    {
      ball.pos.x += (ballLeft < -xThresh) ? -xThresh - ballLeft : 0;
      ball.pos.x += (ballRight > xThresh) ? xThresh - ballRight : 0;
      ball.vel.x = _chaosStatus["Sticky Situation"] ? 0 : -ball.vel.x;
      ball.vel.y = _chaosStatus["Sticky Situation"] ? 0 : ball.vel.y;
    }
    float yThresh = (_tableWidth - 75) / 2.0f;
    float ballBottom = ball.pos.y - ballRadius;
    float ballTop = ball.pos.y + ballRadius;
    if (ball.pos.y < -yThresh || ball.pos.y > yThresh)
    {
      ball.pos.y += (ballBottom < -xThresh) ? -yThresh - ballBottom : 0;
      ball.pos.y += (ballTop > xThresh) ? yThresh - ballTop : 0;
      ball.vel.y = _chaosStatus["Sticky Situation"] ? 0 : -ball.vel.y;
      ball.vel.x = _chaosStatus["Sticky Situation"] ? 0 : ball.vel.x;
    }
  }

  bool holeDetection(Ball& ball)
  {
    for (int i = 0; i < 6; i++)
    {
      if (length(_holes[i] - ball.pos) < _viewVolumeSide / 150)
      {
        _congratsMessage = congratsMessages[ball.id];
        _congratsStartTime = elapsedTime() + 1;
        _numBallsSunk += 1;
        if (_numBallsSunk == 16) _endGame = true;
        ball.vel = 0.5f * (_glorbPos - ball.pos);
        ball.size *= 2;
        return true;
      }
      else if (length(_holes[i] - ball.pos) < _viewVolumeSide / 50)
      {
        ball.vel = 10.0f * (_holes[i] - ball.pos);
        return true;
      }
    }
    return false;
  }

  void endGame() {
    _enableChaos = false;
    _showLogo = false;
    float timer = elapsedTime() - _congratsStartTime;
    string message; float x; float y;
    if (timer > 5) {
      _showLogo = false;
      if (timer < 9) {
        renderer.fontSize(width() / 13);
        message = "\"I don't feel so good...\"";
        x = width() / 2 - renderer.textWidth(message) * 0.5f;
        y = height() * 0.9 + renderer.textHeight() * 0.25f;
        renderer.text(message, x, y);
      } else if (timer < 13) {
        _eyeDiameterModifier += (width() / 500) * 0.008;
      } else {
        renderer.fontSize(width() / 3);
        message = "FIN";
        x = width() / 2 - renderer.textWidth(message) * 0.5f;
        y = height() / 2 + renderer.textHeight() * 0.25f;
        renderer.text(message, x, y);
      }
    }
  }

  int launchDetection(int clickX, int clickY)
  {
    float closestDist = 99999999;
    int closestDistIdx = -1;
    vec2 clickPos = vec2(clickX, clickY);
    for (int i = 0; i < _numBalls; i++)
    {
      vec2 ballPos = worldToScreen(_balls[i].pos, false);
      float dist = length(ballPos - clickPos);
      if (dist < _ballDefaultSize)
      {
        if (dist < closestDist)
        {
          closestDist = dist;
          closestDistIdx = i;
          _launching = true;
        }
      }
    }
    return closestDistIdx;
  }

  vec2 worldToScreen(vec3 worldPos, bool flipY) {
    vec4 eyePos = renderer.viewMatrix() * vec4(worldPos, 1.0);
    vec4 projPos = renderer.projectionMatrix() * eyePos;
    projPos /= projPos.w;
    float screenX = (projPos.x + 1.0f) * width() / 2;
    float screenY = (projPos.y + 1.0f) * height() / 2;
    if (flipY) screenY = (1.0f - projPos.y) * height() / 2;
    return vec2(screenX, screenY);
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
      renderer.translate(-_launchVel * 0.2f);
      renderer.translate(-_launchVel * (0.5f * _stickLength / length(_launchVel)));
      renderer.translate(_balls[_activeBall].pos);
      renderer.rotate(vec3(0, 0, atan2(_launchVel.y, _launchVel.x) - M_PI_2));
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
      renderer.rotate(_balls[i].rot);
      // renderer.rotate(vec3(0, M_PI_2, M_PI_2));
      renderer.rotate(vec3(0, 0, 0));
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
    renderer.setUniform("MaterialColor", _eyeColor);
    renderer.setUniform("EyeOfSauron", true);
    renderer.texture("Image", "eye");
    renderer.push();
    renderer.translate(_glorbPos);
    vec3 lookPos;
    // face camera or active ball
    if (_startGame || (!_launching && length(_balls[_activeBall].vel) < 5) || _orbiting || _endGame) {
      lookPos = vec3(_camPos.x, -_camPos.z, _camPos.y);
    } else {
      lookPos = _balls[_activeBall].pos;
    }
    vec3 z = normalize(lookPos - _glorbPos);
    vec3 x = normalize(cross(_up, z));
    vec3 y = normalize(cross(z, x));
    mat3 R = mat3(x, y, z);
    renderer.rotate(R);
    renderer.rotate(vec3(0, -M_PI_2, 0));
    renderer.scale(vec3(_eyeDiameterModifier));
    renderer.scale(_eyeScaleVector);
    renderer.translate(_eyeCenterVector);
    renderer.mesh(_eyeMesh);
    renderer.pop();
    renderer.setUniform("EyeOfSauron", false);
  }

  void chaos()
  {
    if (_chaosStatus["Hover Havoc"])
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
      _chaosAnimStart = elapsedTime() + 1;
      _chaosAnimation = true;
      string effect = _chaosEffects[rand() % _chaosEffects.size()];
      for (auto it = _chaosStatus.begin(); it != _chaosStatus.end(); it++)
      {
        if (it->first == effect)
        {
          if (it->second == false)
          {
            _chaosStatus[it->first] = true;
            _chaosEffect = effect;
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
            _chaosStatus[it->first] = false;
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
    if (elapsedTime() - _chaosAnimStart > 2)
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
        _balls[i].pos.z += _sphereRadius * (_balls[i].size - prevSize);
      }
    }
  }

  void resetSize()
  {
    for (int i = 0; i < _numBalls; i++)
    {
      float prevSize = _balls[i].size;
      _balls[i].size = _ballDefaultSize;
      _balls[i].pos.z += _sphereRadius * (_balls[i].size - prevSize);
    }
  }

  void drawChaosTransition() {
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
    // renderer.sprite(vec3(0, 250, 100), vec4(1.0f), 250.0);
    renderer.sprite(vec3(0, -200, -20), vec4(1.0f), 50.0);
    renderer.endShader();
    renderer.setDepthTest(true);
    renderer.blendMode(agl::DEFAULT);
  }

  void drawLogo() {
    renderer.push();
    renderer.beginShader("texture");
    renderer.texture("Image", "logo");
    // renderer.sprite(vec3(-100, -100, 200), vec4(1.0f), 150.0);
    float xPos = _viewVolumeSide / 3;
    float yPos = -_viewVolumeSide / 6.7;
    float zPos = _viewVolumeSide / 4;
    renderer.translate(vec3(xPos, yPos, zPos));
    // vec3 n = normalize(_camPos - vec3(-250, 0, 75));
    // float thetaX = clamp(atan2(n.z, -n.y) - M_PI_2, -M_PI_2, 0.0);
    // vec3 x1 = vec3(1, 0, 0);
    // vec3 y1 = vec3(0, cos(thetaX), -sin(thetaX));
    // vec3 z1 = vec3(0, sin(thetaX), cos(thetaX));
    // mat3 R_x = mat3(x1, y1, z1);
    // renderer.rotate(R_x);

    vec3 logoPos = vec3(xPos, yPos, zPos);
    // vec3 logoPos = vec3(0);
    vec3 lookPos = vec3(_camPos.x, -_camPos.z, _camPos.y);
    vec3 z = normalize(lookPos - logoPos);
    vec3 x = normalize(cross(vec3(0, 0, 1), z));
    vec3 y = normalize(cross(z, x));
    mat3 R = mat3(x, y, z);
    renderer.rotate(R);

    // renderer.rotate(vec3(0, M_PI_2, 0));
    renderer.scale(vec3(_viewVolumeSide / 3.5));
    renderer.translate(vec3(-0.5, -0.5, 0));
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
    vec2 ballPos = worldToScreen(_balls[_activeBall].pos, true);
    renderer.setUniform("BallPos", _balls[_activeBall].pos);
    renderer.push();
    renderer.translate(vec3(0, 0, -20));
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
    // renderer.rotate(vec3(M_PI_2, 0, 0));
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
        _launchVel += vec3(dx, -dy, 0);
        for (int i = 0; i < _trajectoryBalls.size(); i++)
        {
          _trajectoryBalls[i].pos = _balls[_activeBall].pos + ((1.0f / (i + 1)) * _launchVel);
        }
      }
      else if (!_orbiting)
      {
        int closestDistIdx = launchDetection(x, y);
        if (_launching)
        {
          _activeBall = closestDistIdx;
          _balls[_activeBall].vel = vec3(0);
          _balls[_activeBall].color /= 2.0f;
          _launchVel = vec3(dx, -dy, 0);
          createTrajecBalls();
        }
        else
        {
          _orbiting = true;
        }
      }
      if (_orbiting)
      {
        orbit(dx, dy);
      }
    }
  }

  void orbit(float dx, float dy)
  {
    float ONE_DEG = 0.017;
    _elevation += dy * (M_PI / 180);
    if (_elevation > M_PI_2 - ONE_DEG)
    {
      _elevation = M_PI_2 - ONE_DEG;
    }
    else if (_elevation < -(M_PI_2 - ONE_DEG))
    {
      _elevation = -(M_PI_2 - ONE_DEG);
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
        if (_chaosStatus["Get Gaslit"])
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
    } else if (key == GLFW_KEY_E) {
      _congratsStartTime = elapsedTime() + 1;
      _endGame = true;
    } else if (key == GLFW_KEY_S) {
      _startGame = false;
    }
  }

  void draw()
  {
    float aspect = width() / height();
    renderer.perspective(glm::radians(60.0f), aspect, 0.1f, _viewVolumeSide * _skyBoxSize);
    // renderer.ortho(0, width(), 0, height(), -_radius, _radius);
    // cout << renderer.projectionMatrix() << endl << endl;

    _camPos = updateOrbitPos(0);
    renderer.lookAt(_camPos, _lookPos, _up);

    _lightPos = updateOrbitPos(M_PI_4);

    _time += dt();

    if (_startGame) startGame();
    else _glorbPos.y = 0;

    if (elapsedTime() - _congratsStartTime < 5)
    {
      renderer.fontSize(width() / 20);
      float x = width() / 2 - renderer.textWidth(_congratsMessage) * 0.5f;
      float y = height() * 0.94 + renderer.textHeight() * 0.25f;
      renderer.text(_congratsMessage, x, y);
      _glorbPos.z += 0.5 * sin(10 * _time);
    } else {
      _glorbPos.z = 200;
    }
    if (elapsedTime() - _congratsStartTime > 1)
    {
      _eyeColor = vec4(1);
    }

    if (!_startGame && !_endGame) {
      if (_chaosEffect == "Plain Jane") {
        renderer.fontColor(vec4(0, 1, 0, 1));
      } else {
        renderer.fontColor(vec4(1, 0, 0, 1));
      }
      renderer.fontSize(width() / 15);
      string message = "MODE: " + _chaosEffect;
      float x = width() / 2 - renderer.textWidth(message) * 0.5f;
      float y = height() * 0.85 + renderer.textHeight() * 0.25f;
      renderer.text(message, x, y);
      renderer.fontColor(vec4(0.98, 0.94, 0.82, 1));

      message = "BALLS DEVOURED: " + to_string(_numBallsSunk);
      renderer.fontSize(width() / 20);
      x = width() * 0.97 - renderer.textWidth(message);
      y = height() / 10 + renderer.textHeight() * 0.25f;
      renderer.text(message, x, y);
    }

    renderer.beginShader("cubemap");

    // setting phong shading uniforms
    setupReflections();
    drawSkybox("shanghai-bund");
    renderer.push();
    renderer.rotate(vec3(-M_PI_2, 0, 0));
    drawPoolTable();
    drawFluid();
    drawCueStick();
    updatePoolBalls();
    drawPoolBalls();
    drawTrajectoryBalls();
    if (_showLogo) drawLogo();
    drawEye();
    if (_enableChaos) chaos();
    if (_chaosAnimation) drawChaosTransition();
    if (_endGame) endGame();
    renderer.pop();
    
    renderer.endShader();
    
    if (_time - int(_time) < 0.01)
      cout << int(1/dt()) << " FPS" << endl;
  }

protected:
  int _viewVolumeSide = 500;
  int _radius = 500;
  float _sphereRadius = 0.5;
  float _ballDefaultSize = _viewVolumeSide / 20;
  int _skyBoxSize = 10;

  bool _startGame = true;
  bool _showLogo = false;

  vec3 _camPos;
  vec3 _lookPos = vec3(0, 0, 0);
  vec3 _up = vec3(0, 1, 0);
  float _azimuth = M_PI;
  float _elevation = M_PI_2 - 0.017;
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
  vec4 _eyeColor = vec4(1);
  vec3 _glorbPos = vec3(0, 0, 200);


  vector<string> _chaosEffects = {"Plain Jane", "Sticky Situation", "Hover Havoc", "Biggie Smalls", "Friction Affliction", "Tilt-a-Table", "Get Gaslit"};
  map<string, bool> _chaosStatus;
  string _chaosEffect = "Plain Jane";
  float _time = 0.0f;
  bool _enableChaos = false;
  bool _chaosAnimation = false;
  float _chaosAnimStart = 9999;

  float _congratsStartTime = -9999;
  string _congratsMessage;

  vector<string> congratsMessages = {
    "\"Hole in one! Oops, wrong game...\"",
    "\"I'd give you a round of applause if I could!\"",
    "\"Glorb is impressed, but still hungry!\"",
    "\"Being that good can't be legal; I have my eye on you!\"",
    "\"Your precision is giving me Jenga-with-a-neurosurgeon flashbacks!\"",
    "\"You're making those balls disappear faster than I can blink!\"",
    "\"Is this your full-time job or what?\"",
    "\"Don't let your head get too big, only room for one giant ego!\"",
    "\"My God, someone get this guy in the olympics!\"",
    "\"That shot was so clean, I could eat off of it!\"",
    "\"I could do this too you know, if I had body parts...\"",
    "\"That shot was so devastating it made the other balls cry!\"",
    "\"Now you see it, now you don't!\"",
    "\"You're like the Michael Jordan of Tabletop Trick Shots!\"",
    "\"There's more talent in your little finger than in my entire iris!\"",
    "\"That was too perfect, are you sure you're not cheating?\""
  };

  bool _endGame = false;
};

int main(int argc, char **argv)
{
  Viewer viewer;
  viewer.run();
  return 0;
}
