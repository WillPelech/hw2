#include "CS3113/cs3113.h"
//resources : https://www.raylib.com/examples/text/loader.html?name=text_format_text
//
enum Paddle1Status { HANGING, RELEASED };
enum ball_movement {LEFT, RIGHT};
enum ball_num {BALL_ONE = 1, BALL_TWO = 2, BALL_THREE = 3};
enum ball_status {ACTIVE, INACTIVE};
enum PaddleControlMode { FOLLOW, PLAYER };
// Global Constants
constexpr int SCREEN_WIDTH = 1000, SCREEN_HEIGHT = 650, FPS = 60, SPEED = 200,
              SHRINK_RATE = 100;

Vector2 ORIGIN = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2},
        BACKGROUND_BASE_SIZE = {SCREEN_WIDTH, SCREEN_HEIGHT},
        Paddle1_BASE_SIZE = {60.0f, 170.0f},
        
        Paddle2_BASE_SIZE = {60.0f, 170.0f},
        Paddle1_INIT_POS = {ORIGIN.x- 450.0f, ORIGIN.y },
        Paddle2_INIT_POS = {ORIGIN.x + 450.0f, ORIGIN.y };

Vector2 BALL1_BASE_SIZE = {30.0f, 30.0f},
        BALL2_BASE_SIZE = {30.0f, 30.0f},
        BALL3_BASE_SIZE = {30.0f, 30.0f};

constexpr char BACKGROUND_FP[] = "Assets/background.png";
constexpr char PADDLE1_FP[] = "Assets/bluedragon.png";
constexpr char PADDLE2_FP[] = "Assets/reddragon.png";
constexpr char BALL_FP[] = "Assets/fireball.png";
// Global Variables
AppStatus gAppStatus = RUNNING;
float gAngle = 0.0f, gPreviousTicks = 0.0f;
bool isPaddleControlled1 = false;
bool isPaddleControlled2 = false;
PaddleControlMode paddle2_state=FOLLOW;
ball_status ball2_status = INACTIVE;
ball_status ball3_status = INACTIVE;
Vector2 paddle1_position = Paddle1_INIT_POS, paddle1_movement = {0.0f, 0.0f},
        paddle1_scale = Paddle1_BASE_SIZE,

        paddle2_position = Paddle2_INIT_POS, paddle2_movement = {0.0f, 0.0f},
        paddle2_scale = Paddle2_BASE_SIZE,

        background_position = ORIGIN, background_movement = {0.0f, 0.0f},
        background_scale = BACKGROUND_BASE_SIZE,

        ball1_position = ORIGIN, ball1_movement = {0.0f, 0.0f},
        ball1_scale = BALL1_BASE_SIZE,

        ball2_position = ORIGIN, ball2_movement = {0.0f, 0.0f},
        ball2_scale = BALL2_BASE_SIZE,

        ball3_position = ORIGIN, ball3_movement = {0.0f, 0.0f},
        ball3_scale = BALL3_BASE_SIZE,

        gMousePosition = GetMousePosition();

Texture2D background_texture;
Texture2D paddle1_texture;
Texture2D paddle2_texture;
Texture2D ball1_texture;
Texture2D ball2_texture;
Texture2D ball3_texture;
// Game state
bool gameOver = false;
enum Winner {NONE_WINNER, BLUE_WINNER, RED_WINNER};
Winner winner = NONE_WINNER;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();
void resetGame();

// Linear interpolation function
float lerp(float start, float end, float t) {
    return start + t * (end - start);
}
bool isColliding(const Vector2 *postionA, const Vector2 *scaleA,
                 const Vector2 *positionB, const Vector2 *scaleB);

// Function Definitions

/**
 * @brief Checks for a square collision between 2 Rectangle objects.
 
 * @see
 *
 * @param postionA The position of the first object
 * @param scaleA The scale of the first object
 * @param positionB The position of the second object
 * @param scaleB The scale of the second object
 * @return true if a collision is detected,
 * @return false if a collision is not detected
 */

bool isColliding(const Vector2 *postionA, const Vector2 *scaleA,
                 const Vector2 *positionB, const Vector2 *scaleB) {
  float xDistance =
      fabs(postionA->x - positionB->x) - ((scaleA->x + scaleB->x) / 2.0f);
  float yDistance =
      fabs(postionA->y - positionB->y) - ((scaleA->y + scaleB->y) / 2.0f);

  if (xDistance < 0.0f && yDistance < 0.0f)
    return true;

  return false;
}

