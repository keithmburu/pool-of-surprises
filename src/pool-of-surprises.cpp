/**
 * @file pool-of-surprises.cpp
 * @author 
 * @date 2023-04-18
 * @brief Implements an alternate version of pool
 */

#include "pool-of-surprises.h"
#include "agl/window.h"
#include "plymesh.h"
#include "unistd.h"
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <ctime>

using namespace std;
using namespace glm;
using namespace agl;

void Game::setup()
{
  srand(time(nullptr));
  renderer.fontColor(vec4(0.98, 0.94, 0.82, 1));

  loadTextures();
  loadCubemaps();
  loadShaders();
  loadMeshes();

  createPoolBalls();
  createPockets();

  for (string effect : _chaosEffects)
  {
    _chaosStatus[effect] = false;
  }

  vec4 x = vec4(1, 0, 0, 0);
  vec4 y = vec4(0, cos(-M_PI_2), -sin(-M_PI_2), 0);
  vec4 z = vec4(0, sin(-M_PI_2), cos(-M_PI_2), 0);
  vec4 w = vec4(0, 0, 0, 1);
  _sceneRotMat = mat4(x, y, z, w);
}

void Game::loadTextures()
{
  for (int i = 0; i < std::min(_numBalls, 16); i++)
  {
    renderer.loadTexture("ball_" + to_string(i + 1), "../textures/pool-balls/ball_" + to_string(i + 1) + ".png", 0);
  }
  renderer.loadTexture("trajectoryDot", "../textures/pool-balls/ParticleBokeh.png", 0);
  renderer.loadTexture("pool-table", "../textures/pool-table/PoolTable_poolTable_BaseColor.png", 0);
  renderer.loadTexture("cue-stick", "../textures/cue-stick/Cue_diff.png", 0);
  renderer.loadTexture("fireball", "../textures/sprite-sheets/fireball.png", 0);
  renderer.loadTexture("eye", "../textures/eye-of-sauron/eye.jpg", 0);
  renderer.loadTexture("logo", "../textures/pool-of-surprises-logo.png", 0);
}

void Game::loadCubemaps()
{
  // renderer.loadCubemap("blue-photo-studio", "../cubemaps/blue-photo-studio", 5);
  // renderer.loadCubemap("colorful-studio", "../cubemaps/colorful-studio", 5);
  renderer.loadCubemap("shanghai-bund", "../cubemaps/shanghai-bund", 5);
  // renderer.loadCubemap("sea-cubemap", "../cubemaps/sea-cubemap", 5);
  // renderer.loadCubemap("pure-sky", "../cubemaps/pure-sky", 5);
}

void Game::loadShaders()
{
  // renderer.loadShader("phong-pixel", "../shaders/phong-pixel.vs", "../shaders/phong-pixel.fs");
  renderer.loadShader("texture", "../shaders/texture.vs", "../shaders/texture.fs");
  renderer.loadShader("cubemap", "../shaders/cubemap.vs", "../shaders/cubemap.fs");
  renderer.loadShader("billboard-animated", "../shaders/billboard-animated.vs", "../shaders/billboard-animated.fs");
  renderer.loadShader("fluid", "../shaders/fluid.vs", "../shaders/fluid.fs");
  renderer.loadShader("billboard", "../shaders/billboard.vs", "../shaders/billboard.fs");
  renderer.loadShader("vignette", "../shaders/vignette.vs", "../shaders/vignette.fs");
}

void Game::loadMeshes()
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

vec3 Game::scaleVector(PLYMesh mesh, string meshName)
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

vec3 Game::centerVector(PLYMesh mesh, string meshName)
{
  vec3 minBounds = mesh.minBounds();
  vec3 maxBounds = mesh.maxBounds();
  float centroidX = (minBounds[0] + maxBounds[0]) / 2.0f;
  float centroidY = (minBounds[1] + maxBounds[1]) / 2.0f;
  float centroidZ = (minBounds[2] + maxBounds[2]) / 2.0f;
  return vec3(-centroidX, -centroidY, -centroidZ);
}

