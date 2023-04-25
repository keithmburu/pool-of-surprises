# Pool of Surprises
## A Wild and Wacky Billiards Bonanza

<img src="demo/screenshot-4.png" width="400">

This project presents an alternative take on the classic game of pool/billiards. 

The user is able to launch the pool balls using a virtual cue stick, and resulting collisions among the balls are modeled realistically. Visual cues are also provided to help the user estimate launch trajectory. The game includes textures and environment cubemaps, with corresponding environment-mapped reflections implemented. 

Also implemented is an anomalous "fluid" on the table surface that reacts to the balls as they roll along it. Furthermore, an "Eye of Sauron" tracks the user and the ball most recently interacted with. Finally, the game physics are randomly tweaked in humorous ways, such as by causing some balls to hover, changing the sizes of some balls, making balls stick to the table edges, tilting the table, and subverting the expected launch direction of the balls.

Overall, this is a distinctive and fun offshoot of the traditional pool game, with unique features that make gameplay more dynamic.

## Results

<img src="demo/screenshot-4891.png" width="400">
<img src="demo/screenshot-1144.png" width="400">
<img src="demo/screenshot-3938.png" width="400">


## Details

The PLYMesh class is used to represent the mesh data (positions, normals, UVs) of the game objects, including the pool table, pool balls, and cue stick. A struct data structure is used to store the position, velocity, rotation, color, and size of each ball in the game. A map from strings to booleans is used to keep track of the activation of different status effects.

The starting position of the balls is not in the usual triangular pool rack, but instead in a rose curve shape, described by k = 3 / 1, a = half of table width, theta = (ball index / number of balls) * pi, and r = a * cos(k * theta). I set the x-position of the ball at each index to r * cos(theta), and its y-position to r * sin(theta) in order to form this shape.

To create the vignetting going on at the start of the game, I calculate an alpha value for each pixel based on the distance between the pixel and the center of the screen. I then multiply this alpha value by an intensity value that starts at 1.0 and decreases smoothly across all the pixels down to 0.0 over a period of 4 seconds. By multiplying the product of the alpha and intensity by vec3(1, 1, 1), we get an effect where pixels that are further away from the center are a less transparent white than those closer to the center, and the overall whiteness fades out over 4 seconds.

When the game starts, and the user interacts with a ball, a launch detection algorithm takes the x and y position of the click and loops through all the balls to find the closest ball that is within the click area. To accomplish this, it converts the position of each ball from world coordinates to screen coordinates using the view and projection matrices before testing for 2D intersection.

To calculate the launch velocity of this closest ball, I start with the change in mouse coordinates vec2(-dx, dy). Since the user is able to move around the table, I also rotate this vector by the number of radians between the user and their initial position so that the ball is launched in the desired direction.

The game enables the user to estimate the trajectory of the ball before it is launched by drawing trajectory dots along the path given by the launch velocity of the ball. The trajectory dots are spaced further apart the further they are from the ball.

To have the balls roll along the surface of the pool table realistically, we update the rotation of each ball based on the distance it has traveled on the screen. The change in rotation during every frame is calculated by taking the distance the ball has traveled on the screen, and converting that to radians by multiplying it by 2 * pi / c, where c represents the circumference of the ball (one full rotation corresponds to one circumference). 

When two balls inevitably get too close to each other, a collision detection algorithm calculates the overlap distance between each pair of balls, and if the overlap is greater than 0.1, the two balls are moved away from each other by half the overlap distance along the normal vector between them. The velocities of the two balls are then updated by reflecting them across the collision normal. If one ball is moving faster than the other, it is reflected off the other ball while the slower ball is pushed away. If the balls have the same speed, they both reflect off each other. 

When a ball approaches a table edge, a boundary detection algorithm checks whether the ball has gone beyond the threshold in either the x or y direction. If the condition is met, the algorithm adjusts the ball's position so that it is within the boundaries of the table and reflects its velocity in the appropriate direction. 