void renderObject(const Texture2D *texture, const Vector2 *position,
                  const Vector2 *scale) {
  // Whole texture (UV coordinates)
  Rectangle textureArea = {// top-left corner
                           0.0f, 0.0f,

                           // bottom-right corner (of texture)
                           static_cast<float>(texture->width),
                           static_cast<float>(texture->height)};

  // Destination rectangle – centred on paddle1_position
  Rectangle destinationArea = {position->x, position->y,
                               static_cast<float>(scale->x),
                               static_cast<float>(scale->y)};

  // Origin inside the source texture (centre of the texture)
  Vector2 originOffset = {static_cast<float>(scale->x) / 2.0f,
                          static_cast<float>(scale->y) / 2.0f};

  // Render the texture on screen
  DrawTexturePro(*texture, textureArea, destinationArea, originOffset, gAngle,
                 WHITE);
}

void initialise() {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "User Inout / Collision Detection");

  background_texture = LoadTexture(BACKGROUND_FP);
  paddle1_texture = LoadTexture(PADDLE1_FP);
  paddle2_texture = LoadTexture(PADDLE2_FP);
  ball1_texture = LoadTexture(BALL_FP);
  ball2_texture = LoadTexture(BALL_FP);
  ball3_texture = LoadTexture(BALL_FP);
  // Initialize game state
  resetGame();
  
  SetTargetFPS(FPS);
}

// Reset game state to initial position and speed
void resetGame() {
  gameOver = false;
  gAppStatus = RUNNING;
  winner = NONE_WINNER;

  paddle1_position = Paddle1_INIT_POS;
  paddle2_position = Paddle2_INIT_POS;

  ball1_position = ORIGIN;
  ball2_position = ORIGIN;
  ball3_position = ORIGIN;

  // Default movements befoere the first hit 
  ball1_movement = {static_cast<float>(-SPEED), static_cast<float>(SPEED)};
  ball2_movement = {static_cast<float>(SPEED), static_cast<float>(-SPEED)};
  ball3_movement = {static_cast<float>(-SPEED), static_cast<float>(-SPEED)};

  // By default only ball1 is active
  ball2_status = INACTIVE;
  ball3_status = INACTIVE;
}

void processInput() {
  // Get mouse position not needed
  gMousePosition = GetMousePosition();
  
  // Toggle paddle2 control mode with P key
  if(!gameOver) {
    if(IsKeyPressed(KEY_P)) {
      paddle2_state = (paddle2_state == FOLLOW) ? PLAYER : FOLLOW;
    }
  }

  // Restart if game over
  if(gameOver) {
    if(IsKeyPressed(KEY_R)) {
      resetGame();
    }
    // allow quit while game over
    if(IsKeyPressed(KEY_Q) || WindowShouldClose())
      gAppStatus = TERMINATED;
    return;
  }

  // Dynamic ball control (1-3 balls)
  if(IsKeyPressed(KEY_ONE)) {
    // Activate only ball 1, deactivate others
    ball1_movement = {static_cast<float>(-SPEED), static_cast<float>(SPEED)};
    ball2_status = INACTIVE;
    ball3_status = INACTIVE;
  }
  else if(IsKeyPressed(KEY_TWO)) {
    // Activate balls 1 and 2, keep three inactive 
    ball1_movement = {static_cast<float>(-SPEED), static_cast<float>(SPEED)};
    ball2_status = ACTIVE;
    ball2_movement = {static_cast<float>(SPEED), static_cast<float>(-SPEED)};
    ball3_status = INACTIVE;
  }
  else if(IsKeyPressed(KEY_THREE)) {
    // Activate all three balls
    ball1_movement = {static_cast<float>(-SPEED), static_cast<float>(SPEED)};
    ball2_status = ACTIVE;
    ball2_movement = {static_cast<float>(SPEED), static_cast<float>(-SPEED)};
    ball3_status = ACTIVE;
    ball3_movement = {static_cast<float>(-SPEED), static_cast<float>(-SPEED)};
  }
  
  // Control paddle1
  if(IsKeyDown(KEY_W) || IsKeyDown(KEY_S)) {
    isPaddleControlled1 = true;
    if(IsKeyDown(KEY_W)) {
      paddle1_movement.y = -1.0f;
    }
    else if(IsKeyDown(KEY_S)) {
      paddle1_movement.y = 1.0f;
    }
  } else {
    isPaddleControlled1 = false;
    paddle1_movement.y = 0;
  }

  // Control paddle 2 
  if(paddle2_state == PLAYER) {
    if(IsKeyDown(KEY_UP)) {
      paddle2_movement.y = -1.0f;
    }
    else if(IsKeyDown(KEY_DOWN)) {
      paddle2_movement.y = 1.0f;
    }
    else {
      paddle2_movement.y = 0;
    }
  }
  //part that didnt work where I was trying to use mouse
  // if (isColliding(&gMousePosition, &Paddle1_BASE_SIZE, &paddle1_position,
  //                 &Paddle1_BASE_SIZE)) {
  //   isPaddleControlled1 = true;
  //   paddle1_movement = {0.0f, -1.0f};
    
  //   // Get random values between -1 and 1 for direction
  //   float randX = (float)(rand() % 200 - 100) / 100.0f;  
  //   float randY = (float)(rand() % 200 - 100) / 100.0f; 
  //   ball1_movement = {-(ball1_movement.x/ball1_movement.x) *randX, randY * SPEED};
  // }

  if (GetLength(&paddle1_movement) > 1.0f)
    Normalise(&paddle1_movement);
  if (IsKeyPressed(KEY_Q) || WindowShouldClose())
    gAppStatus = TERMINATED;
}

