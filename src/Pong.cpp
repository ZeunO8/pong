/*
*/
#include <Pong.hpp>
#include <iostream>
#include <ostream>
#include <string>
#include <random>
#include <NeuralNetwork.hpp>
#include <fstream>
#include <ByteStream.hpp>
#include <Visualizer.hpp>
using namespace pong;
using namespace zeuron;
using namespace bs;

std::random_device _rd;
std::mt19937 _mt19937(_rd());
/*
 */
std::shared_ptr<NeuralNetwork> loadOrCreateAINetwork();
void saveAINetwork();
std::mutex aiNetworkMutex;
std::shared_ptr<NeuralNetwork> aiNetwork;

int main()
{
  aiNetwork = loadOrCreateAINetwork();
  Visualizer visualizer(*aiNetwork, 640, 480);
  PongGame game(960, 540);
  saveAINetwork();
};

ButtonEntity::ButtonEntity(anex::IGame& game,
                           const char* text,
                           const int& x,
                           const int& y,
                           const int& width,
                           const int& height,
                           const int& borderWidth,
                           const int& padding,
                           const bool& selected,
                           const std::function<void()>& onEnter):
  IEntity(game),
  text(text),
  x(x),
  y(y),
  width(width),
  height(height),
  borderWidth(borderWidth),
  padding(padding),
  selected(selected),
  scale((height / 2 - (padding * 2) - (borderWidth * 2)) / 5),
  textBounds(fenster_text_bounds(text, scale)),
  onEnter(onEnter)
{
};

void ButtonEntity::render()
{
  auto &fensterGame = (FensterGame &)game;
  uint32_t borderColor = selected ? 0x00999999 : 0x00555555;
  uint32_t bgColor = selected ? 0x00222222 : 0x00000000;
  fenster_rect(fensterGame.f, x, y, width, height, borderColor);
  fenster_rect(fensterGame.f, x + borderWidth, y + borderWidth, width - borderWidth * 2, height - borderWidth * 2,
               bgColor);
  fenster_text(fensterGame.f, x + width / 2 - std::get<0>(textBounds) / 2, y + height / 2 - std::get<1>(textBounds) / 2,
               text, scale, 0x00ffffff);
};

/*
 */
PongGame::PongGame(const int& windowWidth, const int& windowHeight):
  FensterGame(windowWidth, windowHeight)
{
  setIScene(std::make_shared<MainMenuScene>(*this));
  escKeyId = addKeyHandler(27, std::bind(&PongGame::onEscape, this, std::placeholders::_1));
};

void PongGame::onEscape(const bool& pressed)
{
  if (pressed)
  {
    close();
  }
};

MainMenuScene::MainMenuScene(anex::IGame& game):
  IScene(game),
  borderWidth(4),
  padding(4),
  playerVsAIButton(std::make_shared<ButtonEntity>(game, "Player vs AI", 0, 0, int(game.windowWidth / 1.5),
                                                  game.windowHeight / 5, borderWidth, padding, true,
                                                  std::bind(&MainMenuScene::onPlayerVsAIEnter, this))),
  trainAIButton(std::make_shared<ButtonEntity>(game, "Train AI", 0, 0, int(game.windowWidth / 1.5),
                                               game.windowHeight / 5, borderWidth, padding, false,
                                               std::bind(&MainMenuScene::onTrainAIEnter, this))),
  playerVsPlayerButton(std::make_shared<ButtonEntity>(game, "Player vs Player", 0, 0,
                                                      int(game.windowWidth / 1.5), game.windowHeight / 5,
                                                      borderWidth, padding, false,
                                                      std::bind(&MainMenuScene::onPlayerVsPlayerEnter, this))),
  exitButton(std::make_shared<ButtonEntity>(game, "Exit", 0, 0, int(game.windowWidth / 1.5),
                                            game.windowHeight / 5, borderWidth, padding, false,
                                            std::bind(&MainMenuScene::onExitEnter, this))),
  buttonsList({playerVsAIButton, trainAIButton, playerVsPlayerButton, exitButton})
{
  addEntity(playerVsAIButton);
  addEntity(trainAIButton);
  addEntity(playerVsPlayerButton);
  addEntity(exitButton);
  positionButtons();
  upKeyId = game.addKeyHandler(17, std::bind(&MainMenuScene::onUpKey, this, std::placeholders::_1));
  downKeyId = game.addKeyHandler(18, std::bind(&MainMenuScene::onDownKey, this, std::placeholders::_1));
  enterKeyId = game.addKeyHandler(10, std::bind(&MainMenuScene::onEnterKey, this, std::placeholders::_1));
};

