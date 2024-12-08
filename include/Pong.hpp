/*
 */
#pragma once
#include <memory>
#include <thread>
#include <vector>
/*
 */
struct fenster;
namespace pong
{
	struct PongGame;
	struct IEntity
  {
    IEntity() = default;
    virtual ~IEntity() = default;
    virtual void render() = 0;
  };
  struct ButtonEntity : IEntity
{
  	PongGame &pongGame;
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
    ButtonEntity(PongGame &pongGame,
                 const char *text,
                 const int &x,
                 const int &y,
                 const int &width,
                 const int &height,
                 const int &borderWidth,
                 const int &padding,
                 const bool &selected);
    void render() override;
  };
  struct Scene
  {
    PongGame &pongGame;
    std::vector<std::shared_ptr<IEntity>> entities;
    Scene(PongGame &pongGame);
    virtual ~Scene() = default;
    std::shared_ptr<IEntity> addEntity(const std::shared_ptr<IEntity> &entity);
    void render();
  };
  struct PongGame
  {
    int windowWidth;
    int windowHeight;
    std::shared_ptr<uint32_t> buf;
    struct fenster *f = 0;
    std::thread windowThread;
    std::shared_ptr<Scene> scene;
    PongGame(const int &windowWidth, const int &windowHeight);
    ~PongGame();
    void startWindow();
    void render();
    std::shared_ptr<Scene> setScene(const std::shared_ptr<Scene> &scene);
  };
  struct MainMenuScene : Scene
  {
    int borderWidth;
    int padding;
    std::shared_ptr<ButtonEntity> playerVsAIButton;
    std::shared_ptr<ButtonEntity> playerVsPlayerButton;
    std::shared_ptr<ButtonEntity> exitButton;
    std::vector<std::shared_ptr<ButtonEntity>> buttonsList;
    MainMenuScene(PongGame &pongGame);
    void positionButtons();
  };
}