/**
 * @file pool-of-surprises.h
 * @author Keith Mburu
 * @date 2023-04-24
 * @brief An alternative version of pool
 */

#include <string>
#include <vector>
#include <map>
#include "agl/window.h"
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

class Game : public Window
{
public:
    Game() : Window()
    {
        setWindowSize(500, 500);
    }

    virtual ~Game() 
    {
    }

    /**
    * Initializes the application and sets up the environment for rendering.
    */
    void setup();

    /**
    * Loads the textures required for rendering.
    */
    void loadTextures();

    /**
    * Loads the cubemaps required for rendering.
    */
    void loadCubemaps();

    /**
    * Loads the shaders required for rendering.
    */
    void loadShaders();

    /**
    * Loads the meshes required for rendering.
    */
    void loadMeshes();

    /**
    * Computes the vector that scales the mesh in the corresponding x, y, and z * direction in order to fit the mesh in the view volume
    *
    * @param mesh The mesh to scale.
    * @param meshName The name of the mesh.
    * @return The scaling vector.
    */
    vec3 scaleVector(PLYMesh mesh, string meshName);

    /**
    * Computes the centroid vector that is subtracted from every vertex of the * mesh in order to center the mesh at the origin
    *
    * @param mesh The mesh to center.
    * @param meshName The name of the mesh.
    * @return The centeroid vector.
    */
    vec3 centerVector(PLYMesh mesh, string meshName);

    /**
    * Runs the start-of-game sequence.
    */
    void startGame();

    /**
    * Initializes the pool balls, their positions, velocities, etc.
    */
    void createPoolBalls();

    /**
    * Initializes dots that visualize active ball trajectory.
    */
    void createTrajectoryDots();

    /**
    * Initializes the "pockets" in the pool table and their positions.
    */
    void createHoles();

    /**
    * Updates the positions,velocities, etc. of the pool balls.
    */
    void updatePoolBalls();

    /**
    * Detects if two balls have collided.
    *
    * @param i The index of the first ball.
    * @param j The index of the second ball.
    * @return True if there is a collision, false otherwise.
    */
    bool collisionDetection(int i, int j);

    /**
    * Detects if a ball has hit the boundary of the pool table and changes its * velocity accordingly.
    *
    * @param ball The ball to check for a collision.
    */
    void boundaryDetection(Ball& ball);

    /**
    * Detects if a ball is sufficiently close to a hole and executes           * the appropriate game actions.
    *
    * @param ball The ball to check.
    * @return True if the ball is sunk, false otherwise.
    */
    bool holeDetection(Ball& ball);

    /**
    * Runs the end-of-game sequence.
    */
    void endGame();

    /**
    * Detects if the user has clicked to launch a ball.
    *
    * @param clickX The x-coordinate of the click.
    * @param clickY The y-coordinate of the click.
    * @return The index of the closest ball that overlaps with the click *         position
    */
    int launchDetection(int clickX, int clickY);

    /**
    * Converts a world position to a screen position.
    *
    * @param worldPos The world position to convert.
    * @param flipY If true, flips the y-coordinate.
    * @return The converted screen position.
    */
    vec2 worldToScreen(vec3 worldPos, bool flipY);

    /**
    * Renders the pool table, rotating, centering and scaling it as appropriate.
    */
    void drawPoolTable();

    /**
    * Renders the cue stick, rotating, centering and scaling it as appropriate.
    */
    void drawCueStick();

    /**
    * Draws the pool balls, rotating, centering and scaling them as appropriate.
    */
    void drawPoolBalls();

    /**
    * Draws the trajectory dots for trajectory estimation.
    */
    void drawTrajectoryDots();

    /**
    * Draws the Eye of Sauron, rotating it to face the user of the active ball
    */
    void drawEye();

    /**
    * Randomly changes the physics of the game.
    */
    void chaos();

    /**
    * Causes a random subset of balls to hover over the table.
    */
    void gravityChaos();

    /**
    * Resets the positions of the floating balls.
    */
    void resetGravity();

    /**
    * Causes a random subset of balls to either grow or shrink.
    */
    void sizeChaos();

    /**
    * Resets the sizes of the balls.
    */
    void resetSize();

    /**
    * Draws the explosion animation when the game physics change.
    */
    void drawChaosTransition();

    /**
    * Draws the game logo, rotating it to face the user.
    */
    void drawLogo();

    /**
    * Renders the quad on the table surface where the fluid is rendered.
    */
    void drawFluid();

    /**
    * Draws the skybox.
    *
    * @param cubemapName The name of the cubemap to use
    */
    void drawSkybox(string cubemapName);

    /**
    * Updates camera position based on current azimuth and elevation.
    */
    void updateCamPos();

    /**
     * Responds to mouse motion events.
     *
     * @param x The current x-coordinate of the mouse cursor.
     * @param y The current y-coordinate of the mouse cursor.
     * @param dx The change in the x-coordinate of the mouse cursor.
     * @param dy The change in the y-coordinate of the mouse cursor.
     */
    void mouseMotion(int x, int y, int dx, int dy);

    /**
     * Updates camera azimuth and elevation based on changes in x and y mouse *  coordinates.
     *
     * @param dx The change in the x-coordinate of the mouse cursor.
     * @param dy The change in the y-coordinate of the mouse cursor.
     */
    void orbit(float dx, float dy);

    /**
     * Responds to mouse down events.
     *
     * @param button The button that was pressed.
     * @param mods The modifier keys that were pressed.
     */
    void mouseDown(int button, int mods);

    /**
     * Responds to mouse up events.
     *
     * @param button The button that was released.
     * @param mods The modifier keys that were pressed.
     */
    void mouseUp(int button, int mods);

    /**
     * Responds to scroll events.
     *
     * @param dx The change in the x-coordinate of the mouse cursor.
     * @param dy The change in the y-coordinate of the mouse cursor.
     */
    void scroll(float dx, float dy);

    /**
     * Responds to key up events.
     *
     * @param key The key that was released.
     * @param mods The modifier keys that were pressed.
     */
    void keyUp(int key, int mods);

    /**
     * Draws the current scene.
     */
    void draw();

protected:
    int _viewVolumeSide = 500;
    int _radius = 500;
    float _sphereRadius = 0.5;
    float _ballDefaultSize = _viewVolumeSide / 20;
    int _skyBoxSize = 10;
    mat4 _sceneRotMat;

    bool _startGame = true;
    bool _showLogo = false;
    bool _flipY = true;

    vec3 _camPos;
    vec3 _lookPos = vec3(0, 0, 0);
    vec3 _up = vec3(0, 1, 0);
    float _azimuth = M_PI;
    float _elevation = M_PI_2 - 0.017;
    float _orbiting = false;

    std::vector<Ball> _balls;
    int _numBalls = 16;
    int _numBallsSunk = 0;

    bool _leftClick = false;
    bool _launching = false;
    std::vector<vec3> _trajectoryDots;
    std::vector<vec3> _holes;
    int _activeBall = -1;
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
    bool _enableChaos = true;
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