void update() {
    float ticks =(float)  GetTime();
    float deltaTime =ticks - gPreviousTicks;
    gPreviousTicks = ticks;
    // Update paddle1 position with speed and boundary checking
    paddle1_position.y += paddle1_movement.y * SPEED * deltaTime;
    
    // Keep paddle1 within screen bounds
    if (paddle1_position.y < paddle1_scale.y/2) {
        paddle1_position.y = paddle1_scale.y/2;
    }
    if (paddle1_position.y > SCREEN_HEIGHT - paddle1_scale.y/2) {
        paddle1_position.y = SCREEN_HEIGHT - paddle1_scale.y/2;
    }
    
    // Update paddle2 position based on control mode
    if (paddle2_state == FOLLOW) {
        float targetY = lerp(paddle2_position.y, ball1_position.y, 0.1f);
        paddle2_position.y = targetY;
    } else {
        paddle2_position.y += paddle2_movement.y * SPEED * deltaTime;
    }
    
    if (paddle2_position.y < paddle2_scale.y/2) {
        paddle2_position.y = paddle2_scale.y/2;
    }
    if (paddle2_position.y > SCREEN_HEIGHT - paddle2_scale.y/2) {
        paddle2_position.y = SCREEN_HEIGHT - paddle2_scale.y/2;
    }
    // Ball 1
    ball1_position.x += ball1_movement.x * deltaTime;
    ball1_position.y += ball1_movement.y * deltaTime;
    
    // Ball 2
    if(ball2_status == ACTIVE) {
        ball2_position.x += ball2_movement.x * deltaTime;
        ball2_position.y += ball2_movement.y * deltaTime;
        
        if (ball2_position.y < ball2_scale.y/2) { 
            ball2_position.y = ball2_scale.y/2; 
            ball2_movement.y *= -1; 
        }
        if (ball2_position.y > SCREEN_HEIGHT - ball2_scale.y/2) { 
            ball2_position.y = SCREEN_HEIGHT - ball2_scale.y/2;
            ball2_movement.y *= -1; 
        }
        
        // Ball 2 paddle collisions
        if(isColliding(&ball2_position, &ball2_scale, &paddle1_position, &paddle1_scale)) {
            ball2_movement.x *= -1;
            ball2_position.x = paddle1_position.x + (ball2_scale.x + paddle1_scale.x) / 2.0f;
        }
        
        if(isColliding(&ball2_position, &ball2_scale, &paddle2_position, &paddle2_scale)) {
            ball2_movement.x *= -1;
            ball2_position.x = paddle2_position.x - (ball2_scale.x + paddle2_scale.x) / 2.0f;
        }
    }
    
    // Ball 3
    if(ball3_status == ACTIVE) {
        ball3_position.x += ball3_movement.x * deltaTime;
        ball3_position.y += ball3_movement.y * deltaTime;
        
        // Ball 3 boundary bouncing
        if (ball3_position.y < ball3_scale.y/2) { 
            ball3_position.y = ball3_scale.y/2; 
            ball3_movement.y *= -1; 
        }
        if (ball3_position.y > SCREEN_HEIGHT - ball3_scale.y/2) { 
            ball3_position.y = SCREEN_HEIGHT - ball3_scale.y/2;
            ball3_movement.y *= -1; 
        }
        
        // Ball 3 paddle collisions
        if(isColliding(&ball3_position, &ball3_scale, &paddle1_position, &paddle1_scale)) {
            ball3_movement.x *= -1;
            ball3_position.x = paddle1_position.x + (ball3_scale.x + paddle1_scale.x) / 2.0f;
        }
        
        if(isColliding(&ball3_position, &ball3_scale, &paddle2_position, &paddle2_scale)) {
            ball3_movement.x *= -1;
            ball3_position.x = paddle2_position.x - (ball3_scale.x + paddle2_scale.x) / 2.0f;
        }
    }
    
    // Keep balls within screen bounds
    if( paddle2_state == FOLLOW){
      paddle2_position.y = lerp(paddle2_position.y, ball1_position.y, 0.1f);
      
    }

    if (ball1_position.y < ball1_scale.y/2) { 
      ball1_position.y = ball1_scale.y/2; 
      ball1_movement.y *= -1; }
    if (ball1_position.y > SCREEN_HEIGHT - ball1_scale.y/2) 
    { ball1_position.y = SCREEN_HEIGHT - ball1_scale.y/2;
       ball1_movement.y *= -1; 
      }
      //checks the collision with paddles
      if(isColliding(&ball1_position, &ball1_scale, &paddle1_position, &paddle1_scale)) {
        float randY = (float)(rand() % 200 - 100) / 100.0f; 
        ball1_movement.x = SPEED;
        ball1_movement.y = randY * SPEED;
        ball1_position.x = paddle1_position.x + (ball1_scale.x + paddle1_scale.x) / 2.0f;
      }
      
      if(isColliding(&ball1_position, &ball1_scale, &paddle2_position, &paddle2_scale)) {
        float randY = (float)(rand() % 200 - 100) / 100.0f;
        ball1_movement.x = -SPEED;
        ball1_movement.y = randY * SPEED;
        ball1_position.x = paddle2_position.x - (ball1_scale.x + paddle2_scale.x) / 2.0f;
      }
    
      if(!gameOver) {
        if (ball1_position.x < 0) {
          winner = RED_WINNER;
          gameOver = true;
        } else if (ball1_position.x > SCREEN_WIDTH) {
          winner = BLUE_WINNER;
          gameOver = true;
        }

        if(!gameOver && ball2_status == ACTIVE) {
          if (ball2_position.x < 0) {
            winner = RED_WINNER;
            gameOver = true;
          } else if (ball2_position.x > SCREEN_WIDTH) {
            winner = BLUE_WINNER;
            gameOver = true;
          }
        }
        if(!gameOver && ball3_status == ACTIVE) {
          if (ball3_position.x < 0) {
            winner = RED_WINNER;
            gameOver = true;
          } else if (ball3_position.x > SCREEN_WIDTH) {
            winner = BLUE_WINNER;
            gameOver = true;
          }
        }

        if(gameOver) {
          ball1_movement = {0.0f, 0.0f};
          ball2_movement = {0.0f, 0.0f};
          ball3_movement = {0.0f, 0.0f};
        }
      }
  }