MainMenuScene::~MainMenuScene()
{
  game.removeKeyHandler(17, upKeyId);
  game.removeKeyHandler(18, downKeyId);
};

void MainMenuScene::positionButtons()
{
  int buttonsTotalX = 0, buttonsTotalY = 0;
  auto buttonsListSize = buttonsList.size();
  auto buttonsListData = buttonsList.data();
  buttonsTotalX += buttonsListData[0]->width;
  for (unsigned int buttonIndex = 0; buttonIndex < buttonsListSize; ++buttonIndex)
  {
    auto& button = buttonsListData[buttonIndex];
    buttonsTotalY += button->height;
    if (buttonIndex < buttonsListSize - 1)
    {
      buttonsTotalY += 2;
    }
  }
  int placementX = game.windowWidth / 2 - buttonsTotalX / 2;
  int placementY = game.windowHeight / 2 - buttonsTotalY / 2;
  for (unsigned int buttonIndex = 0; buttonIndex < buttonsListSize; ++buttonIndex)
  {
    auto& button = buttonsListData[buttonIndex];
    button->x = placementX;
    button->y = placementY;
    placementY += button->height;
    if (buttonIndex < buttonsListSize - 1)
    {
      placementY += 2;
    }
  }
};

void MainMenuScene::onUpKey(const bool& pressed)
{
  if (pressed)
  {
    auto buttonsListSize = buttonsList.size();
    auto buttonsListData = buttonsList.data();
    for (int i = 0; i < buttonsListSize; ++i)
    {
      auto& button = buttonsList[i];
      if (button->selected)
      {
        button->selected = false;
        auto prevI = i - 1;
        if (prevI >= 0)
        {
          buttonsListData[prevI]->selected = true;
        }
        else
        {
          buttonsListData[buttonsListSize - 1]->selected = true;
        }
        break;
      }
    }
  }
};

void MainMenuScene::onDownKey(const bool& pressed)
{
  if (pressed)
  {
    auto buttonsListSize = buttonsList.size();
    auto buttonsListData = buttonsList.data();
    for (int i = 0; i < buttonsListSize; ++i)
    {
      auto& button = buttonsList[i];
      if (button->selected)
      {
        button->selected = false;
        auto nextI = i + 1;
        if (nextI < buttonsListSize)
        {
          buttonsListData[nextI]->selected = true;
        }
        else
        {
          buttonsListData[0]->selected = true;
        }
        break;
      }
    }
  }
};

void MainMenuScene::onEnterKey(const bool& pressed)
{
  if (pressed)
  {
    auto buttonsListSize = buttonsList.size();
    auto buttonsListData = buttonsList.data();
    for (int i = 0; i < buttonsListSize; ++i)
    {
      auto& button = buttonsList[i];
      if (button->selected)
      {
        button->onEnter();
        break;
      }
    }
  }
};

void MainMenuScene::onPlayerVsAIEnter()
{
  auto pongScenePointer = std::dynamic_pointer_cast<PongScene>(game.setIScene(std::make_shared<PongScene>(
    game,
    std::make_shared<AIBat>(game, Bat::Left),
    std::make_shared<PlayerBat>(game, Bat::Right, PlayerBat::UpDown)
  )));
  auto aiLeftBatPointer = std::dynamic_pointer_cast<AIBat>(pongScenePointer->leftBat);
  aiLeftBatPointer->startActivation(pongScenePointer);
};

