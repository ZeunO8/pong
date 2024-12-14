/*
 */
#pragma once
#include <memory>
#include <thread>
#include <vector>
#include <unordered_map>
#include <map>
#include <functional>
/*
 */
struct fenster;
namespace pong
{
  struct PongGame;
  struct PongScene;
  struct IEntity
  {
  	PongGame &pongGame;
    IEntity(PongGame &pongGame);
    virtual ~IEntity() = default;
    virtual void render() = 0;
  };
  struct ButtonEntity : IEntity
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
    ButtonEntity(PongGame &pongGame,
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
  struct Scene
  {
    PongGame &pongGame;
    std::unordered_map<unsigned int, std::shared_ptr<IEntity>> entities;
    unsigned int entitiesCount;
    Scene(PongGame &pongGame);
    virtual ~Scene() = default;
    unsigned int addEntity(const std::shared_ptr<IEntity> &entity);
    void removeEntity(const unsigned int &id);
    void render();
  };
  struct PongGame
  {
    int windowWidth;
    int windowHeight;
    std::shared_ptr<uint32_t> buf;
    struct fenster *f = 0;
    std::thread windowThread;
    std::unordered_map<unsigned int, int> keys;
    std::unordered_map<unsigned int, std::pair<unsigned int, std::map<unsigned int, std::function<void(const bool &)>>>> keyHandlers;
    std::unordered_map<unsigned int, std::pair<unsigned int, std::map<unsigned int, std::function<void()>>>> keyUpdateHandlers;
    std::shared_ptr<Scene> scene;
    bool open = true;
    unsigned int escKeyId = 0;
    PongGame(const int &windowWidth, const int &windowHeight);
    ~PongGame();
    void startWindow();
    void render();
    void updateKeys();
    unsigned int addKeyHandler(const unsigned int &key, const std::function<void(const bool &)> &callback);
    void removeKeyHandler(const unsigned int &key, unsigned int &id);
    unsigned int addKeyUpdateHandler(const unsigned int &key, const std::function<void()> &callback);
    void removeKeyUpdateHandler(const unsigned int &key, unsigned int &id);
    std::shared_ptr<Scene> setScene(const std::shared_ptr<Scene> &scene);
    void close();
    void onEscape(const bool &pressed);
  };
  struct MainMenuScene : Scene
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
    MainMenuScene(PongGame &pongGame);
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
  struct Bat : IEntity
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
    Bat(PongGame &pongGame, const Bat::Side &side);
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
  struct Ball : IEntity
  {
    PongScene &pongScene;
    float x;
    float y;
    int radius;
    float velocityX;
    float velocityY;
    PongScene *pongScenePointer = 0;
    std::pair<std::vector<Bounce>, Point> trajectory;
    Ball(PongGame &pongGame, PongScene &pongScene, const int &radius);
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
  struct Board : IEntity
  {
    PongScene &pongScene;
    float boardX;
    float boardY;
    float boardWidth;
    float boardHeight;
    PlayArea playArea;
    Board(PongGame &pongGame, PongScene &pongScene);
    void render() override;
    PlayArea& getPlayArea();
  };
  struct Countdown : IEntity
  {
    int x;
    int y;
    int scale;
    int timer;
    std::function<void()> onZero;
    std::thread countdownThread;
    Countdown(PongGame &pongGame, const int &x, const int &y, const int &scale, const std::function<void()> &onZero);
    ~Countdown();
    void render() override;
    void startCountdown();
  };
  struct PongScene : Scene
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
    PongScene(PongGame &pongGame, const std::shared_ptr<Bat> &leftBat, const std::shared_ptr<Bat> &rightBat);
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
    PlayerBat(PongGame &pongGame, const Bat::Side &side, const UseKeys &useKeys);
  };
  struct AIBat : Bat
  {
    std::thread activationThread;
    std::shared_ptr<PongScene> pongScenePointer;
    AIBat(PongGame &pongGame, const Bat::Side &side);
    void startActivation(const std::shared_ptr<PongScene> &pongScenePointer);
    void activationFunction();
  };
}