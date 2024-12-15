/*
 */
#pragma once
#include <memory>
#include <thread>
#include <vector>
#include <unordered_map>
#include <map>
#include <functional>
#include <anex/modules/fenster/Fenster.hpp>
/*
 */
namespace pong
{
  using namespace anex::modules::fenster;
  struct PongGame;
  struct PongScene;
  struct ButtonEntity : anex::IEntity
  {
    const char *text;
    int x;
    int y;
    int width;
    int height;
    int borderWidth;
    int padding;
    bool selected;
    int scale;
    std::pair<int, int> textBounds;
    std::function<void()> onEnter;
    ButtonEntity(anex::IGame &game,
                 const char *text,
                 const int &x,
                 const int &y,
                 const int &width,
                 const int &height,
                 const int &borderWidth,
                 const int &padding,
                 const bool &selected,
                 const std::function<void()> &onEnter);
    void render() override;
  };
  struct PongGame : FensterGame
  {
    unsigned int escKeyId = 0;
    PongGame(const int &windowWidth, const int &windowHeight);
    void onEscape(const bool &pressed);
  };
  struct MainMenuScene : anex::IScene
  {
    int borderWidth;
    int padding;
    unsigned int upKeyId = 0;
    unsigned int downKeyId = 0;
    unsigned int enterKeyId = 0;
    std::shared_ptr<ButtonEntity> playerVsAIButton;
    std::shared_ptr<ButtonEntity> trainAIButton;
    std::shared_ptr<ButtonEntity> playerVsPlayerButton;
    std::shared_ptr<ButtonEntity> exitButton;
    std::vector<std::shared_ptr<ButtonEntity>> buttonsList;
    MainMenuScene(anex::IGame &game);
    ~MainMenuScene();
    void positionButtons();
    void onUpKey(const bool &pressed);
    void onDownKey(const bool &pressed);
    void onEnterKey(const bool &pressed);
    void onPlayerVsAIEnter();
    void onTrainAIEnter();
    void onPlayerVsPlayerEnter();
    void onExitEnter();
  };
  struct Bat : anex::IEntity
  {
    enum Side
    {
      Left,
      Right
    };
    float x;
    float y;
    int height;
    Side side;
    float velocityY = 0;
    Bat(anex::IGame &game, const Bat::Side &side);
    void render() override;
    void onUpKey(const bool &pressed);
    void onDownKey(const bool &pressed);
  };
  struct Point
  {
    float x;
    float y;
  };
  struct Bounce
  {
    Point start;
    Point end;
  };
  struct Ball : anex::IEntity
  {
    PongScene &pongScene;
    float x;
    float y;
    int radius;
    float velocityX;
    float velocityY;
    PongScene *pongScenePointer = 0;
    std::pair<std::vector<Bounce>, Point> trajectory;
    Ball(anex::IGame &game, PongScene &pongScene, const int &radius);
    void startMoving();
    void render() override;
    void reset();
    std::pair<std::vector<Bounce>, Point> calculateTrajectory();
  };
  struct PlayArea
  {
    float x;
    float y;
    float width;
    float height;
  };
  struct Board : anex::IEntity
  {
    PongScene &pongScene;
    float boardX;
    float boardY;
    float boardWidth;
    float boardHeight;
    PlayArea playArea;
    Board(anex::IGame &game, PongScene &pongScene);
    void render() override;
    PlayArea& getPlayArea();
  };
  struct Countdown : anex::IEntity
  {
    int x;
    int y;
    int scale;
    int timer;
    std::function<void()> onZero;
    std::thread countdownThread;
    Countdown(anex::IGame &game, const int &x, const int &y, const int &scale, const std::function<void()> &onZero);
    ~Countdown();
    void render() override;
    void startCountdown();
  };
  struct PongScene : anex::IScene
  {
    std::shared_ptr<Bat> leftBat;
    std::shared_ptr<Bat> rightBat;
    std::shared_ptr<Board> board;
    std::shared_ptr<Ball> ball;
    std::shared_ptr<Countdown> countdown;
    PlayArea& playArea;
    unsigned char leftScore = 0;
    unsigned char rightScore = 0;
    unsigned int countdownId;
    unsigned int ballId;
    bool gameStarted = false;
    PongScene(anex::IGame &game, const std::shared_ptr<Bat> &leftBat, const std::shared_ptr<Bat> &rightBat);
    ~PongScene();
    void onCountdownZero();
  };
  struct PlayerBat : Bat
  {
    enum UseKeys
    {
      WS,
      UpDown
    };
    UseKeys useKeys;
    unsigned int upKeyId;
    unsigned int downKeyId;
    PlayerBat(anex::IGame &game, const Bat::Side &side, const UseKeys &useKeys);
  };
  struct AIBat : Bat
  {
    std::thread activationThread;
    std::shared_ptr<PongScene> pongScenePointer;
    AIBat(anex::IGame &game, const Bat::Side &side);
    void startActivation(const std::shared_ptr<PongScene> &pongScenePointer);
    void activationFunction();
  };
}