void MainMenuScene::onTrainAIEnter()
{
  auto pongScenePointer = std::dynamic_pointer_cast<PongScene>(game.setIScene(std::make_shared<PongScene>(
    game,
    std::make_shared<AIBat>(game, Bat::Left),
    std::make_shared<AIBat>(game, Bat::Right)
  )));
  auto aiLeftBatPointer = std::dynamic_pointer_cast<AIBat>(pongScenePointer->leftBat);
  aiLeftBatPointer->startActivation(pongScenePointer);
  auto aiRightBatPointer = std::dynamic_pointer_cast<AIBat>(pongScenePointer->rightBat);
  aiRightBatPointer->startActivation(pongScenePointer);
};

void MainMenuScene::onPlayerVsPlayerEnter()
{
  game.setIScene(std::make_shared<PongScene>(
    game,
    std::make_shared<PlayerBat>(game, Bat::Left, PlayerBat::WS),
    std::make_shared<PlayerBat>(game, Bat::Right, PlayerBat::UpDown)
  ));
};

void MainMenuScene::onExitEnter()
{
  game.close();
};

Bat::Bat(anex::IGame& game, const Bat::Side& side):
  IEntity(game),
  side(side),
  height(game.windowHeight / 5)
{
  x = side == Left ? 20 : game.windowWidth - 20;
  y = game.windowHeight / 2;
};

void Bat::render()
{
  auto &fensterGame = (FensterGame &)game;
  if ((velocityY < 0 && y - height / 2 > 44) || (velocityY > 0 && y + height / 2 < game.windowHeight - 44))
  {
    y += velocityY;
  }
  uint32_t color = side == Bat::Left ? 0x00ff0000 : 0x000000ff;
  fenster_rect(fensterGame.f, x - 2, y - height / 2, 4, height, color);
};

void Bat::onUpKey(const bool& pressed)
{
  velocityY = pressed ? -8 : 0;
};

void Bat::onDownKey(const bool& pressed)
{
  velocityY = pressed ? 8 : 0;
};

Ball::Ball(anex::IGame& game, PongScene& pongScene, const int& radius):
  IEntity(game),
  pongScene(pongScene),
  radius(radius)
{
  reset();
};

void Ball::startMoving()
{
  std::uniform_int_distribution<int> distribution(1, 4);
  auto startingDirection = distribution(_mt19937);
  switch (startingDirection)
  {
  case 1:
    {
      velocityX = 4;
      velocityY = 2;
      break;
    };
  case 2:
    {
      velocityX = -4;
      velocityY = 2;
      break;
    };
  case 3:
    {
      velocityX = 4;
      velocityY = -2;
      break;
    };
  case 4:
    {
      velocityX = -4;
      velocityY = -2;
      break;
    };
  }
}


void Ball::render()
{
  auto &fensterGame = (FensterGame &)game;
  x += velocityX;
  y += velocityY;
  if (y <= 40 || y >= game.windowHeight - 40)
  {
    velocityY = -velocityY;
  }
  else if (x <= 28 || x >= game.windowWidth - 28)
  {
    if (x <= 16)
    {
      ++pongScene.rightScore;
      reset();
      return;
    }
    else if (x == 28)
    {
      auto& leftBat = *pongScene.leftBat;
      if (y < leftBat.y - leftBat.height / 2 || y > leftBat.y + leftBat.height / 2)
      {
        goto _draw;
      }
      else
      {
        velocityY = velocityY + (leftBat.velocityY * 1) / 1;
      }
    }
    else if (x >= game.windowWidth - 16)
    {
      ++pongScene.leftScore;
      reset();
      return;
    }
    else if (x == game.windowWidth - 28)
    {
      auto& rightBat = *pongScene.rightBat;
      if (y < rightBat.y - rightBat.height / 2 || y > rightBat.y + rightBat.height / 2)
      {
        goto _draw;
      }
      else
      {
        velocityY = velocityY + (rightBat.velocityY * 1) / 1;
      }
    }
    else if (x < 28 || x > game.windowWidth - 28)
    {
      goto _draw;
    }
  _reflect:
    velocityX = -velocityX;
  }
_draw:
  fenster_circle(fensterGame.f, x, y, radius, 0x00ffffff);
  trajectory = calculateTrajectory();
  auto &bounces = std::get<0>(trajectory);
  auto &finalPosition = std::get<1>(trajectory);
  for (auto &bounce : bounces)
  {
    auto &start = bounce.start;
    auto &end = bounce.end;
    fenster_line(fensterGame.f, start.x, start.y, end.x, end.y, 0x0000ff00);
  }
};