void Game::startGame() {
  float timer = elapsedTime();
  renderer.fontSize(width() / 13);
  string message; float x; float y;
  if (timer < 27) {
    if (timer < 10) {
      renderer.setDepthTest(false);
      renderer.blendMode(agl::ADD);  
      renderer.beginShader("vignette");
      renderer.setUniform("Resolution", vec2(width(), height()));
      renderer.setUniform("Time", timer);
      renderer.push();
      renderer.translate(vec3(0, 0, _viewVolumeSide / 2.0f));
      renderer.scale(vec3(width(), height(), _viewVolumeSide));
      renderer.translate(vec3(-0.5, -0.5, 0.0));
      renderer.quad();
      renderer.pop();
      renderer.endShader(); 
      renderer.setDepthTest(true);
      renderer.blendMode(agl::DEFAULT);  
    } else if (timer < 13) {
      message = "\"Hey, you there!\"";   
    } else if (timer < 18) {
      message = "\"Welcome to Omicron Persei 8, stranger. I'm Glorb.\"";
    } else if (timer < 23) {
      message = "\"Fancy a game of pool while you find your bearings?\"";
    } else {
      message = "\"Fair warning, it might not be quite like you expect...\"";
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

void Game::createPoolBalls()
{
  float k = (float) 3 / 1;
  float a = (_tableWidth - 100) / 2;
  for (int i = 0; i < _numBalls; i++)
  {
    Ball ball;
    ball.id = i + 1;
    float theta = ((float) i / _numBalls) * M_PI;
    float r = a * cos(k * theta);
    ball.pos.x = r * cos(theta);
    ball.pos.y = r * sin(theta);
    ball.vel = vec3(0);
    ball.rot = vec3(0);
    ball.color = vec4(1.0);
    ball.size = _ballDefaultSize;
    _balls.push_back(ball);
  }
}

void Game::createTrajectoryDots()
{
  for (int i = 0; i < 5; i++)
  {
    vec3 offset = (1.0f / (i + 1)) * _launchVel;
    vec3 trajectoryDot = _balls[_activeBall].pos + offset;
    _trajectoryDots.push_back(trajectoryDot);
  }
}

void Game::updateTrajectoryDots()
{
  for (int i = 0; i < 5; i++)
  {
    vec3 offset = (1.0f / (i + 1)) * _launchVel;
    vec3 trajectoryDot = _balls[_activeBall].pos + offset;
    _trajectoryDots[i] = trajectoryDot;
  }
}

void Game::createPockets()
{
  for (int i = 0; i < 6; i++)
  {
    vec3 pocket;
    if ((i % 3) == 0) {
      pocket.x =  -(_tableLength / 2) + 50;
    } else if ((i % 3) == 1) {
      pocket.x =  0;
    } else {
      pocket.x =  (_tableLength / 2) - 50;
    }
    pocket.y = (i < 3) ? (_tableWidth / 2) - 50 : -(_tableWidth / 2) + 50;
    pocket.z = 0;
    _pockets.push_back(pocket);
  }
}

void Game::updatePoolBalls()
{
  for (int i = 0; i < _numBalls; i++)
  {
    Ball ball = _balls[i];
    if (ball.size == 2 * _ballDefaultSize) {
      // floating up to glorb
      float glorbRadius = _eyeScaleVector.x * _eyeDiameterModifier * 0.5;
      if (length(_glorbPos - ball.pos) <= glorbRadius) {
        ball.pos = vec3(0, 1000, 200);
        ball.vel = vec3(0);
        ball.size = 0;
        _eyeDiameterModifier += 0.02;
        _eyeColor = vec4(0, 1, 0, 0.5);
        _numBallsSunk += 1;
        if (_numBallsSunk == 16) _endGame = true;
      }
    } else {
      bool sinking = pocketDetection(ball);
      if (!sinking) {
        bool collided = false;
        for (int j = i + 1; j < _numBalls && !collided; j++)
        {
          collided = collisionDetection(i, j);
        }
        ball = _balls[i];
        boundaryDetection(ball);
        if (_chaosStatus["Tilt-a-Table"]) ball.vel += vec3(1, 0, 0);
        ball.vel *= _chaosStatus["Friction Affliction"] ? 0.8f : 0.99f;
        // if not hovering and not floating up to glorb, null z
        if (ball.pos.z < 40) {
          ball.vel.z = 0;
        }
      }
    }
    vec3 dist = ball.vel * dt();
    ball.pos += dist;
    ball.rot += vec3(-dist.y, dist.x, 0) * float((M_PI * 2) / (M_PI * ball.size * _sphereRadius * 2));
    _balls[i] = ball;
  }
}

bool Game::collisionDetection(int i, int j)
{
  Ball ball1 = _balls[i];
  Ball ball2 = _balls[j];
  // if not floating up to glorb
  if (ball2.size != 2 * _ballDefaultSize) {
    float overlap = _sphereRadius * (ball1.size + ball2.size) - length(ball1.pos - ball2.pos);
    if (overlap > 0.1)
    {
      vec3 normal = normalize(ball1.pos - ball2.pos);
      ball1.pos += normal * overlap / 2.0f;
      ball2.pos -= normal * overlap / 2.0f;
      float ball1Vel = length(ball1.vel);
      float ball2Vel = length(ball2.vel);
      if (ball1Vel < ball2Vel) {
        ball1.vel = (ball1.vel + ball2.vel) / 4.0f;
        ball2.vel = reflect(ball2.vel, -normal) / 2.0f;
      } else if (ball1Vel > ball2Vel) {
        ball1.vel = reflect(ball1.vel, normal) / 2.0f;
        ball2.vel = (ball1.vel + ball2.vel) / 4.0f;
      } else {
        ball1.vel = reflect(ball1.vel, normal);
        ball2.vel = reflect(ball2.vel, -normal);
      }
      _balls[i] = ball1;
      _balls[j] = ball2;
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

void Game::boundaryDetection(Ball& ball)
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

bool Game::pocketDetection(Ball& ball)
{
  for (int i = 0; i < 6; i++)
  {
    if (length(_pockets[i] - ball.pos) < _viewVolumeSide / 150)
    {
      _congratsMessage = congratsMessages[ball.id - 1];
      _congratsStartTime = elapsedTime() + 1;
      ball.vel = 0.5f * (_glorbPos - ball.pos);
      ball.size *= 2;
      return true;
    }
    else if (length(_pockets[i] - ball.pos) < _viewVolumeSide / 50)
    {
      ball.vel = 10.0f * (_pockets[i] - ball.pos);
      return true;
    }
  }
  return false;
}

void Game::endGame() {
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
    } else if (timer < 12) {
      _eyeDiameterModifier += (width() / 500) * 0.007;
    } else {
      renderer.fontSize(width() / 3);
      message = "FIN";
      x = width() / 2 - renderer.textWidth(message) * 0.5f;
      y = height() / 2 + renderer.textHeight() * 0.25f;
      renderer.text(message, x, y);
    }
  }
}

int Game::launchDetection(int clickX, int clickY)
{
  float closestDist = 99999999;
  int closestDistIdx = -1;
  vec2 clickPos = vec2(clickX, clickY);
  for (int i = 0; i < _numBalls; i++)
  {
    // if not floating up to glorb
    if (_balls[i].size != 2 * _ballDefaultSize) {
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
  }
  return closestDistIdx;
}

vec2 Game::worldToScreen(vec3 worldPos, bool flipY) {
  mat4 viewMat = inverse(_sceneRotMat * inverse(renderer.viewMatrix()));
  vec4 eyePos = viewMat * vec4(worldPos, 1.0);
  mat4 projMat = renderer.projectionMatrix();
  vec4 projPos = projMat * eyePos;
  projPos /= projPos.w;
  float screenX = (projPos.x + 1.0f) * width() / 2;
  float screenY;
  if (flipY) screenY = (projPos.y + 1.0f) * height() / 2;
  else screenY = (1.0f - projPos.y) * height() / 2;
  return vec2(screenX, screenY);
}

void Game::drawPoolTable()
{
  renderer.setUniform("MaterialColor", vec4(1));
  renderer.texture("Image", "pool-table");
  renderer.push();
  renderer.translate(vec3(0, 0, -75));
  renderer.scale(_tableScaleVector);
  renderer.rotate(vec3(0, 0, M_PI_2));
  renderer.translate(_tableCenterVector);
  renderer.mesh(_poolTableMesh);
  renderer.pop();
}

void Game::drawCueStick()
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

void Game::drawPoolBalls()
{
  renderer.setUniform("PoolBall", true);
  for (int i = 0; i < _numBalls; i++)
  {
    renderer.setUniform("MaterialColor", _balls[i].color);
    renderer.texture("Image", "ball_" + to_string(i + 1));
    renderer.push();
    renderer.translate(_balls[i].pos);
    renderer.rotate(_balls[i].rot);
    renderer.rotate(vec3(0, 0, M_PI));
    renderer.scale(vec3(_balls[i].size));
    renderer.sphere();
    renderer.pop();
  }
  renderer.setUniform("PoolBall", false);
}

void Game::drawTrajectoryDots()
{
  if (_launching)
  {
    renderer.setDepthTest(false);
    renderer.blendMode(agl::ADD);
    renderer.beginShader("texture");
    renderer.setUniform("Color", vec4(1));
    renderer.texture("Image", "trajectoryDot");
    for (int i = 0; i < _trajectoryDots.size(); i++)
    {
      renderer.push();
      renderer.translate(_trajectoryDots[i]);
      renderer.scale(vec3(5 + (4 - i)));
      renderer.translate(vec3(-0.5, -0.5, 0));
      renderer.quad();
      renderer.pop();
    }
    renderer.endShader();
    renderer.setDepthTest(true);
    renderer.blendMode(agl::DEFAULT);
  }
}

void Game::drawEye()
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

void Game::chaos()
{
  if (_chaosStatus["Hover Havoc"])
  {
    for (int i = 0; i < _numBalls; i++)
    {
      // if floating but not going up to glorb
      if (_balls[i].pos.z >= 40 && _balls[i].size != 2 * _ballDefaultSize)
      {
        _balls[i].pos.z = 50.0f + 10 * sin(elapsedTime());
      }
    }
  }

  int newEffectThresh = 250;
  int frame = int(_time * 30);
  if (frame % newEffectThresh == 0 && !_startGame && !_endGame)
  {
    _chaosAnimStart = elapsedTime() + 1;
    _chaosAnimation = true;
    string effect = _chaosEffects[rand() % _chaosEffects.size()];
    while (effect == _chaosEffect) {
      effect = _chaosEffects[rand() % _chaosEffects.size()];
    } 
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

void Game::gravityChaos()
{
  for (int i = 0; i < _numBalls; i++)
  {
    // if not floating up to glorb
    if (rand() % 4 == 0 && _balls[i].size != 2 * _ballDefaultSize)
    {
      _balls[i].pos.z = 50.0f + 10 * sin(elapsedTime());
    }
  }
}

void Game::resetGravity()
{
  for (int i = 0; i < _numBalls; i++)
  {
    _balls[i].pos.z = 0;
  }
}

void Game::sizeChaos()
{
  for (int i = 0; i < _numBalls; i++)
  {
    if (rand() % 4 == 0)
    {
      // if not floating up to glorb
      if (_balls[i].size != 2 * _ballDefaultSize) {
        float prevSize = _balls[i].size;
        (rand() % 2)? _balls[i].size *= 3 : _balls[i].size /= 2; 
        _balls[i].pos.z += _sphereRadius * (_balls[i].size - prevSize);
      }
    }
  }
}

void Game::resetSize()
{
  for (int i = 0; i < _numBalls; i++)
  {
    float prevSize = _balls[i].size;
    _balls[i].size = _ballDefaultSize;
    _balls[i].pos.z += _sphereRadius * (_balls[i].size - prevSize);
  }
}

void Game::drawChaosTransition() {
  renderer.setDepthTest(false);
  renderer.blendMode(agl::ADD);
  renderer.beginShader("billboard-animated");
  renderer.texture("Image", "fireball");
  int numRows = 2;
  int numCols = 8;
  int frame = int(_time * 30) % (numRows * numCols);
  renderer.setUniform("Frame", frame);
  renderer.setUniform("Rows", numRows);
  renderer.setUniform("Cols", numCols);
  renderer.setUniform("TopToBottom", false);
  renderer.push();
  renderer.rotate(vec3(0, M_PI, _azimuth - M_PI));
  renderer.sprite(vec3(0, -0.4* _viewVolumeSide, 0), vec4(1.0f), 0.6 * _viewVolumeSide, 0.0);
  renderer.pop();
  renderer.endShader();
  renderer.setDepthTest(true);
  renderer.blendMode(agl::DEFAULT);
}

void Game::drawLogo() {
  renderer.beginShader("texture");
  renderer.texture("Image", "logo");
  renderer.push();
  float xPos = _viewVolumeSide / 2.75;
  float yPos = -_viewVolumeSide / 6.7;
  float zPos = _viewVolumeSide / 3.5;
  renderer.translate(vec3(xPos, yPos, zPos));
  vec3 logoPos = vec3(xPos, yPos, zPos);
  vec3 lookPos = vec3(_camPos.x, -_camPos.z, _camPos.y);
  vec3 z = normalize(lookPos - logoPos);
  vec3 x = normalize(cross(vec3(0, 0, 1), z));
  vec3 y = normalize(cross(z, x));
  mat3 R = mat3(x, y, z);
  renderer.rotate(R);
  renderer.scale(vec3(_viewVolumeSide / 3));
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
  renderer.pop();
  renderer.endShader();
}

void Game::drawFluid() {
  renderer.setDepthTest(false);
  renderer.blendMode(agl::ADD);
  renderer.beginShader("fluid");
  renderer.setUniform("Resolution", vec2(width(), height()));
  renderer.setUniform("Time", elapsedTime());
  vec3 ballPos;
  if (_activeBall == -1) {
    ballPos = vec3(worldToScreen(vec3(0), true), 1);
  } else {
    ballPos = vec3(worldToScreen(_balls[_activeBall].pos, true), 1);
  }
  renderer.setUniform("BallPos", ballPos);
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

void Game::drawSkybox(string cubemapName) {
  renderer.push();
  renderer.cubemap("Cubemap", cubemapName);
  renderer.setUniform("Skybox", true);
  renderer.skybox(_viewVolumeSide * _skyBoxSize);
  renderer.setUniform("Skybox", false);
  renderer.pop();
}

void Game::updateCamPos()
{
  float x = _radius * sin(_azimuth) * cos(_elevation);
  float y = _radius * sin(_elevation);
  float z = _radius * cos(_azimuth) * cos(_elevation);
  _camPos =  vec3(x, y, z);
}

void Game::mouseMotion(int x, int y, int dx, int dy)
{
  if (_leftClick)
  {
    if (_launching)
    {
      vec4 addVel = vec4(-dx, dy, 0, 0);
      addVel = glm::rotate(mat4(1.0), _azimuth, vec3(0, 0, 1)) * addVel;
      _launchVel += vec3(addVel);
      updateTrajectoryDots();
    }
    else if (!_orbiting)
    {
      int closestDistIdx = launchDetection(x, y);
      if (_launching)
      {
        _activeBall = closestDistIdx;
        _balls[_activeBall].vel = vec3(0);
        _balls[_activeBall].color /= 2.0f;
        vec4 launchVel = vec4(-dx, dy, 0, 0);
        launchVel = glm::rotate(mat4(1.0), _azimuth, vec3(0, 0, 1)) * launchVel;
        _launchVel = vec3(launchVel);
        createTrajectoryDots();
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

void Game::orbit(float dx, float dy)
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
  _flipY = !(_azimuth < M_PI_2 || _azimuth > 1.5 * M_PI);
}

void Game::mouseDown(int button, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT)
  {
    _leftClick = true;
  }
}

void Game::mouseUp(int button, int mods)
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
      _trajectoryDots.clear();
    }
    else if (_orbiting)
    {
      _orbiting = false;
    }
  }
}

void Game::scroll(float dx, float dy)
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

void Game::keyUp(int key, int mods)
{
  if (key == GLFW_KEY_R)
  {
    for (int i = 0; i < _numBalls; i++)
    {
      _balls[i].vel = vec3(0);
    }
  } else if (key == GLFW_KEY_E) {
    _congratsStartTime = elapsedTime();
    _endGame = true;
  } else if (key == GLFW_KEY_S) {
    _startGame = false;
    _showLogo = true;
    _elevation = M_PI_4 * 0.75;
  } else if (key == GLFW_KEY_X) {
     screenshot("../demo/screenshot-" + std::to_string(rand() % 10000) + ".png");
  } 
}

void Game::draw()
{
  float aspect = width() / height();
  renderer.perspective(glm::radians(60.0f), aspect, 0.1f, _viewVolumeSide * _skyBoxSize);

  updateCamPos();
  renderer.lookAt(_camPos, _lookPos, _up);

  _time += dt();

  if (elapsedTime() - _congratsStartTime > 1)
  {
    _eyeColor = vec4(1);
  }

  renderer.beginShader("cubemap");

  renderer.setUniform("ModelMatrix", renderer.modelMatrix());
  renderer.setUniform("ViewMatrix", renderer.viewMatrix());
  renderer.setUniform("CamPos", _camPos);

  drawSkybox("shanghai-bund");
  renderer.push();
  renderer.rotate(vec3(-M_PI_2, 0, 0));
  if (_chaosAnimation) drawChaosTransition();
  drawPoolTable();
  drawFluid();
  drawCueStick();
  updatePoolBalls();
  drawPoolBalls();
  drawTrajectoryDots();
  if (_showLogo) drawLogo();
  drawEye();
  if (_enableChaos) chaos();
  if (_endGame) endGame();
  if (_startGame) startGame();
  else _glorbPos.y = 0;
  if (_endGame) endGame();
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
  if (!_startGame && !_endGame) {
    if (_chaosEffect == "Plain Jane") {
      renderer.fontColor(vec4(0, 1, 0, 1));
    } else {
      renderer.fontColor(vec4(1, 0, 0, 1));
    }
    renderer.fontSize(width() / 15);
    string message = "STATUS EFFECT: " + _chaosEffect;
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
  renderer.pop();
  
  renderer.endShader();
  
  if (_time - int(_time) < 0.01)
    cout << int(1/dt()) << " FPS" << endl;
}

int main(int argc, char **argv)
{
  Game game;
  game.run();
  return 0;
}
