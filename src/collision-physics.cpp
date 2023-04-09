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
    _radius = 20;
    srand(elapsedTime());
  }

  void setup() {
    setWindowSize(_width, _height);
    createBalls();
    renderer.setDepthTest(false);
    renderer.blendMode(agl::ADD);
    renderer.loadTexture("ball", "../textures/white-circle.png", 0);
  }

void createBalls()
  {
    for (int i = 0; i < _numBalls; i++) {
      Ball ball;
      ball.id = i;
      ball.pos.x = pow(-1, rand()) * (rand() % 26);
      ball.pos.y = pow(-1, rand()) * (rand() % 26);
      // ball.vel = vec3(random(-0.005, 0.005), random(-0.005, 0.005), 0);
      ball.vel = vec3(0);
      ball.color.x = std::max(rand() % 256, 64) / 255.0f;
      ball.color.y = std::max(rand() % 256, 64) / 255.0f;
      ball.color.z = std::max(rand() % 256, 64) / 255.0f;
      ball.color.w = 1.0;
      ball.size = 5.0;
      mBalls.push_back(ball);
    }
  }

  void updateBalls()
  {
    std::vector<Ball> newBalls = mBalls;
    for (int i = 0; i < _numBalls; i++) {
      Ball ball = mBalls[i];
      Ball newBall = newBalls[i];
      for (int j = 0; j < _numBalls; j++) {
        Ball otherBall = mBalls[j];
        if (ball.id != otherBall.id && length(ball.pos - otherBall.pos) <= 3.5) {
          newBall.vel = (otherBall.vel - ball.vel) / 2.0f;
          break;
        } 
      }
      newBall.pos += newBall.vel * dt();
      if (newBall.pos.x < -25 || newBall.pos.x > 25) {
        newBall.pos.x = (newBall.pos.x < -25)? -25 : newBall.pos.x;
        newBall.pos.x = (newBall.pos.x > 25)? 25 : newBall.pos.x;
        newBall.vel.x = -newBall.vel.x;
      }
      if (newBall.pos.y < -25 || newBall.pos.y > 25) {
        newBall.pos.y = (newBall.pos.y < -25)? -25 : newBall.pos.y;
        newBall.pos.y = (newBall.pos.y > 25)? 25 : newBall.pos.y;
        newBall.vel.y = -newBall.vel.y;
      }
      // cout << "pos: " << newBall.pos << "  vel: " << newBall.vel << endl;
      newBalls[i] = newBall;
    }
    mBalls = newBalls;
  }

  void drawBalls()
  {
    renderer.texture("image", "ball");
    for (int i = 0; i < _numBalls; i++)
    {
      renderer.sprite(mBalls[i].pos, mBalls[i].color, mBalls[i].size, 0.0);
    }
  }

void mouseMotion(int x, int y, int dx, int dy) {
      if (_leftClick) {
        if (_launching) {
          _launchVel += vec3(-dx, dy, 0);
          // cout << _launchVel << endl;
        } else {
          float closestDist = 99999999;
          float closestDistIdx = -1;
          for (int i = 0; i < _numBalls; i++) {
            int clickX = (x / 10.0f) - 25;
            int clickY = -((y / 10.0f) - 25);
            // cout << ball.pos << " vs " << clickX << " " << clickY << "  diff: " << abs(ball.pos.x - clickX) << " " << abs(ball.pos.y - clickY) << endl;
            if (length(mBalls[i].pos - vec3(clickX, clickY, 0)) < 2.5) {
              // cout << "threshold crossed" << endl;
              float dist = length(mBalls[i].pos - vec3(clickX, clickY, 0));
              if (dist < closestDist) {
                closestDist = dist;
                closestDistIdx = i;
              }
              _launching = true;
            } 
          }
          if (_launching) {
            Ball closestBall = mBalls[closestDistIdx];
            closestBall.vel = vec3(0);
            mBalls[closestDistIdx] = closestBall;
            _activeBall = closestDistIdx;
            _launchVel = vec3(-dx, dy, 0);
            // cout << _launchVel << endl;
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
          // cout << "active ball: " << _activeBall << "  launchVel: " << _launchVel << endl;
          mBalls[_activeBall].vel = _launchVel;
          _launching = false;
        }
      }
   }

  void scroll(float dx, float dy) {
    eyePos.x += dy;
  }

  void keyUp(int key, int mods) {
    if (key == GLFW_KEY_R) {
      for (int i = 0; i < _numBalls; i++) {
        mBalls[i].vel = vec3(0);
      }  
    }
  }

  void draw() {
    renderer.beginShader("sprite");

    float aspect = ((float)width()) / height();
    renderer.perspective(glm::radians(60.0f), aspect, 0.1f, 50.0f);
    renderer.lookAt(eyePos, lookPos, up);

    updateBalls();
    drawBalls();

    renderer.endShader();
  }

protected:

  vec3 eyePos = vec3(0, 0, 40);
  vec3 lookPos = vec3(0, 0, 0);
  vec3 up = vec3(0, 1, 0);

  std::vector<Ball> mBalls;
  int _numBalls = 10;
  int _width;
  int _height;
  int _radius;

  bool _leftClick = false;
  bool _launching = false;
  int _activeBall;
  vec3 _launchVel;
};

int main(int argc, char** argv)
{
  Viewer viewer;
  viewer.run();
  return 0;
}