void Ball::reset()
{
  x = game.windowWidth / 2;
  y = game.windowHeight / 2;
  startMoving();
};

std::pair<std::vector<Bounce>, Point> Ball::calculateTrajectory()
{
  std::vector<Bounce> bounces;
  Point currentPos = {x, y};
  Point velocity = {velocityX, velocityY};
  auto &playArea = pongScenePointer->playArea;
  float leftWall = playArea.x - (playArea.width / 2);
  float rightWall = playArea.width + playArea.x - (playArea.width / 2);
  float topWall = playArea.y - (playArea.height / 2);
  float bottomWall = playArea.height + playArea.y - (playArea.height / 2);

  while (true)
  {
    float timeToVerticalWall = std::numeric_limits<float>::infinity();
    float timeToHorizontalWall = std::numeric_limits<float>::infinity();

    // Calculate time to the next vertical wall (left or right)
    if (velocity.x > 0)
    {
      timeToVerticalWall = (rightWall - currentPos.x) / velocity.x;
    }
    else if (velocity.x < 0)
    {
      timeToVerticalWall = (leftWall - currentPos.x) / velocity.x;
    }

    // Calculate time to the next horizontal wall (top or bottom)
    if (velocity.y > 0)
    {
      timeToHorizontalWall = (bottomWall - currentPos.y) / velocity.y;
    }
    else if (velocity.y < 0)
    {
      timeToHorizontalWall = (topWall - currentPos.y) / velocity.y;
    }

    // Determine which wall will be hit first
    if (timeToVerticalWall < timeToHorizontalWall)
    {
      // Ball hits a vertical wall
      Point nextPos = {currentPos.x + velocity.x * timeToVerticalWall, currentPos.y + velocity.y * timeToVerticalWall};
      bounces.push_back({currentPos, nextPos});

      // Check if it's a final hit (left or right wall)
      if (nextPos.x == leftWall || nextPos.x == rightWall)
      {
        return {bounces, nextPos};
      }

      // Update for bounce
      currentPos = nextPos;
      velocity.x = -velocity.x; // Reverse horizontal direction
    }
    else
    {
      // Ball hits a horizontal wall
      Point nextPos = {
        currentPos.x + velocity.x * timeToHorizontalWall, currentPos.y + velocity.y * timeToHorizontalWall
      };
      bounces.push_back({currentPos, nextPos});

      // Update for bounce
      currentPos = nextPos;
      velocity.y = -velocity.y; // Reverse vertical direction
    }
  }
};

Board::Board(anex::IGame& game, PongScene& pongScene):
  IEntity(game),
  pongScene(pongScene),
  boardX((float)(12 + (game.windowWidth - 24) / 2)),
  boardY((float)(36 + (game.windowHeight - 72) / 2)),
  boardWidth((float)game.windowWidth - 24),
  boardHeight((float)game.windowHeight - 72),
  playArea({boardX, boardY, boardWidth, boardHeight})
{
};