void render() {
  BeginDrawing();
  ClearBackground(RAYWHITE);
  
  renderObject(&background_texture, &background_position, &background_scale);
  
  renderObject(&paddle1_texture, &paddle1_position, &paddle1_scale);
  renderObject(&paddle2_texture, &paddle2_position, &paddle2_scale);
  
  renderObject(&ball1_texture, &ball1_position, &ball1_scale);
  
  if(ball2_status == ACTIVE) {
      renderObject(&ball2_texture, &ball2_position, &ball2_scale);
  }
  if(ball3_status == ACTIVE) {
      renderObject(&ball3_texture, &ball3_position, &ball3_scale);
  }

  if(gameOver) {
    DrawText("GAME OVER", SCREEN_WIDTH/2 - MeasureText("GAME OVER", 40)/2, SCREEN_HEIGHT/2 - 80, 40, RED);
    if(winner == BLUE_WINNER) {
      DrawText("BLUE WINS!", SCREEN_WIDTH/2 - MeasureText("BLUE WINS!", 30)/2, SCREEN_HEIGHT/2 - 20, 30, BLUE);
    } else if(winner == RED_WINNER) {
      DrawText("RED WINS!", SCREEN_WIDTH/2 - MeasureText("RED WINS!", 30)/2, SCREEN_HEIGHT/2 - 20, 30, RED);
    }
    DrawText("Press R to Restart or Q to Quit", SCREEN_WIDTH/2 - MeasureText("Press R to Restart or Q to Quit", 20)/2, SCREEN_HEIGHT/2 + 30, 20, BLACK);
  }
  
  EndDrawing();
}

void shutdown() { CloseWindow(); }

int main(void) {
  initialise();

  while (gAppStatus == RUNNING) {
    processInput();
    update();
    render();
  }

  shutdown();

  return 0;
}