A pocket detection algorithm checks if a ball is sufficiently close to one of six pockets on the table. If the distance between the center of the ball and the center of the pocket is less than a certain threshold, the ball has sunk into the pocket. If the ball is close to a pocket but not close enough, its velocity is modified slightly such that it moves towards the pocket. If all 16 balls are sunk, the game ends.

To simulate friction, each ball's velocity is multiplied by 0.99 during every frame.

If the "Friction Affliction" status effect is on, each ball's velocity is multiplied by 0.8 instead of 0.99 so that its speed is reduced significantly during every frame.

If the "Sticky Situation" status effect is active and a ball hits a table edge, the ball's velocity in the reflected direction is set to zero instead of being reversed such that it sticks.

If the "Tilt-a-Table" status effect is on, vec3(1, 0, 0) is added to each ball's velocity such that they all roll to one side of the table.

If the "Hover Havoc" status effect is on, the height of each ball in a random subset is changed from 0.0f to 50.0f + 10 * sin(elapsedTime()), such that the balls float over the table, moving smoothly up and down with time.

If the "Biggie Smalls" status effect is on, each ball from a random subset is made to either be three times its size or half its size. Their positions are also adjusted accordingly such that their lowest points still lie on the table.

If the "Get Gaslit" status effect is on, the y-direction of the ball being launched is inverted so that aiming the balls towards the pockets is made more difficult.

The orientation of the "Eye of Sauron" is determined by the game state and modified using point billboarding. If the game is just starting, the most recently launched ball is moving slow enough, or if the game is ending, the eye is pointed at the user. Otherwise, the eye is pointed at the ball most recently interacted with. To set the eye orientation, the program first finds the vector pointing from the camera to the designated look-at position. This vector is then used to calculate two additional vectors using cross product, one perpendicular to the up vector and the look-at vector, and the other perpendicular to both of the first two vectors. These three vectors are then used to create a rotation matrix, which is applied to the eye. The logo is also oriented in this way, but unlike the eye, it is set to face the user at all times.

For the "fluid" simulation, a shader calculates each pixel's position in UV coordinates, as well as the position of the ball most recently interacted with in UV coordinates. It then iterates over a 2D grid of points, calculating for each point the closest point on the line segment between the pixel and the ball. To do this, we use vector projection to project the point onto the line segment. The color c1 of the pixel is initialized to black, and for each point in the grid, it is updated by mixing it with a new color c2 based on the above stated distance. The greater the distance is, the closer the final color will be to the original value of c2 (black). The closer it is however, the closer it will be to c2. The color c2 is defined as vec3(0, clamp(length(line) + sin(Time), 0.2, 0.4), 0). Thus, it is a variant of the color green with its intensity determined by the length of the line between the pixel and the ball, as well as the sin of time for a breathing effect.

## How to build

*Windows*

Open git bash to the directory containing this repository.

```
pool-of-surprises $ mkdir build
pool-of-surprises $ cd build
pool-of-surprises/build $ cmake ..
pool-of-surprises/build $ start pool-of-surprises.sln
```

To run from the git bash command shell, 

```
pool-of-surprises/build $ ../bin/Debug/pool-of-surprises.exe
```

*macOS*

Open terminal to the directory containing this repository.

```
pool-of-surprises $ mkdir build
pool-of-surprises $ cd build
pool-of-surprises/build $ cmake ..
pool-of-surprises/build $ make
```

To run the program from build, you would type

```
pool-of-surprises/build $ ../bin/pool-of-surprises
```

## Credits

- "Pool Table" (https://skfb.ly/oCIBB) by Jayden Miles is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/).

- "Cue stick (LP)" (https://skfb.ly/6ASFq) by afferu is licensed under Creative Commons Attribution-ShareAlike (http://creativecommons.org/licenses/by-sa/4.0/).

- "Eye" (https://skfb.ly/RJG7) by germydan is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/).