void Board::render()
{
  auto &fensterGame = (FensterGame &)game;
  // white border
  fenster_rect(fensterGame.f, 8, 32, game.windowWidth - 16, game.windowHeight - 64, 0x00ffffff);
  // black play area
  fenster_rect(fensterGame.f, 12, 36, game.windowWidth - 24, game.windowHeight - 72, 0x00000000);
  // blue left hit rect
  fenster_rect(fensterGame.f, 12, 36, 4, game.windowHeight - 72, 0x000000ff);
  // red right hit rect
  fenster_rect(fensterGame.f, game.windowWidth - 16, 36, 4, game.windowHeight - 72, 0x00ff0000);
  // left score
  fenster_text(fensterGame.f, game.windowWidth / 4, 6, std::to_string(pongScene.leftScore).c_str(), 4, 0x00ffffff);
  // right score
  fenster_text(fensterGame.f, game.windowWidth / 2 + game.windowWidth / 4, 6,
               std::to_string(pongScene.rightScore).c_str(), 4, 0x00ffffff);
};

PlayArea& Board::getPlayArea()
{
  return playArea;
};

Countdown::Countdown(anex::IGame& game, const int& x, const int& y, const int& scale,
                     const std::function<void()>& onZero):
  IEntity(game),
  x(x),
  y(y),
  scale(scale),
  timer(3),
  onZero(onZero),
  countdownThread(&Countdown::startCountdown, this)
{
};

Countdown::~Countdown()
{
  countdownThread.join();
};

void Countdown::render()
{
  auto &fensterGame = (FensterGame &)game;
  fenster_text(fensterGame.f, x - int(1.5 * scale), y - int(2.5 * scale), std::to_string(timer).c_str(), scale,
               0x00ffffff);
};

void Countdown::startCountdown()
{
  while (timer > 0)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (!game.open)
    {
      return;
    }
    --timer;
  }
  onZero();
};

PongScene::PongScene(anex::IGame& game, const std::shared_ptr<Bat>& leftBat, const std::shared_ptr<Bat>& rightBat):
  IScene(game),
  leftBat(leftBat),
  rightBat(rightBat),
  board(std::make_shared<Board>(game, *this)),
  ball(std::make_shared<Ball>(game, *this, 4)),
  countdown(std::make_shared<Countdown>(game, game.windowWidth / 2, game.windowHeight / 2,
                                        game.windowHeight / 30, std::bind(&PongScene::onCountdownZero, this))),
  playArea(board->getPlayArea())
{
  ball->pongScenePointer = this;
  addEntity(board);
  addEntity(leftBat);
  addEntity(rightBat);
  countdownId = addEntity(countdown);
};

PongScene::~PongScene()
{
  gameStarted = false;
}

void PongScene::onCountdownZero()
{
  removeEntity(countdownId);
  ballId = addEntity(ball);
  gameStarted = true;
};

PlayerBat::PlayerBat(anex::IGame& game, const Bat::Side& side, const UseKeys& useKeys):
  Bat(game, side),
  useKeys(useKeys)
{
  upKeyId = game.addKeyHandler(
    useKeys == UseKeys::WS ? 87 : 17,
    std::bind(&Bat::onUpKey, this, std::placeholders::_1)
  );
  downKeyId = game.addKeyHandler(
    useKeys == UseKeys::WS ? 83 : 18,
    std::bind(&Bat::onDownKey, this, std::placeholders::_1)
  );
};

std::pair<std::shared_ptr<char>, unsigned long> readFileToBuffer(const std::string& filename)
{
  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  if (!file.is_open())
  {
    throw std::ios_base::failure("Error: Unable to open file for reading.");
  }
  std::streampos fileSize = file.tellg();
  if (fileSize <= 0)
  {
    throw std::ios_base::failure("Error: File is empty or has invalid size.");
  }
  unsigned long size = static_cast<unsigned long>(fileSize);
  std::shared_ptr<char> buffer(new char[size], std::default_delete<char[]>());
  file.seekg(0, std::ios::beg);
  file.read(buffer.get(), size);
  if (!file)
  {
    throw std::ios_base::failure("Error: Reading the file failed.");
  }
  file.close();
  return std::make_pair(buffer, size);
};

void writeBufferToFile(const char* buffer, unsigned long size, const std::string& filename)
{
  std::ofstream file(filename, std::ios::binary);
  if (!file.is_open())
  {
    std::cerr << "Error: Unable to open file for writing.\n";
    return;
  }
  file.write(buffer, static_cast<std::streamsize>(size));
  if (!file)
  {
    std::cerr << "Error: Writing to the file failed.\n";
  }
  file.close();
};

std::shared_ptr<NeuralNetwork> loadOrCreateAINetwork()
{
  try
  {
    auto bytesSizePair = readFileToBuffer("pong.nrl");
    ByteStream byteStream(std::get<1>(bytesSizePair), std::get<0>(bytesSizePair));
    return std::make_shared<NeuralNetwork>(byteStream);
  }
  catch (...)
  {
    return std::make_shared<NeuralNetwork>(
      9, // Inputs: which side [0 or 1], distance to bat center, height of bat, ballVelocityX/Y, hitPointX/Y
      std::vector<std::pair<NeuralNetwork::ActivationType, unsigned long>>({
        {NeuralNetwork::ReLU, 10}, // First hidden layer with ReLU for feature extraction
        {NeuralNetwork::ReLU, 8}, // Second hidden layer for refinement
        {NeuralNetwork::Sigmoid, 4}, // Third hidden layer to add non-linearity
        {NeuralNetwork::Sigmoid, 2} // Output layer: Sigmoid for binary outputs (keyUp, keyDown)
      }),
      0.01 // Reduced learning rate for stable convergence
    );
  }
};

void saveAINetwork()
{
  std::lock_guard lock(aiNetworkMutex);
  auto nnStream = aiNetwork->serialize();
  writeBufferToFile(nnStream.bytes.get(), nnStream.bytesSize, "pong.nrl");
};

AIBat::AIBat(anex::IGame& game, const Bat::Side& side):
  Bat(game, side)
{
};

void AIBat::startActivation(const std::shared_ptr<PongScene>& pongScenePointer)
{
  this->pongScenePointer = pongScenePointer;
  activationThread = std::thread(&AIBat::activationFunction, this);
};

long double distance(const long double& a, const long double& b)
{
  return std::abs(a - b);
}

long double distance(const std::pair<long double, long double>& point1,
                     const std::pair<long double, long double>& point2)
{
  long double dx = point1.first - point2.first;
  long double dy = point1.second - point2.second;
  return std::sqrt(dx * dx + dy * dy);
}

void AIBat::activationFunction()
{
  long double sideDouble = side == Bat::Side::Left ? 0 : 1;
  auto& pongScene = *pongScenePointer;
  while (!pongScene.gameStarted)
  {
  }
  auto& ball = *pongScene.ball;
  auto& aiNetworkRef = *aiNetwork;
  auto& playArea = pongScene.playArea;
  while (pongScene.gameStarted)
  {
    std::lock_guard lock(aiNetworkMutex);
    long double distanceToBall = distance({x, y}, {ball.x, ball.y});
    long double heightOfBat = height;
    auto &trajectory = ball.trajectory;
    auto &hitPoint = std::get<1>(trajectory);
    auto &hitPointX = hitPoint.x;
    auto &hitPointY = hitPoint.y;
    std::vector<long double> input({
      sideDouble, distanceToBall, heightOfBat, ball.velocityX, ball.velocityY, ball.x, ball.y, hitPointX, hitPointY
    });
    aiNetworkRef.feedforward(input);
    auto outputs = aiNetworkRef.getOutputs();
    if (distance(outputs[0], 1) <= 0.03)
    {
      onUpKey(true);
    }
    else if (distance(outputs[1], 1) <= 0.03)
    {
      onDownKey(true);
    }
    else
    {
      velocityY = 0;
    }
    std::vector<long double> expectedOutputs;
    // auto yDifference = distance(y, hitPointY);
    auto onSide = (side == Bat::Side::Left ? hitPointX == 12 : hitPointX == 948);
    expectedOutputs.push_back(!onSide || hitPointY > y ? 0 : 1);
    expectedOutputs.push_back(!onSide || hitPointY < y ? 0 : 1);
    aiNetworkRef.backpropagate(expectedOutputs);
  }
}